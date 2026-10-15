file(GLOB ESPEAK_SRCS "src/libespeak-ng/*.c" "src/ucd/*.c")

idf_component_register(SRCS ${ESPEAK_SRCS} "espeak_data.c"
		INCLUDE_DIRS "include"
		PRIV_INCLUDE_DIRS "src" "src/libespeak-ng"
		PRIV_REQUIRES heap log esp_partition vfs
		LDFRAGMENTS "linker.lf")

# Upstream code, keep the build quiet without patching it
target_compile_options(${COMPONENT_LIB} PRIVATE
		-Wno-format-overflow -Wno-format-truncation
		-Wno-char-subscripts -Wno-implicit-fallthrough -Wno-restrict)

# Data files are packed into one image and flashed to the "espeak" partition
if(NOT CMAKE_BUILD_EARLY_EXPANSION)
	idf_build_get_property(python PYTHON)
	idf_build_get_property(build_dir BUILD_DIR)
	partition_table_get_partition_info(espeak_size "--partition-name espeak" "size")

	set(ESPEAK_DATA_BIN "${build_dir}/espeak-data.bin")
	file(GLOB_RECURSE ESPEAK_DATA_FILES CONFIGURE_DEPENDS "${COMPONENT_DIR}/data/*")

	add_custom_command(OUTPUT ${ESPEAK_DATA_BIN}
			COMMAND ${python} ${COMPONENT_DIR}/tools/pack_data.py ${COMPONENT_DIR}/data ${ESPEAK_DATA_BIN} --max-size ${espeak_size}
			DEPENDS ${ESPEAK_DATA_FILES} ${COMPONENT_DIR}/tools/pack_data.py
			VERBATIM)
	add_custom_target(espeak_data ALL DEPENDS ${ESPEAK_DATA_BIN})
	add_dependencies(flash espeak_data)

	esptool_py_flash_to_partition(flash espeak ${ESPEAK_DATA_BIN})
endif()
