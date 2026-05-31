cmake_minimum_required(VERSION 3.20)
include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(
    COMPILE_DEFINITIONS
    BOOTLOADER_TEST_FLASH_SECTOR_SIZE=256
)

opendeck_test_add_modules(
    opendeck_common_metadata                                 "app/common/src/metadata"
    opendeck_common_dfu_flash_stream_writer                  "app/common/src/dfu/flash_stream_writer"
    opendeck_common_dfu_dfu_stream_parser                    "app/common/src/dfu/dfu_stream_parser"
    opendeck_common_protocols_websockets_firmware_upload     "app/common/src/protocols/websockets/firmware_upload"
    opendeck_bootloader_dfu_direct_update_writer             "app/bootloader/src/dfu/direct_update_writer"
    opendeck_bootloader_protocols_websockets_command_handler "app/bootloader/src/protocols/websockets/command_handler"
)

target_sources(app
    PRIVATE
    bootloader_websockets_test.cpp
    $ENV{ZENV_PROJECT_ROOT}/app/bootloader/src/protocols/websockets/instance/impl/websockets.cpp
)

target_link_libraries(app
    PUBLIC
    zlibs_utils_misc
    zlibs_utils_threads
)
