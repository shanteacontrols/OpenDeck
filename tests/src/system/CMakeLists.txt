add_executable(system)

target_sources(system
    PRIVATE
    test.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/database/database.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/database/custom_init.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/system/system.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/util/cinfo/cinfo.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/protocol/midi/midi.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/io/buttons/buttons.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/io/encoders/encoders.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/io/leds/leds.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/io/analog/analog.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/io/i2c/i2c.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/io/i2c/peripherals/display/display.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/io/i2c/peripherals/display/elements.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/io/touchscreen/touchscreen.cpp
)

target_compile_definitions(system
    PUBLIC
    SW_VERSION_MAJOR=0
    SW_VERSION_MINOR=0
    SW_VERSION_REVISION=0
)

target_link_libraries(system
    PUBLIC
    common
)

add_test(
    NAME system
    COMMAND $<TARGET_FILE:system>
)