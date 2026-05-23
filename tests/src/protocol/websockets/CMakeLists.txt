cmake_minimum_required(VERSION 3.20)
include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap()

opendeck_test_add_modules(
    opendeck_common_mcu                                 "app/common/src/mcu"
    opendeck_common_metadata                            "app/common/src/metadata"
    opendeck_common_dfu_flash_stream_writer                 "app/common/src/dfu/flash_stream_writer"
    opendeck_common_dfu_dfu_stream                          "app/common/src/dfu/dfu_stream"
    opendeck_common_protocols_websockets_firmware_upload "app/common/src/protocols/websockets/firmware_upload"
    opendeck_firmware_protocol_osc_packet                 "app/firmware/src/protocol/osc/packet"
    opendeck_firmware_dfu_staged_update_writer                "app/firmware/src/dfu/staged_update_writer"
    opendeck_firmware_protocol_websockets                 "app/firmware/src/protocol/websockets"
)

target_sources(app
    PRIVATE
    websockets_protocol_test.cpp
    websockets_test.cpp
)
