#ifndef BUTTERBOT_FIRMWARE_HEAPLOG_H
#define BUTTERBOT_FIRMWARE_HEAPLOG_H

#include <esp_heap_caps.h>
#include <esp_log.h>

// Internal RAM is the tight one: Wi-Fi, BLE and DMA can only use it
static inline void logHeap(const char* where){
	ESP_LOGI("Heap", "%-10s int %6u (largest %6u, min ever %6u)  psram %8u", where,
			 (unsigned) heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
			 (unsigned) heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL),
			 (unsigned) heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL),
			 (unsigned) heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
}

#endif //BUTTERBOT_FIRMWARE_HEAPLOG_H
