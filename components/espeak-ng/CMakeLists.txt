file(GLOB ESPEAK_SRCS "src/libespeak-ng/*.c" "src/ucd/*.c")

idf_component_register(SRCS ${ESPEAK_SRCS}
		INCLUDE_DIRS "include"
		PRIV_INCLUDE_DIRS "src" "src/libespeak-ng"
		PRIV_REQUIRES heap log)

# Upstream code, keep the build quiet without patching it
target_compile_options(${COMPONENT_LIB} PRIVATE
		-Wno-format-overflow -Wno-format-truncation
		-Wno-char-subscripts -Wno-implicit-fallthrough -Wno-restrict)
