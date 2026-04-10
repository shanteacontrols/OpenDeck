cmake_minimum_required(VERSION 3.27.0)
list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND DTS_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(KCONFIG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Kconfig)
include($ENV{ZEPHYR_WS}/zenv/cmake/app.cmake)
include(ExternalProject)

project(project)

zephyr_compile_options(-Wno-psabi)
zephyr_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>)

set(SYSEXGEN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/tools/sysexgen)
set(SYSEX_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/../sysexgen)
set(SYSEX_OUTPUT_FILE ${SYSEX_OUTPUT_DIR}/firmware.sysex)
set(ZEPHYR_BIN_FILE ${CMAKE_CURRENT_BINARY_DIR}/zephyr/zephyr.bin)
set(SYSEXGEN_BINARY_DIR ${SYSEX_OUTPUT_DIR}/build)
set(SYSEXGEN_EXECUTABLE ${SYSEXGEN_BINARY_DIR}/sysexgen_tool)

ExternalProject_Add(sysexgen_tool_project
    SOURCE_DIR ${SYSEXGEN_SOURCE_DIR}
    BINARY_DIR ${SYSEXGEN_BINARY_DIR}
    CMAKE_ARGS
        -DPRESET_NAME=${PRESET_NAME}
        -DSYSEXGEN_INPUT_FILE=${ZEPHYR_BIN_FILE}
        -DSYSEXGEN_OUTPUT_FILE=${SYSEX_OUTPUT_FILE}
    BUILD_BYPRODUCTS
        ${SYSEXGEN_EXECUTABLE}
    INSTALL_COMMAND ""
)

add_custom_command(
    OUTPUT ${SYSEX_OUTPUT_FILE}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SYSEX_OUTPUT_DIR}
    COMMAND ${SYSEXGEN_EXECUTABLE}
    DEPENDS
        ${ZEPHYR_BIN_FILE}
        ${SYSEXGEN_EXECUTABLE}
    COMMENT "Generating firmware SysEx image"
    VERBATIM
)

add_custom_target(firmware_sysex ALL
    DEPENDS ${SYSEX_OUTPUT_FILE}
)

add_dependencies(firmware_sysex sysexgen_tool_project)
