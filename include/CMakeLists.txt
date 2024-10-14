add_library(anitomy INTERFACE)

target_sources(anitomy PUBLIC
    FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}
        FILES anitomy.hpp
)
