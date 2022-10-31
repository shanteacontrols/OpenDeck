vpath application/%.cpp ../src
vpath common/%.cpp ../src
vpath modules/%.cpp ../

ifeq (,$(findstring HW_USB_OVER_SERIAL_HOST,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) := \
    application/database/Database.cpp \
    application/database/CustomInit.cpp \
    application/system/System.cpp \
    application/util/cinfo/CInfo.cpp \
    application/protocol/midi/MIDI.cpp \
    modules/sysex/src/SysExConf.cpp \
    application/io/buttons/Buttons.cpp \
    application/io/encoders/Encoders.cpp \
    application/io/leds/LEDs.cpp \
    application/io/analog/Analog.cpp \

    ifneq (,$(findstring HW_SUPPORT_I2C,$(DEFINES)))
        SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
        application/io/i2c/I2C.cpp \
        application/io/i2c/peripherals/Builder.cpp \
        application/io/i2c/peripherals/display/Display.cpp \
        application/io/i2c/peripherals/display/strings/Strings.cpp
    endif

    ifneq (,$(findstring HW_SUPPORT_TOUCHSCREEN,$(DEFINES)))
        SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
        application/io/touchscreen/Touchscreen.cpp \
        application/io/touchscreen/model/Builder.cpp \
        application/io/touchscreen/model/nextion/Nextion.cpp \
        application/io/touchscreen/model/viewtech/Viewtech.cpp
    endif
endif