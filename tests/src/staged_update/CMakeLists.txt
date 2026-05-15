cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(
    APPLY_TARGET
    COMPILE_DEFINITIONS
    CONFIG_OPENDECK_LOG_LEVEL=3
)

opendeck_test_add_modules(
    metadata      "app/common/src/metadata"
    staged_update "app/firmware/src/staged_update"
)

target_sources(app
    PRIVATE
    staged_update_test.cpp
)
