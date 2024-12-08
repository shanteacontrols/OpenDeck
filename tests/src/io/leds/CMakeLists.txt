if(NOT "PROJECT_TARGET_USB_OVER_SERIAL_HOST" IN_LIST PROJECT_TARGET_DEFINES)
    add_executable(leds)

    target_sources(leds
        PRIVATE
        test.cpp
        ${PROJECT_ROOT}/src/firmware/application/database/database.cpp
        ${PROJECT_ROOT}/src/firmware/application/database/custom_init.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/leds/leds.cpp
    )

    target_link_libraries(leds
        PUBLIC
        common
    )

    add_test(
        NAME leds
        COMMAND $<TARGET_FILE:leds>
    )
endif()