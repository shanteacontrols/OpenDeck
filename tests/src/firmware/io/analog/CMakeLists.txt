cmake_minimum_required(VERSION 3.20)
include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(IMPORT_TARGET_CONFIG)

if(CONFIG_PROJECT_TARGET_SUPPORT_ADC)
    opendeck_test_add_modules(
        opendeck_common_metadata                     "app/common/src/metadata"
        opendeck_firmware_database                   "app/firmware/src/database"
        opendeck_firmware_util_configurable          "app/firmware/src/util/configurable"
        opendeck_firmware_io_analog                  "app/firmware/src/io/analog"
        opendeck_firmware_io_digital_switches        "app/firmware/src/io/digital/switches"
    )
endif()

target_sources(app
    PRIVATE
    firmware_io_analog_test.cpp
)
