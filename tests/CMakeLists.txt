cmake_minimum_required(VERSION 3.22)
set(TARGET "" CACHE STRING "Target name")

set(CMAKE_EXECUTABLE_SUFFIX_C   ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_ASM ".elf")

set(PROJECT_NAME "OpenDeck")
set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)

include(ExternalProject)
include(FetchContent)
include(${PROJECT_ROOT}/cmake/include/helpers.cmake)
include(${PROJECT_ROOT}/modules/core/cmake/ccache.cmake)

execute_process(
    COMMAND
    "${CMAKE_CURRENT_LIST_DIR}/scripts/gen/gen_target.sh"
    "--project=${PROJECT_NAME}"
    "--target-config=${PROJECT_ROOT}/config/target/${TARGET}.yml"
    "--gen-dir-target=${PROJECT_ROOT}/src/generated/target/${TARGET}"
    "--touchscreen-config=${PROJECT_ROOT}/config/touchscreen/${TARGET}.json"
    "--gen-dir-touchscreen=${PROJECT_ROOT}/src/generated/application/${TARGET}"
    "--touchscreen-img-dir=${PROJECT_ROOT}/bin/touchscreen/img/${TARGET}"
    "--base-gen-dir-mcu=${PROJECT_ROOT}/src/generated/mcu"
)

project(${PROJECT_NAME})
enable_language(C CXX ASM)
enable_testing()
add_subdirectory(src)