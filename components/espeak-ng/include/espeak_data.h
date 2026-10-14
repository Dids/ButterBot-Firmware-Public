#pragma once

#include <esp_err.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Maps the "espeak" data partition and mounts it read-only at PATH_ESPEAK_DATA ("/espeak").
 * Must be called before espeak_Initialize(). Safe to call more than once.
 */
esp_err_t espeak_data_mount(void);

#ifdef __cplusplus
}
#endif
