cmake_minimum_required(VERSION 3.20)
include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(IMPORT_TARGET_CONFIG)

opendeck_test_add_modules(
    opendeck_common_metadata             "app/common/src/metadata"
    opendeck_common_dfu_flash_stream_writer  "app/common/src/dfu/flash_stream_writer"
    opendeck_common_protocols_mdns        "app/common/src/protocols/mdns"
    opendeck_common_mcu                  "app/common/src/mcu"
    opendeck_firmware_database             "app/firmware/src/database"
    opendeck_firmware_util_configurable         "app/firmware/src/util/configurable"
    opendeck_firmware_util_cinfo                "app/firmware/src/util/cinfo"
    opendeck_firmware_protocol_midi        "app/firmware/src/protocol/midi"
    opendeck_firmware_protocol_mdns        "app/firmware/src/protocol/mdns"
    opendeck_firmware_dfu_staged_update_writer "app/firmware/src/dfu/staged_update_writer"
    opendeck_firmware_io_digital              "app/firmware/src/io/digital"
    opendeck_firmware_io_digital_switches             "app/firmware/src/io/digital/switches"
    opendeck_firmware_io_digital_encoders             "app/firmware/src/io/digital/encoders"
    opendeck_firmware_io_analog               "app/firmware/src/io/analog"
    opendeck_firmware_io_outputs              "app/firmware/src/io/outputs"
    opendeck_firmware_io_touchscreen          "app/firmware/src/io/touchscreen"
    opendeck_firmware_io_i2c                  "app/firmware/src/io/i2c"
    opendeck_firmware_io_indicators           "app/firmware/src/io/indicators"
    opendeck_firmware_system               "app/firmware/src/system"
)

target_sources(app
    PRIVATE
    system_test.cpp
)

target_link_libraries(app
    PUBLIC
    kernel
)
