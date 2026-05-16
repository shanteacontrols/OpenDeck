cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(APPLY_TARGET)

opendeck_test_add_modules(
    metadata      "app/common/src/metadata"
    mcu           "app/firmware/src/mcu"
    database      "app/firmware/src/database"
    configurable  "app/firmware/src/util/configurable"
    cinfo         "app/firmware/src/util/cinfo"
    protocol_midi "app/firmware/src/protocol/midi"
    protocol_mdns "app/firmware/src/protocol/mdns"
    digital       "app/firmware/src/io/digital"
    switches      "app/firmware/src/io/digital/switches"
    encoders      "app/firmware/src/io/digital/encoders"
    analog        "app/firmware/src/io/analog"
    outputs       "app/firmware/src/io/outputs"
    touchscreen   "app/firmware/src/io/touchscreen"
    i2c           "app/firmware/src/io/i2c"
    indicators    "app/firmware/src/io/indicators"
    system        "app/firmware/src/system"
)

target_sources(app
    PRIVATE
    system_test.cpp
)

target_link_libraries(app
    PUBLIC
    kernel
)
