cmake_minimum_required(VERSION 3.20)
include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap()

opendeck_test_add_modules(
    opendeck_common_metadata                     "app/common/src/metadata"
    opendeck_common_dfu_writer                   "app/common/src/dfu/writer"
    opendeck_common_dfu_dfu_stream_parser        "app/common/src/dfu/dfu_stream_parser"
    opendeck_bootloader_dfu_staged_update_reader "app/bootloader/src/dfu/staged_update_reader"
)

target_sources(app
    PRIVATE
    bootloader_dfu_staged_update_reader_test.cpp
)
