#ifndef BUTTERBOT_FIRMWARE_WYOMING_H
#define BUTTERBOT_FIRMWARE_WYOMING_H

#include <atomic>
#include <cstddef>
#include <string>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

struct cJSON;

// Home Assistant's voice satellite protocol: HA connects to us over TCP and sends
// one JSON line per event, optionally followed by more JSON and a binary payload.
class Wyoming {
public:
	static constexpr uint16_t Port = 10700;

	void start();
	bool isHaConnected() const;

private:
	// One connection. HA keeps one open for the satellite and opens short extra ones
	// every 30 s to re-read our info, so several must be served at once.
	struct Session {
		Wyoming* owner = nullptr;
		int fd = -1;
		bool satellite = false;
		char rxBuf[512];
		size_t rxLen = 0;
		size_t rxPos = 0;

		void run();
		bool handleEvent(const std::string& type, cJSON* data);
		bool readLine(std::string& line);
		bool readExact(void* dst, size_t len);
		bool fillRx();
		bool send(const char* type, const char* dataJson = nullptr);
	};

	static constexpr size_t NumSessions = 3;

	int server = -1;
	Session sessions[NumSessions];
	std::atomic<bool> haConnected = false;

	static void workerFunc(void* arg);
};

#endif //BUTTERBOT_FIRMWARE_WYOMING_H
