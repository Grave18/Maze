cmake_minimum_required(VERSION 3.30)
project(maze)

set(CMAKE_CXX_STANDARD 20)

add_executable(maze src/main.cpp)

find_package(raylib CONFIG REQUIRED)
find_path(RAYGUI_INCLUDE_DIRS "raygui.h")
target_include_directories(maze PRIVATE ${RAYGUI_INCLUDE_DIRS})
target_link_libraries(maze PRIVATE raylib)