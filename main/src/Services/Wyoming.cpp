#include "Wyoming.h"
#include <algorithm>
#include <cstring>
#include <cJSON.h>
#include <esp_heap_caps.h>
#include <freertos/idf_additions.h>
#include <lwip/sockets.h>
#include <mdns.h>
#include <Log/Log.h>

DEFINE_LOG(Wyoming)

// Protocol version we speak, the one HA's integration pins
static constexpr const char* ProtocolVersion = "1.10.2";

static constexpr const char* InfoJson = R"({)"
	R"("asr":[],"tts":[],"handle":[],"intent":[],"wake":[],)"
	R"("mic":[{"name":"butterbot-mic","attribution":{"name":"CircuitMess","url":"https://circuitmess.com"},"installed":true,"description":"Butter Bot microphones","version":null,)"
		R"("mic_format":{"rate":16000,"width":2,"channels":1}}],)"
	R"("snd":[{"name":"butterbot-speaker","attribution":{"name":"CircuitMess","url":"https://circuitmess.com"},"installed":true,"description":"Butter Bot speaker","version":null,)"
		R"("snd_format":{"rate":16000,"width":2,"channels":1}}],)"
	R"("satellite":{"name":"Butter Bot","attribution":{"name":"CircuitMess","url":"https://circuitmess.com"},"installed":true,"description":"Butter Bot desk robot","version":"2",)"
		R"("area":null,"has_vad":false,"active_wake_words":[],"max_active_wake_words":0,"supports_trigger":false})"
	R"(})";

// HA pings every ~2 s once connected and drops us after 5 s of silence
static constexpr int RecvTimeoutS = 15;
static constexpr size_t MaxHeaderLen = 2048;
static constexpr size_t MaxDataLen = 4096;

void Wyoming::start(){
	if(task != nullptr) return;

	mdns_init();
	mdns_hostname_set("butterbot");
	mdns_instance_name_set("Butter Bot");
	mdns_service_add("Butter Bot", "_wyoming", "_tcp", Port, nullptr, 0);

	// PSRAM stack keeps internal RAM free; this task never touches flash
	xTaskCreatePinnedToCoreWithCaps(&Wyoming::taskFunc, "Wyoming", 6 * 1024, this, 4, &task, 0, MALLOC_CAP_SPIRAM);
}

bool Wyoming::isHaConnected() const{
	return haConnected;
}

void Wyoming::taskFunc(void* arg){
	static_cast<Wyoming*>(arg)->serve();
}

