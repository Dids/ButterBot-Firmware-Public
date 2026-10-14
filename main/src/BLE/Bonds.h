#ifndef BUTTERBOT_FIRMWARE_BONDS_H
#define BUTTERBOT_FIRMWARE_BONDS_H

#include <cstring>
#include <memory>
#include <esp_gap_ble_api.h>

namespace BLE {

inline bool isBonded(const esp_bd_addr_t addr){
	int count = esp_ble_get_bond_device_num();
	if(count <= 0) return false;

	auto list = std::make_unique<esp_ble_bond_dev_t[]>(count);
	if(esp_ble_get_bond_device_list(&count, list.get()) != ESP_OK) return false;

	for(int i = 0; i < count; i++){
		if(memcmp(list[i].bd_addr, addr, sizeof(esp_bd_addr_t)) == 0) return true;
	}
	return false;
}

}

#endif //BUTTERBOT_FIRMWARE_BONDS_H
