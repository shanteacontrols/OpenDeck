cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(
    COMPILE_DEFINITIONS
    CONFIG_OPENDECK_LOG_LEVEL=3
    BOOTLOADER_TEST_FLASH_SECTOR_SIZE=256
)

opendeck_test_add_modules(
    metadata                            "app/common/src/metadata"
    flash_stream_writer                 "app/common/src/dfu/flash_stream_writer"
    dfu_stream                          "app/common/src/dfu/dfu_stream"
    opendeck_websockets_firmware_upload "app/common/src/protocols/websockets/firmware_upload"
)

target_sources(app
    PRIVATE
    bootloader_websockets_test.cpp
    $ENV{ZEPHYR_PROJECT}/app/bootloader/src/dfu/direct_update_writer/instance/impl/direct_update_writer.cpp
    $ENV{ZEPHYR_PROJECT}/app/bootloader/src/protocols/websockets/instance/impl/websockets.cpp
)

target_link_libraries(app
    PUBLIC
    zlibs-utils-misc
    zlibs-utils-threads
)
