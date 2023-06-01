if(NOT "PROJECT_TARGET_USB_OVER_SERIAL_HOST" IN_LIST PROJECT_TARGET_DEFINES)
    add_executable(buttons)

    target_sources(buttons
        PRIVATE
        test.cpp
        ${PROJECT_ROOT}/src/firmware/application/database/Database.cpp
        ${PROJECT_ROOT}/src/firmware/application/database/CustomInit.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/leds/LEDs.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/buttons/Buttons.cpp
    )

    target_link_libraries(buttons
        PUBLIC
        common
    )

    add_test(
        NAME buttons
        COMMAND $<TARGET_FILE:buttons>
    )
endif()