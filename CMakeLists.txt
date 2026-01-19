cmake_minimum_required(VERSION 3.20)
project(untitled C)

set(CMAKE_C_STANDARD 11)



# 静态链接运行时库（添加在这里）
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static")
endif()

if(MSVC)
    add_compile_options(/utf-8)
endif()

# 添加头文件目录
include_directories(Inc)

# Define the minimum required Windows version
add_definitions(-D_WIN32_WINNT=0x0600)

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
    target_link_libraries(untitled comctl32 uxtheme)
endif()