void Wyoming::serve(){
	const int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	const int one = 1;
	setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(Port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(server, (sockaddr*) &addr, sizeof(addr)) != 0 || listen(server, 1) != 0){
		CMF_LOG(Wyoming, LogLevel::Error, "Can't listen on port %d (errno %d)", Port, errno);
		close(server);
		vTaskDelete(nullptr);
		return;
	}

	CMF_LOG(Wyoming, LogLevel::Info, "Listening on port %d", Port);

	while(true){
		client = accept(server, nullptr, nullptr);
		if(client < 0){
			vTaskDelay(pdMS_TO_TICKS(1000));
			continue;
		}

		timeval tv = { .tv_sec = RecvTimeoutS, .tv_usec = 0 };
		setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
		setsockopt(client, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

		rxLen = rxPos = 0;
		handleClient();

		if(haConnected){
			CMF_LOG(Wyoming, LogLevel::Info, "HA disconnected");
			haConnected = false;
		}

		close(client);
		client = -1;
	}
}

void Wyoming::handleClient(){
	std::string line;
	std::string dataBuf;

	while(readLine(line)){
		cJSON* header = cJSON_Parse(line.c_str());
		if(header == nullptr){
			CMF_LOG(Wyoming, LogLevel::Warning, "Bad event header");
			return;
		}

		const cJSON* typeItem = cJSON_GetObjectItem(header, "type");
		const std::string type = cJSON_IsString(typeItem) ? typeItem->valuestring : "";
		const cJSON* dataLenItem = cJSON_GetObjectItem(header, "data_length");
		const cJSON* payloadLenItem = cJSON_GetObjectItem(header, "payload_length");
		const size_t dataLen = cJSON_IsNumber(dataLenItem) ? (size_t) dataLenItem->valuedouble : 0;
		size_t payloadLen = cJSON_IsNumber(payloadLenItem) ? (size_t) payloadLenItem->valuedouble : 0;

		// Data can sit inline in the header, in a separate block, or both (merged)
		cJSON* data = cJSON_DetachItemFromObject(header, "data");
		cJSON_Delete(header);
		if(data != nullptr && !cJSON_IsObject(data)){
			cJSON_Delete(data);
			data = nullptr;
		}

		if(dataLen > 0){
			if(dataLen > MaxDataLen){
				cJSON_Delete(data);
				return;
			}
			dataBuf.resize(dataLen);
			if(!readExact(dataBuf.data(), dataLen)){
				cJSON_Delete(data);
				return;
			}
			cJSON* extra = cJSON_ParseWithLength(dataBuf.data(), dataLen);
			if(data == nullptr){
				data = extra;
			}else if(extra != nullptr){
				cJSON* item = extra->child;
				while(item != nullptr){
					cJSON* next = item->next;
					cJSON_AddItemToObject(data, item->string, cJSON_DetachItemViaPointer(extra, item));
					item = next;
				}
				cJSON_Delete(extra);
			}
		}

		// Payloads (audio) aren't used yet: skip them
		uint8_t sink[256];
		while(payloadLen > 0){
			const size_t n = std::min(payloadLen, sizeof(sink));
			if(!readExact(sink, n)){
				cJSON_Delete(data);
				return;
			}
			payloadLen -= n;
		}

		const bool ok = handleEvent(type, data);
		cJSON_Delete(data);
		if(!ok) return;
	}
}

bool Wyoming::handleEvent(const std::string& type, cJSON* data){
	if(type == "describe"){
		return send("info", InfoJson);
	}

	if(type == "ping"){
		const cJSON* text = cJSON_GetObjectItem(data, "text");
		if(cJSON_IsString(text)){
			cJSON* reply = cJSON_CreateObject();
			cJSON_AddStringToObject(reply, "text", text->valuestring);
			char* json = cJSON_PrintUnformatted(reply);
			const bool ok = send("pong", json);
			cJSON_free(json);
			cJSON_Delete(reply);
			return ok;
		}
		return send("pong");
	}

	if(type == "run-satellite"){
		haConnected = true;
		CMF_LOG(Wyoming, LogLevel::Info, "HA connected, free internal %u B", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
		return true;
	}

	if(type == "pause-satellite"){
		haConnected = false;
		CMF_LOG(Wyoming, LogLevel::Info, "HA paused us");
		return true;
	}

	CMF_LOG(Wyoming, LogLevel::Info, "Ignoring event %s", type.c_str());
	return true;
}

bool Wyoming::send(const char* type, const char* dataJson){
	char header[160];
	int len;
	if(dataJson != nullptr){
		len = snprintf(header, sizeof(header), R"({"type":"%s","version":"%s","data_length":%u})" "\n", type, ProtocolVersion, (unsigned) strlen(dataJson));
	}else{
		len = snprintf(header, sizeof(header), R"({"type":"%s","version":"%s"})" "\n", type, ProtocolVersion);
	}

	auto sendAll = [this](const char* buf, size_t n){
		while(n > 0){
			const int sent = ::send(client, buf, n, 0);
			if(sent <= 0) return false;
			buf += sent;
			n -= sent;
		}
		return true;
	};

	if(!sendAll(header, len)) return false;
	if(dataJson != nullptr && !sendAll(dataJson, strlen(dataJson))) return false;
	return true;
}

bool Wyoming::fillRx(){
	const int n = recv(client, rxBuf, sizeof(rxBuf), 0);
	if(n <= 0) return false;
	rxLen = n;
	rxPos = 0;
	return true;
}

bool Wyoming::readLine(std::string& line){
	line.clear();
	while(true){
		if(rxPos >= rxLen && !fillRx()) return false;

		const char* start = rxBuf + rxPos;
		const char* nl = (const char*) memchr(start, '\n', rxLen - rxPos);
		if(nl != nullptr){
			line.append(start, nl - start);
			rxPos += (nl - start) + 1;
			return true;
		}

		line.append(start, rxLen - rxPos);
		rxPos = rxLen;
		if(line.size() > MaxHeaderLen) return false;
	}
}

bool Wyoming::readExact(void* dst, size_t len){
	auto out = static_cast<uint8_t*>(dst);
	while(len > 0){
		if(rxPos >= rxLen && !fillRx()) return false;
		const size_t n = std::min(len, rxLen - rxPos);
		memcpy(out, rxBuf + rxPos, n);
		rxPos += n;
		out += n;
		len -= n;
	}
	return true;
}
