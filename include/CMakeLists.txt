add_library(anitomy INTERFACE)

target_sources(anitomy PUBLIC
    FILE_SET HEADERS
        BASE_DIRS ${CMAKE_SOURCE_DIR}/include
        FILES anitomy.hpp
)
