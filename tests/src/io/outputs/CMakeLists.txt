cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(IMPORT_TARGET_CONFIG)

opendeck_test_add_modules(
    opendeck_common_metadata     "app/common/src/metadata"
    opendeck_firmware_database     "app/firmware/src/database"
    opendeck_firmware_util_configurable "app/firmware/src/util/configurable"
    opendeck_firmware_io_outputs      "app/firmware/src/io/outputs"
)

target_sources(app
    PRIVATE
    outputs_test.cpp
)
