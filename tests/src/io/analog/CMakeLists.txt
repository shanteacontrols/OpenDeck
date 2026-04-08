if(NOT "PROJECT_TARGET_USB_OVER_SERIAL_HOST" IN_LIST PROJECT_TARGET_DEFINES)
    add_executable(analog)

    target_sources(analog
        PRIVATE
        test.cpp
        $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/database/database.cpp
        $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/database/custom_init.cpp
        $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/io/buttons/buttons.cpp
        $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/io/leds/leds.cpp
        $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/io/analog/analog.cpp
    )

    target_link_libraries(analog
        PUBLIC
        common
    )

    add_test(
        NAME analog
        COMMAND $<TARGET_FILE:analog>
    )
endif()