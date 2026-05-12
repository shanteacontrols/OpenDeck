cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(APPLY_TARGET)

opendeck_test_add_modules(
    metadata     "app/common/metadata"
    database     "app/firmware/src/database"
    configurable "app/firmware/src/util/configurable"
    switches      "app/firmware/src/io/digital/switches"
    encoders     "app/firmware/src/io/digital/encoders"
)

target_sources(app
    PRIVATE
    digital_switches_test.cpp
    digital_encoders_test.cpp
)
