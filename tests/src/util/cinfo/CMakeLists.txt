cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(
    IMPORT_TARGET_CONFIG
    COMPILE_DEFINITIONS
    CONFIG_OPENDECK_LOG_LEVEL=3
)

opendeck_test_add_modules(
    metadata     "app/common/src/metadata"
    configurable "app/firmware/src/util/configurable"
    database     "app/firmware/src/database"
    cinfo        "app/firmware/src/util/cinfo"
)

target_sources(app
    PRIVATE
    cinfo_test.cpp
)
