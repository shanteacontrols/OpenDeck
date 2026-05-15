cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(
    APPLY_TARGET
    COMPILE_DEFINITIONS
    CONFIG_OPENDECK_LOG_LEVEL=3
)

opendeck_test_add_modules(
    mcu           "app/firmware/src/mcu"
    database      "app/firmware/src/database"
    configurable  "app/firmware/src/util/configurable"
    metadata      "app/common/src/metadata"
    protocol_mdns "app/firmware/src/protocol/mdns"
)

target_sources(app
    PRIVATE
    mdns_test.cpp
)
