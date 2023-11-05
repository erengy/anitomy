add_executable(anitomy-cli
	main.cpp
)

set_target_properties(anitomy-cli PROPERTIES
	OUTPUT_NAME anitomy
)

target_link_libraries(anitomy-cli anitomy)

if (MSVC)
	target_compile_options(anitomy-cli PRIVATE
		/guard:cf
		/MP
		/permissive-
		/utf-8
		/W3
		/Zc:__cplusplus
	)
else()
	target_compile_options(anitomy-cli PRIVATE
		-Wall
		-Wextra
	)
endif()
