add_executable(anitomy-cli
	main.cpp
)

set_target_properties(anitomy-cli PROPERTIES
	OUTPUT_NAME anitomy
)

target_link_libraries(anitomy-cli PRIVATE
	anitomy
	anitomy-config
)
