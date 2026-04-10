zephyr_app_library_named(system)

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
