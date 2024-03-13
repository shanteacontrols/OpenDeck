if(NOT "PROJECT_TARGET_USB_OVER_SERIAL_HOST" IN_LIST PROJECT_TARGET_DEFINES)
    add_executable(system)

    target_sources(system
        PRIVATE
        test.cpp
        ${PROJECT_ROOT}/src/firmware/application/database/Database.cpp
        ${PROJECT_ROOT}/src/firmware/application/database/CustomInit.cpp
        ${PROJECT_ROOT}/src/firmware/application/system/System.cpp
        ${PROJECT_ROOT}/src/firmware/application/util/cinfo/CInfo.cpp
        ${PROJECT_ROOT}/src/firmware/application/protocol/midi/MIDI.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/buttons/Buttons.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/encoders/Encoders.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/leds/LEDs.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/analog/Analog.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/i2c/I2C.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/i2c/peripherals/Builder.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/i2c/peripherals/display/Display.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/touchscreen/Touchscreen.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/touchscreen/model/Builder.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/touchscreen/model/nextion/Nextion.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/touchscreen/model/viewtech/Viewtech.cpp
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