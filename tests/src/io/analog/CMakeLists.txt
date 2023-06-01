if(NOT "PROJECT_TARGET_USB_OVER_SERIAL_HOST" IN_LIST PROJECT_TARGET_DEFINES)
    add_executable(analog)

    target_sources(analog
        PRIVATE
        test.cpp
        ${PROJECT_ROOT}/src/firmware/application/database/Database.cpp
        ${PROJECT_ROOT}/src/firmware/application/database/CustomInit.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/buttons/Buttons.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/leds/LEDs.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/analog/Analog.cpp
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