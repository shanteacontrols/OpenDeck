cmake_minimum_required(VERSION 3.20)
include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap()

opendeck_test_add_modules(
    opendeck_common_metadata                   "app/common/src/metadata"
    opendeck_firmware_dfu_staged_update_writer "app/firmware/src/dfu/staged_update_writer"
)

target_sources(app
    PRIVATE
    firmware_dfu_staged_update_writer_test.cpp
)
