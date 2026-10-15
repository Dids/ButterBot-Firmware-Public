#ifndef BUTTERBOT_FIRMWARE_HOMENET_H
#define BUTTERBOT_FIRMWARE_HOMENET_H

#include <atomic>
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_timer.h>

// Keeps the robot on the home Wi-Fi. RC mode borrows the same connection for the remote's
// network, because starting Wi-Fi a second time runs out of internal RAM.
class HomeNet {
public:
	HomeNet();
	~HomeNet();

	void start();
	bool isRunning() const;
	bool isConnected() const;

	void joinRemote(const char* ssid, const char* password);
	void rejoinHome();
	// True once the remote's network was joined and then lost
	bool remoteDropped() const;

private:
	enum class Mode : uint8_t { Off, Home, Remote };

	esp_netif_t* netif = nullptr;
	esp_event_handler_instance_t wifiHandler = nullptr;
	esp_event_handler_instance_t ipHandler = nullptr;
	esp_timer_handle_t retryTimer = nullptr;

	std::atomic<Mode> mode = Mode::Off;
	std::atomic<bool> connected = false;
	std::atomic<bool> dropped = false;
	uint32_t retryDelayMs = RetryMinMs;

	static constexpr uint32_t RetryMinMs = 1000;
	static constexpr uint32_t RetryMaxMs = 30000;
	static constexpr uint32_t SwitchDelayMs = 500;

	void switchNetwork(const char* ssid, const char* password);
	void scheduleRetry(uint32_t delayMs);

	static void onWifiEvent(void* arg, esp_event_base_t base, int32_t id, void* data);
	static void onIpEvent(void* arg, esp_event_base_t base, int32_t id, void* data);
	static void onRetry(void* arg);
};

#endif //BUTTERBOT_FIRMWARE_HOMENET_H
