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
    "--target-config=${CMAKE_CURRENT_LIST_DIR}/config/target/${TARGET}.yml"
    "--gen-dir-target=${CMAKE_CURRENT_LIST_DIR}/src/generated/target/${TARGET}"
    "--touchscreen-config=${CMAKE_CURRENT_LIST_DIR}/config/touchscreen/${TARGET}.json"
    "--gen-dir-touchscreen=${CMAKE_CURRENT_LIST_DIR}/src/generated/application/${TARGET}"
    "--touchscreen-img-dir=${CMAKE_CURRENT_LIST_DIR}/bin/touchscreen/img/${TARGET}"
    "--base-gen-dir-mcu=${CMAKE_CURRENT_LIST_DIR}/src/generated/mcu"
)

project(${PROJECT_NAME})
enable_language(C CXX ASM)
enable_testing()
add_subdirectory(src)