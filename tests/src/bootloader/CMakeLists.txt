cmake_minimum_required(VERSION 3.20)
set(KCONFIG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Kconfig)
list(APPEND CONF_FILE "${CMAKE_CURRENT_LIST_DIR}/prj.conf")
include($ENV{ZEPHYR_WS}/zenv/cmake/test.cmake)

project(test)

add_compile_definitions(
    OPENDECK_TEST
    BOOTLOADER_TEST_FLASH_PAGE_SIZE=256
)

add_subdirectory($ENV{ZEPHYR_PROJECT}/app/common/metadata ${CMAKE_CURRENT_BINARY_DIR}/metadata)

set(BOOTLOADER_TEST_PAYLOAD_FILE ${CMAKE_CURRENT_BINARY_DIR}/bootloader_test_payload.bin)
set(BOOTLOADER_TEST_DFU_FILE ${CMAKE_CURRENT_BINARY_DIR}/bootloader_test_dfu.bin)

file(GENERATE
    OUTPUT ${BOOTLOADER_TEST_PAYLOAD_FILE}
    CONTENT "OpenDeck bootloader test fixture payload 1234567890 ABCDEFGHIJKLMNOPQRSTUVWXYZ"
)

add_custom_command(
    OUTPUT ${BOOTLOADER_TEST_DFU_FILE}
    COMMAND
    ${CMAKE_COMMAND}
    -DINPUT_FILE=${BOOTLOADER_TEST_PAYLOAD_FILE}
    -DOUTPUT_FILE=${BOOTLOADER_TEST_DFU_FILE}
    -DTARGET_UID=${OPENDECK_TARGET_UID}
    -DFORMAT_VERSION=${OPENDECK_DFU_FORMAT_VERSION}
    -DBEGIN_MAGIC=${OPENDECK_DFU_BEGIN_MAGIC}
    -DEND_MAGIC=${OPENDECK_DFU_END_MAGIC}
    -P $ENV{ZEPHYR_PROJECT}/cmake/dfu_bin.cmake
    DEPENDS ${BOOTLOADER_TEST_PAYLOAD_FILE}
    COMMENT "Generating bootloader test DFU binary"
    VERBATIM
)

add_custom_target(bootloader_test_dfu
    DEPENDS ${BOOTLOADER_TEST_DFU_FILE}
)

target_sources(app
    PRIVATE
    bootloader_test.cpp
    $ENV{ZEPHYR_PROJECT}/app/bootloader/src/updater/updater.cpp
)

target_include_directories(app
    PRIVATE
    $ENV{ZEPHYR_PROJECT}/app/firmware/src
    $ENV{ZEPHYR_PROJECT}/app/bootloader/src
)

add_dependencies(app
    metadata
    bootloader_test_dfu
)

target_link_libraries(app
    PUBLIC
    metadata
)

target_compile_definitions(app
    PRIVATE
    OPENDECK_TEST_PAYLOAD_FILE="${BOOTLOADER_TEST_PAYLOAD_FILE}"
    OPENDECK_TEST_DFU_FILE="${BOOTLOADER_TEST_DFU_FILE}"
)
