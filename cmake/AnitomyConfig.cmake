add_library(anitomy-config INTERFACE)

if (MSVC)
	target_compile_options(anitomy-config INTERFACE
		/guard:cf
		/MP
		/permissive-
		/utf-8
		/W3
		/Zc:__cplusplus
	)
else()
	target_compile_options(anitomy-config INTERFACE
		-Wall
		-Wextra
	)
endif()
