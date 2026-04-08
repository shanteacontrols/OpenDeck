add_executable(leds)

target_sources(leds
    PRIVATE
    test.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/database/database.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/database/custom_init.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/io/leds/leds.cpp
)

target_link_libraries(leds
    PUBLIC
    common
)

add_test(
    NAME leds
    COMMAND $<TARGET_FILE:leds>
)