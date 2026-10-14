#pragma once
// eSpeak-NG build configuration for ESP-IDF, based on arduino-espeak-ng's arduino/config-espk.h

#define HAVE_DLFCN_H 0
#define HAVE_ENDIAN_H 0
#define HAVE_FCNTL_H 0
#define HAVE_FORK 0
#define HAVE_GETOPT_H 0
#define HAVE_GETOPT_LONG 0
#define HAVE_GETTIMEOFDAY 0
#define HAVE_INTTYPES_H 1
#define HAVE_LOCALE_H 0
#define HAVE_MALLOC 1
#define HAVE_MEMCHR 1
#define HAVE_MEMMOVE 1
#define HAVE_MEMSET 1
#define HAVE_MKDIR 1
#define HAVE_MKSTEMP 0
#define HAVE_PCAUDIOLIB_AUDIO_H 0
#define HAVE_POW 1
#define HAVE_REALLOC 1
#define HAVE_SETLOCALE 1
#define HAVE_SQRT 1
#define HAVE_STDBOOL_H 1
#define HAVE_STDDEF_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDIO_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRCHR 1
#define HAVE_STRCOLL 0
#define HAVE_STRDUP 1
#define HAVE_STRERROR 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_STRRCHR 1
#define HAVE_STRSTR 1
#define HAVE_SYS_STAT_H 0
#define HAVE_SYS_TIME_H 0
#define HAVE_SYS_TYPES_H 0
#define HAVE_UNISTD_H 0
#define HAVE_VFORK 0
#define HAVE_WCHAR_H 1
#define HAVE_WCTYPE_H 0
#define HAVE_WORKING_FORK 0
#define HAVE_WORKING_VFORK 0

#define PACKAGE "espeak-ng"
#define PACKAGE_BUGREPORT "https://github.com/espeak-ng/espeak-ng/issues"
#define PACKAGE_NAME "eSpeak NG"
#define PACKAGE_STRING "eSpeak NG 1.52-dev"
#define PACKAGE_TARNAME "espeak-ng"
#define PACKAGE_URL "https://github.com/espeak-ng/espeak-ng"
#define PACKAGE_VERSION "1.52-dev"
#define STDC_HEADERS 1
#define VERSION "1.52-dev"

// Synchronous output only, no audio device, no sonic/mbrola
#define USE_ASYNC 0
#define USE_LIBPCAUDIO 0
#define USE_LIBSONIC 0
#define USE_MBROLA 0
#define USE_SPEECHPLAYER 0
// Needed by the klatt* and robosoft voice variants
#define USE_KLATT 1

#ifndef PATH_ESPEAK_DATA
#  define PATH_ESPEAK_DATA "/espeak"
#endif

// ESP32-S3 is little endian
#ifndef le16toh
#  define le16toh(X) (X)
#endif
#ifndef le32toh
#  define le32toh(X) (X)
#endif

#define ESPEAK_ARDUINO_POSIX_FS 0

// Big stack buffers become statics, big tables are allocated on demand
#define ESPEAK_STACK_HACK 1
#define ESPEAK_HEAP_HACK 1
#define ESPEAK_LOGGING 1

#if ESPEAK_STACK_HACK
#  define STACK_T static
#  include <assert.h>
#else
#  define STACK_T
#endif

// Upstream traces every word through this, so keep it at debug level
#if ESPEAK_LOGGING
#  include <esp_log.h>
#  define ESPK_LOG(...) ESP_LOGD("espeak", __VA_ARGS__)
#else
#  define ESPK_LOG(...)
#endif

#ifdef __cplusplus
extern "C" {
#endif
// Implemented in espeak_data.c: maps a packed data file in place from flash
void* espeak_mem_map(const char* path, int* len);
#ifdef __cplusplus
}
#endif

#undef B0

// newlib is built without _MB_CAPABLE, so its isw*() only know ASCII and Ä/Ö would never be
// lower-cased. Route them to eSpeak's own Unicode tables (wctype.h first, so its guard holds).
#include <wctype.h>
#include "ucd/ucd.h"
#define iswalnum(c) ucd_isalnum(c)
#define iswalpha(c) ucd_isalpha(c)
#define iswdigit(c) ucd_isdigit(c)
#define iswlower(c) ucd_islower(c)
#define iswpunct(c) ucd_ispunct(c)
#define iswspace(c) ucd_isspace(c)
#define iswupper(c) ucd_isupper(c)
