cmake_minimum_required(VERSION 3.20)
include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(
    IMPORT_TARGET_CONFIG
    COMPILE_DEFINITIONS
    CONFIG_OPENDECK_LOG_LEVEL=3
)

opendeck_test_add_modules(
    opendeck_common_mcu           "app/common/src/mcu"
    opendeck_firmware_database      "app/firmware/src/database"
    opendeck_firmware_util_configurable  "app/firmware/src/util/configurable"
    opendeck_common_metadata      "app/common/src/metadata"
    opendeck_common_protocols_mdns "app/common/src/protocols/mdns"
    opendeck_firmware_protocol_mdns "app/firmware/src/protocol/mdns"
)

target_sources(app
    PRIVATE
    mdns_test.cpp
)
