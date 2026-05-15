cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(APPLY_TARGET)

opendeck_test_add_modules(
    metadata     "app/common/src/metadata"
    database     "app/firmware/src/database"
    configurable "app/firmware/src/util/configurable"
    outputs      "app/firmware/src/io/outputs"
)

target_sources(app
    PRIVATE
    outputs_test.cpp
)
