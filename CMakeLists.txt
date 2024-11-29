cmake_minimum_required(VERSION 3.22)

project(cacompress)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(/W4 /constexpr:steps1000000000)

include(Packages.cmake)
add_subdirectory(src)
