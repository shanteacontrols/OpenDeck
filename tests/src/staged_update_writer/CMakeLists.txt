cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(
    IMPORT_TARGET_CONFIG
    COMPILE_DEFINITIONS
    CONFIG_OPENDECK_LOG_LEVEL=3
)

opendeck_test_add_modules(
    metadata             "app/common/src/metadata"
    flash_stream_writer  "app/common/src/flash_stream_writer"
    staged_update_writer "app/firmware/src/staged_update_writer"
)

target_sources(app
    PRIVATE
    staged_update_writer_test.cpp
)
