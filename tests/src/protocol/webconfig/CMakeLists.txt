cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(
    COMPILE_DEFINITIONS
    CONFIG_OPENDECK_LOG_LEVEL=3
)

opendeck_test_add_modules(
    mcu                "app/firmware/src/mcu"
    metadata           "app/common/metadata"
    staged_update      "app/firmware/src/staged_update"
    protocol_webconfig "app/firmware/src/protocol/webconfig"
)

target_sources(app
    PRIVATE
    webconfig_protocol_test.cpp
    webconfig_test.cpp
)
