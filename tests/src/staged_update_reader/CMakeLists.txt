cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(
    IMPORT_TARGET_CONFIG
)

opendeck_test_add_modules(
    metadata                                 "app/common/src/metadata"
    dfu_stream                               "app/common/src/dfu_stream"
    opendeck_bootloader_staged_update_reader "app/bootloader/src/staged_update_reader"
)

target_sources(app
    PRIVATE
    staged_update_reader_test.cpp
)
