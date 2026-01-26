add_executable(anitomy-tests
	main.cpp
)

set_target_properties(anitomy-tests PROPERTIES
	OUTPUT_NAME test
)

target_link_libraries(anitomy-tests PRIVATE
	anitomy
	anitomy-config
)
