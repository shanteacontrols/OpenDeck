cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(
    COMPILE_DEFINITIONS
    CONFIG_OPENDECK_LOG_LEVEL=3
)

opendeck_test_add_modules(
    mcu                                 "app/common/src/mcu"
    metadata                            "app/common/src/metadata"
    flash_stream_writer                 "app/common/src/dfu/flash_stream_writer"
    dfu_stream                          "app/common/src/dfu/dfu_stream"
    opendeck_websockets_firmware_upload "app/common/src/protocols/websockets/firmware_upload"
    protocol_osc_packet                 "app/firmware/src/protocol/osc/packet"
    staged_update_writer                "app/firmware/src/dfu/staged_update_writer"
    protocol_websockets                 "app/firmware/src/protocol/websockets"
)

target_sources(app
    PRIVATE
    websockets_protocol_test.cpp
    websockets_test.cpp
)
