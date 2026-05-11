cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(
    APPLY_TARGET
    COMPILE_DEFINITIONS
    CONFIG_OPENDECK_LOG_LEVEL=3
)

opendeck_test_add_modules(
    metadata     "app/common/metadata"
    database     "app/firmware/src/database"
    configurable "app/firmware/src/util/configurable"
    protocol_osc "app/firmware/src/protocol/osc"
)

target_sources(app
    PRIVATE
    osc_packet_test.cpp
    osc_protocol_test.cpp
)
