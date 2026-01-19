add_library(anitomy INTERFACE)

target_sources(anitomy PUBLIC
    FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}
        FILES anitomy.hpp
)

if(!MSVC)
    if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16)
            target_sources(anitomy PRIVATE
                FILE_SET HEADERS
                BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}
                FILES ranges.hpp
            )
        endif()
    elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 22)
            target_sources(anitomy PRIVATE
                FILE_SET HEADERS
                BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}
                FILES ranges.hpp
            )
        endif()
    endif()
endif()
