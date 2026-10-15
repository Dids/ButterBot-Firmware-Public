#include "HomeNet.h"
#include <algorithm>
#include <cstring>
#include <esp_heap_caps.h>
#include <esp_wifi.h>
#include <Log/Log.h>

#if __has_include("HomeNetSecret.h")
#include "HomeNetSecret.h"
#else
#define HOMENET_SSID ""
#define HOMENET_PASSWORD ""
#endif

DEFINE_LOG(HomeNet)

HomeNet::HomeNet(){
	ESP_ERROR_CHECK(esp_netif_init());

	const esp_err_t err = esp_event_loop_create_default();
	if(err != ESP_OK && err != ESP_ERR_INVALID_STATE){
		ESP_ERROR_CHECK(err);
	}

	esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &HomeNet::onWifiEvent, this, &wifiHandler);
	esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &HomeNet::onIpEvent, this, &ipHandler);

	const esp_timer_create_args_t timerArgs = {
		.callback = &HomeNet::onRetry,
		.arg = this,
		.name = "HomeNetRetry",
	};
	esp_timer_create(&timerArgs, &retryTimer);
}

HomeNet::~HomeNet(){
	if(mode != Mode::Off){
		mode = Mode::Off;
		esp_timer_stop(retryTimer);
		esp_wifi_disconnect();
		esp_wifi_stop();
		esp_wifi_deinit();
		esp_netif_destroy_default_wifi(netif);
	}

	esp_timer_delete(retryTimer);
	esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifiHandler);
	esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, ipHandler);
}

void HomeNet::start(){
	if(strlen(HOMENET_SSID) == 0){
		CMF_LOG(HomeNet, LogLevel::Warning, "No home Wi-Fi set (main/src/Services/HomeNetSecret.h), staying offline");
		return;
	}

	netif = esp_netif_create_default_wifi_sta();
	esp_netif_set_hostname(netif, "butterbot");

	const wifi_init_config_t initCfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&initCfg));

	wifi_config_t cfg = {};
	strncpy(reinterpret_cast<char*>(cfg.sta.ssid), HOMENET_SSID, sizeof(cfg.sta.ssid));
	strncpy(reinterpret_cast<char*>(cfg.sta.password), HOMENET_PASSWORD, sizeof(cfg.sta.password));

	mode = Mode::Home;

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &cfg));
	ESP_ERROR_CHECK(esp_wifi_start());
}

bool HomeNet::isRunning() const{
	return mode != Mode::Off;
}

bool HomeNet::isConnected() const{
	return connected;
}

void HomeNet::joinRemote(const char* ssid, const char* password){
	if(mode == Mode::Off) return;

	CMF_LOG(HomeNet, LogLevel::Info, "Switching to the remote's network");
	mode = Mode::Remote;
	dropped = false;

	// The remote's network has no DHCP; the feed expects this fixed address
	esp_netif_dhcpc_stop(netif);
	esp_netif_ip_info_t ip = {};
	ip.ip.addr = esp_ip4addr_aton("11.0.0.2");
	ip.netmask.addr = esp_ip4addr_aton("255.255.255.0");
	ip.gw.addr = esp_ip4addr_aton("11.0.0.1");
	esp_netif_set_ip_info(netif, &ip);

	switchNetwork(ssid, password);
}

void HomeNet::rejoinHome(){
	if(mode != Mode::Remote) return;

	CMF_LOG(HomeNet, LogLevel::Info, "Back to home Wi-Fi");
	mode = Mode::Home;
	retryDelayMs = RetryMinMs;

	esp_netif_dhcpc_start(netif);
	switchNetwork(HOMENET_SSID, HOMENET_PASSWORD);
}

bool HomeNet::remoteDropped() const{
	return dropped;
}

void HomeNet::switchNetwork(const char* ssid, const char* password){
	connected = false;
	esp_timer_stop(retryTimer);
	esp_wifi_disconnect();

	wifi_config_t cfg = {};
	strncpy(reinterpret_cast<char*>(cfg.sta.ssid), ssid, sizeof(cfg.sta.ssid));
	strncpy(reinterpret_cast<char*>(cfg.sta.password), password, sizeof(cfg.sta.password));
	esp_wifi_set_config(WIFI_IF_STA, &cfg);

	scheduleRetry(SwitchDelayMs);
}

void HomeNet::scheduleRetry(uint32_t delayMs){
	if(esp_timer_is_active(retryTimer)) return;
	esp_timer_start_once(retryTimer, (uint64_t) delayMs * 1000);
}

void HomeNet::onWifiEvent(void* arg, esp_event_base_t base, int32_t id, void* data){
	auto self = static_cast<HomeNet*>(arg);
	const Mode mode = self->mode;
	if(mode == Mode::Off) return;

	if(id == WIFI_EVENT_STA_START){
		esp_wifi_connect();
	}else if(id == WIFI_EVENT_STA_CONNECTED && mode == Mode::Remote){
		CMF_LOG(HomeNet, LogLevel::Info, "On the remote's network");
		self->connected = true;
	}else if(id == WIFI_EVENT_STA_DISCONNECTED){
		const auto info = static_cast<wifi_event_sta_disconnected_t*>(data);
		const bool wasConnected = self->connected.exchange(false);

		if(mode == Mode::Remote){
			if(wasConnected){
				CMF_LOG(HomeNet, LogLevel::Warning, "Lost the remote's network (reason %d)", info->reason);
				self->dropped = true;
			}else{
				self->scheduleRetry(RetryMinMs);
			}
			return;
		}

		CMF_LOG(HomeNet, LogLevel::Warning, "Disconnected (reason %d), retrying in %lu ms", info->reason, self->retryDelayMs);
		self->scheduleRetry(self->retryDelayMs);
		self->retryDelayMs = std::min(self->retryDelayMs * 2, RetryMaxMs);
	}
}

void HomeNet::onIpEvent(void* arg, esp_event_base_t base, int32_t id, void* data){
	auto self = static_cast<HomeNet*>(arg);
	if(self->mode != Mode::Home) return;

	const auto info = static_cast<ip_event_got_ip_t*>(data);
	self->connected = true;
	self->retryDelayMs = RetryMinMs;

	CMF_LOG(HomeNet, LogLevel::Info, "Connected, IP " IPSTR ", free internal %u B (largest %u B)", IP2STR(&info->ip_info.ip),
			heap_caps_get_free_size(MALLOC_CAP_INTERNAL), heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
}

void HomeNet::onRetry(void* arg){
	auto self = static_cast<HomeNet*>(arg);
	if(self->mode == Mode::Off) return;

	esp_wifi_connect();
}
