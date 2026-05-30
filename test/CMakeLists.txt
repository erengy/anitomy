add_executable(anitomy-tests
	main.cpp
)

target_link_libraries(anitomy-tests PRIVATE
	anitomy
	anitomy-config
)
