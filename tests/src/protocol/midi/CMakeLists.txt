cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(APPLY_TARGET)

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
