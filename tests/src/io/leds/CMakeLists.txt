cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(APPLY_TARGET)

opendeck_test_add_modules(
    metadata     "app/common/metadata"
    database     "app/firmware/src/database"
    configurable "app/firmware/src/util/configurable"
    leds         "app/firmware/src/io/leds"
)

target_sources(app
    PRIVATE
    leds_test.cpp
)
