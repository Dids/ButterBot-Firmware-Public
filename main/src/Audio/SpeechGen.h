#ifndef BUTTERBOT_FIRMWARE_SPEECHGEN_H
#define BUTTERBOT_FIRMWARE_SPEECHGEN_H

#include <atomic>
#include <memory>
#include <span>
#include <string>
#include <vector>
#include "freertos/ringbuf.h"
#include "Thread/Threaded.h"
#include "Resampler.h"

/**
 * eSpeak thread, koji vrti TTS
 * callback puni ringbuffer/queue
 * generate vuče iz buffera/queuea, šalje dalje, odblokirava eSpeak thread (ako je napunjen buff)
 *
 * ako je currently running TTS thread, svaki novi parse poziv prazni buffer i callback vraća 1 (abort)
 *
 * eSpeak outputs 22050 Hz, resampled to 16 kHz before it goes into the ringbuffer.
 */
class SpeechGen {
public:
	SpeechGen();
	virtual ~SpeechGen();
	/**
	 * Retrieves speech synthesis audio.
	 * @param output std::span to be filled with speech synthesis data
	 * @param last output parameter, true = last output of current synthesis reached, false = current synthesis still ongoing
	 * @param timeout maximum time to wait for data, 0 for non-blocking call
	 * @return size of retrieved data, in samples; zero if synthesis pending or not started
	 */
	int generate(std::span<int16_t> output, bool& last, TickType_t timeout = 0);

	/**
	 * Initializes speech synthesis with given text.
	 * Aborts current synthesis and empties the buffer if synthesis is currently running.
	 * @param text UTF-8 text to be synthesized, in the language of the configured voice (Finnish)
	 */
	void parseText(std::string text);

	/**
	 * Initializes speech synthesis with given phonemes.
	 * Aborts current synthesis and empties the buffer if synthesis is currently running.
	 * Spoken with the English voice, since the phonemes come from the English speech recognizer.
	 * @param phonemes Phonemes written in ESP-SR's Multinet format (best obtained directly from AudioFrontend's speech mode)
	 */
	void parsePhonemes(const std::string& phonemes);

	/**
	 * Initializes speech synthesis with given SSML string (https://en.wikipedia.org/wiki/Speech_Synthesis_Markup_Language)
	 * Aborts current synthesis and empties the buffer if synthesis is currently running.
	 * @param ssml SSML string
	 */
	void parseSSML(std::string ssml);

	/**
	 * Cancels ongoing synthesis and purges buffers.
	 */
	void cancel();

	enum class InputType : uint8_t {
		Text, Phonemes, SSML
	};

private:
	static constexpr size_t BufferSize = 4 * 1024;

	std::unique_ptr<Threaded> synthThread;

	void synthThreadFunc();

	bool initEngine();
	bool setVoice(const char* name);

	int synthCallback(const int16_t* samples, int count);

	SemaphoreHandle_t startSem;
	SemaphoreHandle_t endSem;

	RingbufHandle_t ringbuf;

	Resampler resampler;
	std::vector<int16_t> resampled;

	std::string synthInput;

	InputType synthInputType = InputType::Text;

	bool engineInitDone = false;
	bool engineReady = false;

	//Timestamp of the last parse call, for time-to-first-sound logging
	std::atomic<int64_t> requestTime = 0;
	bool firstChunk = false;

	//Aborts eSpeak's synthesis (makes the synth callback return 1)
	std::atomic_bool abort = false;
	std::atomic_bool synthRunning = false;

	void checkAndAbort();

	static std::string CMUToESpeak(const std::string& cmu);
	static std::string multinetToCMU(const std::string& input);
};


#endif //BUTTERBOT_FIRMWARE_SPEECHGEN_H
