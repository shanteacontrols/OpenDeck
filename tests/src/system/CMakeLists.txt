if(NOT "PROJECT_TARGET_USB_OVER_SERIAL_HOST" IN_LIST PROJECT_TARGET_DEFINES)
    add_executable(system)

    target_sources(system
        PRIVATE
        test.cpp
        ${PROJECT_ROOT}/src/firmware/application/database/database.cpp
        ${PROJECT_ROOT}/src/firmware/application/database/custom_init.cpp
        ${PROJECT_ROOT}/src/firmware/application/system/system.cpp
        ${PROJECT_ROOT}/src/firmware/application/util/cinfo/cinfo.cpp
        ${PROJECT_ROOT}/src/firmware/application/protocol/midi/midi.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/buttons/buttons.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/encoders/encoders.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/leds/leds.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/analog/analog.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/i2c/i2c.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/i2c/peripherals/display/display.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/i2c/peripherals/display/elements.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/touchscreen/touchscreen.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/touchscreen/models/nextion/nextion.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/touchscreen/models/viewtech/viewtech.cpp
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
endif()