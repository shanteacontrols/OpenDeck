if(NOT "PROJECT_TARGET_USB_OVER_SERIAL_HOST" IN_LIST PROJECT_TARGET_DEFINES)
    add_executable(encoders)

    target_sources(encoders
        PRIVATE
        test.cpp
        ${PROJECT_ROOT}/src/firmware/application/database/Database.cpp
        ${PROJECT_ROOT}/src/firmware/application/database/CustomInit.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/leds/LEDs.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/encoders/Encoders.cpp
    )

    target_link_libraries(encoders
        PUBLIC
        common
    )

    add_test(
        NAME encoders
        COMMAND $<TARGET_FILE:encoders>
    )
endif()