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

zephyr_compile_options(-Wno-psabi)
zephyr_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/bootloader/src)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/common)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/firmware/src)

add_subdirectory(firmware)

set(ZEPHYR_BIN_FILE             ${CMAKE_CURRENT_BINARY_DIR}/zephyr/zephyr.bin)
set(ZEPHYR_DTS_FILE             ${CMAKE_CURRENT_BINARY_DIR}/zephyr/zephyr.dts)
set(APP_VALIDATED_BIN_FILE      ${CMAKE_CURRENT_BINARY_DIR}/app_validated.bin)
set(APP_VALIDATED_HEX_FILE      ${CMAKE_CURRENT_BINARY_DIR}/app_validated.hex)
set(APP_VALIDATED_INFO_FILE     ${CMAKE_CURRENT_BINARY_DIR}/app_validated_info.h)
set(DFU_OUTPUT_FILE             ${CMAKE_CURRENT_BINARY_DIR}/dfu.bin)
set(APP_VALIDATED_IMAGE_SCRIPT  $ENV{ZEPHYR_PROJECT}/scripts/gen/gen_validated_app_image.sh)
set(CRC32_SCRIPT                $ENV{ZEPHYR_PROJECT}/scripts/gen/gen_crc32.sh)

add_custom_command(
    OUTPUT
    ${APP_VALIDATED_BIN_FILE}
    ${APP_VALIDATED_HEX_FILE}
    ${APP_VALIDATED_INFO_FILE}

    COMMAND
    bash
    ${APP_VALIDATED_IMAGE_SCRIPT}
    --input=${ZEPHYR_BIN_FILE}
    --output=${APP_VALIDATED_BIN_FILE}
    --dts=${ZEPHYR_DTS_FILE}
    --hex-output=${APP_VALIDATED_HEX_FILE}
    --info-output=${APP_VALIDATED_INFO_FILE}
    --objcopy=${CMAKE_OBJCOPY}

    DEPENDS
    app
    ${ZEPHYR_BIN_FILE}
    ${ZEPHYR_DTS_FILE}
    ${APP_VALIDATED_IMAGE_SCRIPT}
    ${CRC32_SCRIPT}

    COMMENT "Generating validated application binary"
    VERBATIM
)

add_custom_target(firmware_dfu ALL
    COMMAND
    ${CMAKE_COMMAND}
    -DINPUT_FILE=${APP_VALIDATED_BIN_FILE}
    -DOUTPUT_FILE=${DFU_OUTPUT_FILE}
    -DTARGET_UID=${OPENDECK_TARGET_UID}
    -DFORMAT_VERSION=${OPENDECK_DFU_FORMAT_VERSION}
    -DBEGIN_MAGIC=${OPENDECK_DFU_BEGIN_MAGIC}
    -DEND_MAGIC=${OPENDECK_DFU_END_MAGIC}
    -P $ENV{ZEPHYR_PROJECT}/cmake/dfu_bin.cmake
    DEPENDS ${APP_VALIDATED_BIN_FILE}
    COMMENT "Generating wrapped DFU binary"
    VERBATIM
)
