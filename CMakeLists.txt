cmake_minimum_required(VERSION 3.20)

project(ark-assets)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(common)
add_subdirectory(platform)
add_subdirectory(archive)
add_subdirectory(unity)
add_subdirectory(playground)
