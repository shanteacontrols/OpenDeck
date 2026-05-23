cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(IMPORT_TARGET_CONFIG)

opendeck_test_add_modules(
    opendeck_common_metadata     "app/common/src/metadata"
    opendeck_firmware_database     "app/firmware/src/database"
    opendeck_firmware_util_configurable "app/firmware/src/util/configurable"
    opendeck_firmware_io_digital_switches     "app/firmware/src/io/digital/switches"
    opendeck_firmware_io_digital_encoders     "app/firmware/src/io/digital/encoders"
)

target_sources(app
    PRIVATE
    digital_switches_test.cpp
    digital_encoders_test.cpp
)
