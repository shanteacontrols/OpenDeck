cmake_minimum_required(VERSION 3.20)
include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(IMPORT_TARGET_CONFIG)

opendeck_test_add_modules(
    opendeck_common_metadata                             "app/common/src/metadata"
    opendeck_common_mcu                                  "app/common/src/mcu"
    opendeck_firmware_database                           "app/firmware/src/database"
    opendeck_firmware_util_configurable                  "app/firmware/src/util/configurable"
    opendeck_firmware_util_cinfo                         "app/firmware/src/util/cinfo"
    opendeck_firmware_system                             "app/firmware/src/system"
)

if(CONFIG_PROJECT_TARGET_SUPPORT_MIDI)
    opendeck_test_add_modules(
        opendeck_firmware_protocol_midi "app/firmware/src/protocol/midi"
    )
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_DIGITAL_INPUTS)
    opendeck_test_add_modules(
        opendeck_firmware_io_digital "app/firmware/src/io/digital"
    )
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_SWITCHES)
    opendeck_test_add_modules(
        opendeck_firmware_io_digital_switches "app/firmware/src/io/digital/switches"
    )
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_ENCODERS)
    opendeck_test_add_modules(
        opendeck_firmware_io_digital_encoders "app/firmware/src/io/digital/encoders"
    )
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_ADC)
    opendeck_test_add_modules(
        opendeck_firmware_io_analog "app/firmware/src/io/analog"
    )
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_OUTPUTS)
    opendeck_test_add_modules(
        opendeck_firmware_io_outputs "app/firmware/src/io/outputs"
    )
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_TOUCHSCREEN)
    opendeck_test_add_modules(
        opendeck_firmware_io_touchscreen "app/firmware/src/io/touchscreen"
    )
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_I2C)
    opendeck_test_add_modules(
        opendeck_firmware_io_i2c "app/firmware/src/io/i2c"
    )
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_TRAFFIC_INDICATORS)
    opendeck_test_add_modules(
        opendeck_firmware_io_indicators "app/firmware/src/io/indicators"
    )
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_OSC)
    opendeck_test_add_modules(
        opendeck_firmware_protocol_osc "app/firmware/src/protocol/osc"
    )
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_WEBSOCKETS)
    opendeck_test_add_modules(
        opendeck_firmware_protocol_websockets "app/firmware/src/protocol/websockets"
    )
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_MDNS)
    opendeck_test_add_modules(
        opendeck_firmware_protocol_mdns "app/firmware/src/protocol/mdns"
    )
endif()

target_sources(app
    PRIVATE
    firmware_system_test.cpp
)

target_link_libraries(app
    PUBLIC
    kernel
)
