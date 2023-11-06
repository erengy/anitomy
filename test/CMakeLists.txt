add_executable(anitomy-tests
	main.cpp
)

target_link_libraries(anitomy-tests anitomy)

if (MSVC)
	target_compile_options(anitomy-tests PRIVATE
		/guard:cf
		/MP
		/permissive-
		/utf-8
		/W3
		/Zc:__cplusplus
	)
else()
	target_compile_options(anitomy-tests PRIVATE
		-Wall
		-Wextra
	)
endif()
