cmake_minimum_required(VERSION 3.20)
include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap()

opendeck_test_add_modules(
    opendeck_common_metadata            "app/common/src/metadata"
    opendeck_firmware_database          "app/firmware/src/database"
    opendeck_firmware_util_configurable "app/firmware/src/util/configurable"
    opendeck_firmware_protocol_midi     "app/firmware/src/protocol/midi"
)

target_sources(app
    PRIVATE
    midi_test.cpp
)

target_link_libraries(app
    PUBLIC
    kernel
)
