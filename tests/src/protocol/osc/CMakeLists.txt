cmake_minimum_required(VERSION 3.20)
include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(
    IMPORT_TARGET_CONFIG
    COMPILE_DEFINITIONS
    CONFIG_OPENDECK_LOG_LEVEL=3
)

opendeck_test_add_modules(
    opendeck_common_metadata     "app/common/src/metadata"
    opendeck_firmware_database     "app/firmware/src/database"
    opendeck_firmware_util_configurable "app/firmware/src/util/configurable"
    opendeck_firmware_protocol_osc "app/firmware/src/protocol/osc"
)

target_sources(app
    PRIVATE
    osc_packet_test.cpp
    osc_protocol_test.cpp
)
