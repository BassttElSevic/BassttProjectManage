cmake_minimum_required(VERSION 3.20)
project(untitled C)

set(CMAKE_C_STANDARD 11)

if(MSVC)
    add_compile_options(/utf-8)
endif()

# 添加头文件目录
include_directories(Inc)

# 收集所有源文件
set(SOURCES
    main.c
    Src/task_manager.c
    Src/animation.c
    Src/ui_controls.c
    Src/window_proc.c
)

# 添加资源文件
add_executable(untitled WIN32 ${SOURCES} resource.rc)

if(WIN32)
    target_link_libraries(untitled comctl32)
endif()
