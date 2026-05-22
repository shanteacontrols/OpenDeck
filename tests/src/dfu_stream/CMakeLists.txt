cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(
    IMPORT_TARGET_CONFIG
)

opendeck_test_add_modules(
    metadata   "app/common/src/metadata"
    dfu_stream "app/common/src/dfu_stream"
)

target_sources(app
    PRIVATE
    dfu_stream_test.cpp
)
