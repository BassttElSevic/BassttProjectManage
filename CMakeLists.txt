cmake_minimum_required(VERSION 3.20)
project(untitled C)

set(CMAKE_C_STANDARD 11)

if(MSVC)
    add_compile_options(/utf-8)
endif()

add_executable(untitled WIN32 main.c)

if(WIN32)
    target_link_libraries(untitled comctl32)
endif()
