cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(
    COMPILE_DEFINITIONS
    BOOTLOADER_TEST_FLASH_PAGE_SIZE=256
)

add_subdirectory($ENV{ZEPHYR_PROJECT}/app/common/src/metadata ${CMAKE_CURRENT_BINARY_DIR}/metadata)

if(NOT DEFINED ENV{TARGET} OR "$ENV{TARGET}" STREQUAL "")
    message(FATAL_ERROR "Bootloader tests require TARGET to be exported")
endif()

set(BOOTLOADER_TEST_APP_BUILD_DIR $ENV{ZEPHYR_PROJECT}/build/app/default/release/$ENV{TARGET}/app)
set(BOOTLOADER_TEST_VALIDATED_PAYLOAD_FILE ${BOOTLOADER_TEST_APP_BUILD_DIR}/app_validated.bin)
set(BOOTLOADER_TEST_DFU_FILE ${BOOTLOADER_TEST_APP_BUILD_DIR}/dfu.bin)
set(BOOTLOADER_TEST_APP_INFO_FILE ${BOOTLOADER_TEST_APP_BUILD_DIR}/app_validated_info.h)

if(NOT EXISTS ${BOOTLOADER_TEST_VALIDATED_PAYLOAD_FILE})
    message(FATAL_ERROR "Missing target app payload: ${BOOTLOADER_TEST_VALIDATED_PAYLOAD_FILE}. Build the app for TARGET=$ENV{TARGET} first.")
endif()

if(NOT EXISTS ${BOOTLOADER_TEST_DFU_FILE})
    message(FATAL_ERROR "Missing target DFU payload: ${BOOTLOADER_TEST_DFU_FILE}. Build the app for TARGET=$ENV{TARGET} first.")
endif()

if(NOT EXISTS ${BOOTLOADER_TEST_APP_INFO_FILE})
    message(FATAL_ERROR "Missing target app info: ${BOOTLOADER_TEST_APP_INFO_FILE}. Build the app for TARGET=$ENV{TARGET} first.")
endif()

target_sources(app
    PRIVATE
    bootloader_test.cpp
    $ENV{ZEPHYR_PROJECT}/app/bootloader/src/fw_selector/instance/impl/fw_selector.cpp
    $ENV{ZEPHYR_PROJECT}/app/bootloader/src/updater/instance/impl/updater.cpp
)

target_include_directories(app
    PRIVATE
    ${BOOTLOADER_TEST_APP_BUILD_DIR}
)

add_dependencies(app
    metadata
)

target_link_libraries(app
    PUBLIC
    metadata
)

target_compile_definitions(app
    PRIVATE
    OPENDECK_TEST_VALIDATED_PAYLOAD_FILE="${BOOTLOADER_TEST_VALIDATED_PAYLOAD_FILE}"
    OPENDECK_TEST_DFU_FILE="${BOOTLOADER_TEST_DFU_FILE}"
)
