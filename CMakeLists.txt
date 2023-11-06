cmake_minimum_required(VERSION 3.20)

project(anitomy
	VERSION 2.0.0
	DESCRIPTION "Anime video filename parser"
	HOMEPAGE_URL https://github.com/erengy/anitomy
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(test)

enable_testing()
add_test(NAME "Unit" COMMAND anitomy-tests)
add_test(NAME "Data" COMMAND anitomy-tests --test-data WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test)
