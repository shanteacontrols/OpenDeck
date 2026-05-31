cmake_minimum_required(VERSION 3.20)
include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap()

opendeck_test_add_modules(
    opendeck_common_mcu                     "app/common/src/mcu"
    opendeck_common_metadata                "app/common/src/metadata"
    opendeck_firmware_protocol_osc_packet   "app/firmware/src/protocol/osc/packet"
    opendeck_firmware_protocol_websockets   "app/firmware/src/protocol/websockets"
)

target_sources(app
    PRIVATE
    websockets_protocol_test.cpp
    websockets_test.cpp
)
