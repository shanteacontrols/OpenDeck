cmake_minimum_required(VERSION 3.22)
set(TARGET "" CACHE STRING "Target name")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(OpenDeck)
enable_language(C CXX ASM)

add_subdirectory(src)
