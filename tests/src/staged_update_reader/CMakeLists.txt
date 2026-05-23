cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(
    IMPORT_TARGET_CONFIG
)

opendeck_test_add_modules(
    opendeck_common_metadata                                 "app/common/src/metadata"
    opendeck_common_dfu_dfu_stream                               "app/common/src/dfu/dfu_stream"
    opendeck_bootloader_dfu_staged_update_reader "app/bootloader/src/dfu/staged_update_reader"
)

target_sources(app
    PRIVATE
    staged_update_reader_test.cpp
)
