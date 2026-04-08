cmake_minimum_required(VERSION 3.22)
set(TARGET "" CACHE STRING "Target name")

set(CMAKE_EXECUTABLE_SUFFIX_C   ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_ASM ".elf")

set(PROJECT_NAME "OpenDeck")

include(ExternalProject)
include(FetchContent)
include($ENV{ZEPHYR_PROJECT}/cmake/include/helpers.cmake)
include($ENV{ZEPHYR_WS}/modules/libcore/cmake/ccache.cmake)

execute_process(
    COMMAND
    "$ENV{ZEPHYR_PROJECT}/scripts/gen/gen_target.sh"
    "--project=${PROJECT_NAME}"
    "--target-config=$ENV{ZEPHYR_PROJECT}/config/target/${TARGET}.yml"
    "--gen-dir-target=$ENV{ZEPHYR_PROJECT}/app/src/generated/target/${TARGET}"
    "--touchscreen-config=$ENV{ZEPHYR_PROJECT}/config/touchscreen/${TARGET}.json"
    "--gen-dir-touchscreen=$ENV{ZEPHYR_PROJECT}/app/src/generated/application/${TARGET}"
    "--base-gen-dir-mcu=$ENV{ZEPHYR_PROJECT}/app/src/generated/mcu"
)

project(${PROJECT_NAME})
enable_language(C CXX ASM)
enable_testing()
add_subdirectory(src)