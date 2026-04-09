include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(system)

zephyr_library_sources(
    system.cpp
)

target_link_libraries(system
    PUBLIC
    metadata
    database
    configurable
    cinfo
    protocol_midi
    digital
    buttons
    analog
    leds
    encoders
    touchscreen
    i2c
)
