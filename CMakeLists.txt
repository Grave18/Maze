cmake_minimum_required(VERSION 3.30)
project(maze)

set(CMAKE_CXX_STANDARD 20)

add_executable(maze src/main.cpp)

find_package(raylib CONFIG REQUIRED)
target_link_libraries(maze PRIVATE raylib)