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
	TaskHandle_t task = nullptr;
	std::atomic<bool> haConnected = false;

	int client = -1;
	char rxBuf[512];
	size_t rxLen = 0;
	size_t rxPos = 0;

	static void taskFunc(void* arg);
	void serve();
	void handleClient();
	bool handleEvent(const std::string& type, cJSON* data);

	bool readLine(std::string& line);
	bool readExact(void* dst, size_t len);
	bool fillRx();

	bool send(const char* type, const char* dataJson = nullptr);
};

#endif //BUTTERBOT_FIRMWARE_WYOMING_H
