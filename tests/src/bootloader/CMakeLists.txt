cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(
    COMPILE_DEFINITIONS
    BOOTLOADER_TEST_FLASH_SECTOR_SIZE=256
)

add_subdirectory($ENV{ZEPHYR_PROJECT}/app/common/src/metadata ${CMAKE_CURRENT_BINARY_DIR}/metadata)
add_subdirectory($ENV{ZEPHYR_PROJECT}/app/common/src/dfu/flash_stream_writer ${CMAKE_CURRENT_BINARY_DIR}/flash_stream_writer)
add_subdirectory($ENV{ZEPHYR_PROJECT}/app/common/src/dfu/dfu_stream ${CMAKE_CURRENT_BINARY_DIR}/dfu_stream)

if(NOT DEFINED ENV{TARGET} OR "$ENV{TARGET}" STREQUAL "")
    message(FATAL_ERROR "Bootloader tests require TARGET to be exported")
endif()

set(BOOTLOADER_TEST_APP_BUILD_DIR $ENV{ZEPHYR_PROJECT}/build/app/default/release/$ENV{TARGET}/app)
set(BOOTLOADER_TEST_APP_PAYLOAD_FILE ${BOOTLOADER_TEST_APP_BUILD_DIR}/zephyr/zephyr.signed.bin)
set(BOOTLOADER_TEST_DFU_FILE ${BOOTLOADER_TEST_APP_BUILD_DIR}/dfu.bin)

if(NOT EXISTS ${BOOTLOADER_TEST_APP_PAYLOAD_FILE})
    message(FATAL_ERROR "Missing target app payload: ${BOOTLOADER_TEST_APP_PAYLOAD_FILE}. Build the app for TARGET=$ENV{TARGET} first.")
endif()

if(NOT EXISTS ${BOOTLOADER_TEST_DFU_FILE})
    message(FATAL_ERROR "Missing target DFU payload: ${BOOTLOADER_TEST_DFU_FILE}. Build the app for TARGET=$ENV{TARGET} first.")
endif()

target_sources(app
    PRIVATE
    bootloader_test.cpp
    $ENV{ZEPHYR_PROJECT}/app/bootloader/src/io/indicators/instance/impl/indicators.cpp
    $ENV{ZEPHYR_PROJECT}/app/bootloader/src/dfu/direct_update_writer/instance/impl/direct_update_writer.cpp
    $ENV{ZEPHYR_PROJECT}/app/bootloader/src/dfu/staged_update_reader/instance/impl/staged_update_reader.cpp
)

add_dependencies(app
    opendeck_common_metadata
    opendeck_common_dfu_flash_stream_writer
    opendeck_common_dfu_dfu_stream
)

target_link_libraries(app
    PUBLIC
    opendeck_common_metadata
    opendeck_common_dfu_flash_stream_writer
    opendeck_common_dfu_dfu_stream
)

target_compile_definitions(app
    PRIVATE
    OPENDECK_TEST_APP_PAYLOAD_FILE="${BOOTLOADER_TEST_APP_PAYLOAD_FILE}"
    OPENDECK_TEST_DFU_FILE="${BOOTLOADER_TEST_DFU_FILE}"
)
