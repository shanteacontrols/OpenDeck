add_executable(encoders)

target_sources(encoders
    PRIVATE
    test.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/database/database.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/database/custom_init.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/io/leds/leds.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/io/encoders/encoders.cpp
)

target_link_libraries(encoders
    PUBLIC
    common
)

add_test(
    NAME encoders
    COMMAND $<TARGET_FILE:encoders>
)