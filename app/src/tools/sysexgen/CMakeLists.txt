cmake_minimum_required(VERSION 3.22)
set(TARGET "" CACHE STRING "Target name")

project(sysexgen)
enable_language(CXX)

include($ENV{ZEPHYR_WS}/modules/libcore/cmake/ccache.cmake)
include($ENV{ZEPHYR_PROJECT}/app/src/generated/target/${TARGET}/CMakeLists.txt)

add_executable(sysexgen)

target_include_directories(sysexgen
    PRIVATE
    $ENV{ZEPHYR_PROJECT}/app/src/firmware
    $ENV{ZEPHYR_WS}/modules/liblessdb/include
    $ENV{ZEPHYR_WS}/modules/libsysexconf/include
)

target_sources(sysexgen
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

target_compile_definitions(sysexgen
    PRIVATE
    ${PROJECT_TARGET_DEFINES}
)