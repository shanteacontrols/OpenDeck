cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(APPLY_TARGET)

if(NOT TARGET zlibs-drivers-usb_hw)
    add_library(zlibs-drivers-usb_hw INTERFACE)
endif()

if(NOT TARGET zlibs-drivers-uart_hw)
    add_library(zlibs-drivers-uart_hw INTERFACE)
endif()

opendeck_test_add_modules(
    metadata        "app/common/src/metadata"
    database        "app/firmware/src/database"
    configurable    "app/firmware/src/util/configurable"
    protocol_midi   "app/firmware/src/protocol/midi"
)

target_sources(app
    PRIVATE
    midi_test.cpp
)

target_link_libraries(app
    PUBLIC
    kernel
)
