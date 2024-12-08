if(NOT "PROJECT_TARGET_USB_OVER_SERIAL_HOST" IN_LIST PROJECT_TARGET_DEFINES)
    add_executable(encoders)

    target_sources(encoders
        PRIVATE
        test.cpp
        ${PROJECT_ROOT}/src/firmware/application/database/database.cpp
        ${PROJECT_ROOT}/src/firmware/application/database/custom_init.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/leds/leds.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/encoders/encoders.cpp
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