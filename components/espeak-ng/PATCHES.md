# eSpeak-NG for ButterBot

eSpeak-NG 1.52 (GPL-3, see `COPYING`), vendored from the Arduino port
https://github.com/pschatzmann/arduino-espeak-ng at commit `3b01622`.

## What was taken

- `src/libespeak-ng/*` except `mem_alloc_rp2350.*`
- `src/ucd/*`
- `src/espeak-ng/{espeak_ng.h,speak_lib.h,encoding.h}` -> `include/espeak-ng/`
- `COPYING*` licence files
- `data/`: files from the port's `espeak-ng-data/`, stored under flat names
  (`lang/urj/fi` -> `lang/fi`, `lang/gmw/en` -> `lang/en`) so `LoadVoice()` finds them
  without listing directories

Not taken: `arduino/` (Arduino wrapper, audio, posix-fs glue), `src/pcaudiolib`,
`src/data/*.h`, `src/espeak-ng-data.h`, the port's `config-espk.h`.

## Changes against the port

- `src/libespeak-ng/common.c`: the three `#ifdef ARDUINO` blocks in `IsAlpha()`,
  `IsDigit()` and `IsSpace()` are `#if defined(ARDUINO) || defined(ESP_PLATFORM)`.
  newlib's `iswalpha()` is ASCII-only, so without this ä/ö are not letters and Finnish
  words get split.

## Added files

- `include/config-espk.h`: ESP-IDF build config, replaces the port's (which pulls in
  Arduino headers). No posix-fs, no pcaudio, no async, Klatt enabled for the
  klatt/robosoft variants, `ESPK_LOG` at ESP-IDF debug level.
- `include/direntx.h`: plain `<dirent.h>`.
- `include/espeak_data.h`, `espeak_data.c`: maps the `espeak` flash partition, mounts it
  read-only at `/espeak` and implements `espeak_mem_map()` so the phoneme tables and
  dictionaries are read in place from flash.
- `tools/pack_data.py`: builds the partition image from `data/` at build time.
