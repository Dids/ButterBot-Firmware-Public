#include "SpeechGen.h"
#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <utility>
#include <esp_heap_caps.h>
#include <esp_timer.h>
#include <espeak-ng/speak_lib.h>
#include <espeak_data.h>
#include "Memory/ObjectMemory.h"
#include "Util/stdafx.h"

DEFINE_LOG(SpeechGen)

// Voice settings. Variants go after a '+', e.g. "fi+m3", "fi+f2", "fi+klatt2", "fi+robosoft"
static constexpr const char* Voice = "fi";
static constexpr int Rate = 160; // words per minute
static constexpr int Pitch = 50; // 0-100
static constexpr int Volume = 100; // 0-200

// IR command names were recognized in English, so they are read back in English
static constexpr const char* PhonemeVoice = "en";

// eSpeak calls back with this much audio at a time (60 ms is its minimum)
static constexpr int ChunkMs = 60;

static SpeechGen* instance = nullptr;

SpeechGen::SpeechGen(){
	instance = this;

	if(espeak_data_mount() != ESP_OK){
		CMF_LOG(SpeechGen, LogLevel::Error, "eSpeak data partition not available, speech disabled");
	}

	resampled.resize(Resampler::maxOutput(Resampler::InRate * ChunkMs / 1000));

	ringbuf = xRingbufferCreateWithCaps(BufferSize, RINGBUF_TYPE_BYTEBUF, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

	startSem = xSemaphoreCreateBinary();
	endSem = xSemaphoreCreateBinary();

	synthThread = std::make_unique<Threaded>([this]() {
		synthThreadFunc();
	}, "speech", 0, 16 * 1024, 7, 1, false);

	synthThread->start();
}

SpeechGen::~SpeechGen(){
	checkAndAbort();
	synthThread->stop(0);
	abort = true;
	xSemaphoreGive(startSem);
	while(synthThread->running()){
		delayMillis(1);
	}

	delete synthThread.get();
	vRingbufferDeleteWithCaps(ringbuf);
	vSemaphoreDelete(startSem);
	vSemaphoreDelete(endSem);
	// No espeak_Terminate(): it would free() the data tables that are mapped from flash
	instance = nullptr;
}

int SpeechGen::generate(std::span<int16_t> output, bool& last, TickType_t timeout){
	// Wait in short slices and re-check synthRunning: a synthesis that ends without writing anything
	// (engine not ready, nothing pronounceable) would otherwise leave this blocked on the ringbuffer forever.
	const TickType_t start = xTaskGetTickCount();
	size_t size;
	int16_t* data;
	for(;;){
		const bool running = synthRunning;
		const TickType_t elapsed = xTaskGetTickCount() - start;
		const TickType_t left = (running && elapsed < timeout) ? timeout - elapsed : 0;
		data = (int16_t*)xRingbufferReceiveUpTo(ringbuf, &size, std::min<TickType_t>(left, pdMS_TO_TICKS(10)), output.size() * sizeof(int16_t));
		if(data != nullptr || left == 0) break;
	}

	if(data == nullptr){
		CMF_LOG(SpeechGen, LogLevel::Debug, "Ringbuffer data is nullptr");
		last = !synthRunning;
		return 0;
	}

	CMF_LOG(SpeechGen, LogLevel::Debug, "Generate copying %zu bytes", size);

	memcpy(output.data(), data, size);
	vRingbufferReturnItem(ringbuf, data);

	last = false;

	return (int)size / 2;
}

void SpeechGen::parseText(std::string text){
	checkAndAbort();
	synthInput = std::move(text);
	synthInputType = InputType::Text;
	requestTime = esp_timer_get_time();
	synthRunning = true;
	xSemaphoreGive(startSem);
}

void SpeechGen::parsePhonemes(const std::string& phonemes){
	checkAndAbort();
	synthInput = CMUToESpeak(multinetToCMU(phonemes));
	synthInputType = InputType::Phonemes;
	requestTime = esp_timer_get_time();
	synthRunning = true;
	xSemaphoreGive(startSem);
}

void SpeechGen::parseSSML(std::string ssml){
	checkAndAbort();
	synthInput = std::move(ssml);
	synthInputType = InputType::SSML;
	requestTime = esp_timer_get_time();
	synthRunning = true;
	xSemaphoreGive(startSem);
}

void SpeechGen::cancel(){
	checkAndAbort();
}

bool SpeechGen::initEngine(){
	CMF_LOG(SpeechGen, LogLevel::Info, "Free heap before eSpeak init: internal %zu, PSRAM %zu",
			heap_caps_get_free_size(MALLOC_CAP_INTERNAL), heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

	// DONT_EXIT: on failure eSpeak would otherwise call exit() and reboot the robot
	const int rate = espeak_Initialize(AUDIO_OUTPUT_SYNCHRONOUS, ChunkMs, "/espeak", espeakINITIALIZE_DONT_EXIT);
	if(rate != (int) Resampler::InRate){
		CMF_LOG(SpeechGen, LogLevel::Error, "eSpeak init failed (%d)", rate);
		return false;
	}

	espeak_SetSynthCallback([](short* wav, int numSamples, espeak_EVENT* events) -> int {
		// wav is null once the utterance is complete
		if(instance == nullptr || wav == nullptr) return 0;
		return instance->synthCallback(wav, numSamples);
	});

	if(!setVoice(Voice)){
		return false;
	}

	CMF_LOG(SpeechGen, LogLevel::Info, "eSpeak ready, voice %s. Free heap after init: internal %zu, PSRAM %zu", Voice,
			heap_caps_get_free_size(MALLOC_CAP_INTERNAL), heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
	return true;
}

bool SpeechGen::setVoice(const char* name){
	const espeak_ERROR err = espeak_SetVoiceByName(name);
	if(err != EE_OK){
		CMF_LOG(SpeechGen, LogLevel::Error, "eSpeak voice %s not found (%d)", name, err);
		return false;
	}

	espeak_SetParameter(espeakRATE, Rate, 0);
	espeak_SetParameter(espeakPITCH, Pitch, 0);
	espeak_SetParameter(espeakVOLUME, Volume, 0);
	return true;
}

void SpeechGen::synthThreadFunc(){
	// eSpeak isn't thread-safe: it is initialized and used from this thread only
	if(!engineInitDone){
		engineInitDone = true;
		engineReady = initEngine();
	}

	xSemaphoreTake(startSem, portMAX_DELAY);

	if(abort || synthInput.empty() || !engineReady){
		synthRunning = false;
		xSemaphoreGive(endSem);
		return;
	}

	CMF_LOG(SpeechGen, LogLevel::Debug, "eSpeak start synth");

	resampler.reset();
	firstChunk = true;

	constexpr unsigned Flags = espeakCHARS_UTF8;
	switch(synthInputType){
		case InputType::Text:
			espeak_Synth(synthInput.c_str(), synthInput.size() + 1, 0, POS_CHARACTER, 0, Flags, nullptr, nullptr);
			break;
		case InputType::Phonemes:
			if(setVoice(PhonemeVoice)){
				espeak_Synth(synthInput.c_str(), synthInput.size() + 1, 0, POS_CHARACTER, 0, Flags | espeakPHONEMES, nullptr, nullptr);
				setVoice(Voice);
			}
			break;
		case InputType::SSML:
			espeak_Synth(synthInput.c_str(), synthInput.size() + 1, 0, POS_CHARACTER, 0, Flags | espeakSSML, nullptr, nullptr);
			break;
	}

	CMF_LOG(SpeechGen, LogLevel::Debug, "eSpeak synth done, stack high water mark %u bytes", (unsigned) uxTaskGetStackHighWaterMark(nullptr));
	synthRunning = false;
	xSemaphoreGive(endSem);
}

int SpeechGen::synthCallback(const int16_t* samples, int count){
	if(abort){
		CMF_LOG(SpeechGen, LogLevel::Debug, "synthCallback abort");
		return 1;
	}

	if(firstChunk){
		firstChunk = false;
		CMF_LOG(SpeechGen, LogLevel::Debug, "eSpeak first audio after %lld ms", (esp_timer_get_time() - requestTime) / 1000);
	}

	if(resampled.size() < Resampler::maxOutput(count)){
		resampled.resize(Resampler::maxOutput(count));
	}
	const size_t out = resampler.process(samples, count, resampled.data());

	// Ringbuffer only takes items up to its own size, send in pieces
	constexpr size_t MaxPiece = BufferSize / 2 / sizeof(int16_t);
	for(size_t sent = 0; sent < out; sent += MaxPiece){
		const size_t piece = std::min(MaxPiece, out - sent);
		while(xRingbufferSend(ringbuf, &resampled[sent], piece * sizeof(int16_t), portMAX_DELAY) != pdTRUE);
	}

	CMF_LOG(SpeechGen, LogLevel::Debug, "write to ringbuffer %zu bytes", out * sizeof(int16_t));

	return 0;
}

void SpeechGen::checkAndAbort(){
	if(!synthRunning){
		//always take endSem just in case of double-give overflowing the underlying freertos queue
		xSemaphoreTake(endSem, 0);
		//flush any samples left over from a completed (non-aborted) synthesis
		size_t sz;
		void* p;
		while((p = xRingbufferReceiveUpTo(ringbuf, &sz, 0, BufferSize)) != nullptr){
			vRingbufferReturnItem(ringbuf, p);
		}
		CMF_LOG(SpeechGen, LogLevel::Debug, "check&abort !synthRunning");
		return;
	}

	//If semaphore is taken that means speech synthesis is running, needs to be aborted and buffer cleaned.
	CMF_LOG(SpeechGen, LogLevel::Debug, "Aborting previously ongoing synthesis");
	abort = true;

	//Drain the ringbuffer until the synth thread acknowledges the abort
	size_t size;
	void* p;
	while(xSemaphoreTake(endSem, pdMS_TO_TICKS(10)) != pdTRUE){
		while((p = xRingbufferReceiveUpTo(ringbuf, &size, 0, BufferSize)) != nullptr){
			vRingbufferReturnItem(ringbuf, p);
		}
	}

	//consume leftover content in buffer
	while((p = xRingbufferReceiveUpTo(ringbuf, &size, 0, BufferSize)) != nullptr){
		vRingbufferReturnItem(ringbuf, p);
	}
	CMF_LOG(SpeechGen, LogLevel::Debug, "Purged leftover data in buffer. Remaining %d", xRingbufferGetCurFreeSize(ringbuf));
	//at this point synthesis is stopped and buffer is empty

	abort = false;
}


std::string SpeechGen::CMUToESpeak(const std::string& cmu){
	// CMU ARPAbet -> eSpeak English phoneme mnemonics (phsource/ph_english)
	static const std::unordered_map<std::string, std::string> vowels = {
		{ "AA", "A:" }, { "AE", "a" }, { "AO", "O:" }, { "AW", "aU" },
		{ "AY", "aI" }, { "EH", "E" }, { "EY", "eI" }, { "IH", "I" },
		{ "IY", "i:" }, { "OW", "oU" }, { "OY", "OI" }, { "UH", "U" },
		{ "UW", "u:" }
	};

	static const std::unordered_map<std::string, std::string> consonants = {
		{ "B", "b" }, { "CH", "tS" }, { "D", "d" }, { "DH", "D" },
		{ "F", "f" }, { "G", "g" }, { "HH", "h" }, { "JH", "dZ" },
		{ "K", "k" }, { "L", "l" }, { "M", "m" }, { "N", "n" },
		{ "NG", "N" }, { "P", "p" }, { "R", "r" }, { "S", "s" },
		{ "SH", "S" }, { "T", "t" }, { "TH", "T" }, { "V", "v" },
		{ "W", "w" }, { "Y", "j" }, { "Z", "z" }, { "ZH", "Z" }
	};

	std::string result = "[[";
	bool wordStart = true;
	bool stressed = false;

	// multinetToCMU separates phonemes with one space and words with two
	size_t i = 0;
	while(i <= cmu.size()){
		size_t end = cmu.find(' ', i);
		if(end == std::string::npos) end = cmu.size();
		std::string token = cmu.substr(i, end - i);
		i = end + 1;

		if(token.empty()){
			if(!wordStart){
				result += ' ';
				wordStart = true;
				stressed = false;
			}
			continue;
		}

		if(std::isdigit((unsigned char) token.back())){
			token.pop_back();
		}

		// Multinet carries no stress, so stress the first vowel of each word
		if(token == "AH" || token == "ER"){
			if(!stressed){
				result += token == "AH" ? "'V" : "'3:";
				stressed = true;
			}else{
				result += token == "AH" ? "@" : "3";
			}
		}else if(auto v = vowels.find(token); v != vowels.end()){
			if(!stressed){
				result += '\'';
				stressed = true;
			}
			result += v->second;
		}else if(auto c = consonants.find(token); c != consonants.end()){
			result += c->second;
		}else{
			continue; // <UNK>
		}
		wordStart = false;
	}

	result += "]]";
	return result;
}

std::string SpeechGen::multinetToCMU(const std::string& input){
	// Inverted mapping: single symbol → representative CMU phoneme
	static const std::unordered_map<char, std::string> invertedAlphabet = {
		{ 'a', "AE0" },
		{ 'b', "OW0" },
		{ 'c', "AH0" },
		{ 'd', "EY0" },
		{ 'e', "AO0" },
		{ 'f', "EH0" },
		{ 'g', "IH0" },
		{ 'h', "HH" },
		{ 'i', "AY0" },
		{ 'j', "DH" },
		{ 'k', "ER0" },
		{ 'l', "NG" },
		{ 'm', "IY0" },
		{ 'n', "AA0" },
		{ 'o', "UW0" },
		{ 'p', "CH" },
		{ 'q', "JH" },
		{ 'r', "ZH" },
		{ 's', "SH" },
		{ 't', "AW0" },
		{ 'u', "OY0" },
		{ 'v', "TH" },
		{ 'w', "UH0" },
		// Direct single-letter CMU phonemes
		{ 'N', "N" },
		{ 'V', "V" },
		{ 'L', "L" },
		{ 'F', "F" },
		{ 'S', "S" },
		{ 'B', "B" },
		{ 'R', "R" },
		{ 'D', "D" },
		{ 'G', "G" },
		{ 'K', "K" },
		{ 'W', "W" },
		{ 'T', "T" },
		{ 'M', "M" },
		{ 'Z', "Z" },
		{ 'P', "P" },
		{ 'Y', "Y" },
		{ ' ', " " }
	};


	std::string result;
	result.reserve(input.size() * 4); // rough optimization

	for(char c : input){
		auto it = invertedAlphabet.find(c);
		if(it != invertedAlphabet.end()){
			if(!result.empty() && result.back() != ' ')
				result += ' ';
			result += it->second;
		} else{
			// Unknown symbol — skip or mark
			if(!result.empty() && result.back() != ' ')
				result += ' ';
			result += "<UNK>";
		}
	}

	return result;
}
