cmake_minimum_required(VERSION 3.27.0)
list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND DTS_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(KCONFIG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Kconfig)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
zephyr_get(PRESET_NAME MERGE SYSBUILD GLOBAL)

project(project)

if(NOT DEFINED ENV{TARGET} OR "$ENV{TARGET}" STREQUAL "")
    message(FATAL_ERROR "OpenDeck application requires TARGET to be exported by the makefile")
endif()

set(TARGET "$ENV{TARGET}")

set(OPENDECK_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/../generated)
cmake_path(NORMAL_PATH OPENDECK_GENERATED_DIR)

execute_process(
    COMMAND
    bash
    $ENV{ZEPHYR_PROJECT}/scripts/query_metadata.sh
    target-config
    --kconfig
    ${CMAKE_CURRENT_SOURCE_DIR}/firmware/Kconfig
    --config
    ${CMAKE_CURRENT_BINARY_DIR}/zephyr/.config
    --output
    ${OPENDECK_GENERATED_DIR}/target.conf
    --kconfig-output
    ${OPENDECK_GENERATED_DIR}/target.kconfig
    COMMAND_ERROR_IS_FATAL ANY
)

zephyr_compile_options(-Wno-psabi)
zephyr_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(firmware)

set(ZEPHYR_BIN_FILE             ${CMAKE_CURRENT_BINARY_DIR}/zephyr/zephyr.bin)
set(DFU_PAYLOAD_BIN_FILE        ${CMAKE_CURRENT_BINARY_DIR}/zephyr/zephyr.signed.bin)
set(DFU_OUTPUT_FILE             ${CMAKE_CURRENT_BINARY_DIR}/dfu.bin)

add_custom_target(firmware_dfu ALL
    COMMAND
    ${CMAKE_COMMAND}
    -DINPUT_FILE=${DFU_PAYLOAD_BIN_FILE}
    -DOUTPUT_FILE=${DFU_OUTPUT_FILE}
    -DTARGET_UID=${OPENDECK_TARGET_UID}
    -DFORMAT_VERSION=${OPENDECK_DFU_FORMAT_VERSION}
    -DBEGIN_MAGIC=${OPENDECK_DFU_BEGIN_MAGIC}
    -DEND_MAGIC=${OPENDECK_DFU_END_MAGIC}
    -P $ENV{ZEPHYR_PROJECT}/cmake/dfu_bin.cmake
    DEPENDS app ${DFU_PAYLOAD_BIN_FILE}
    COMMENT "Generating wrapped DFU binary"
    VERBATIM
)
