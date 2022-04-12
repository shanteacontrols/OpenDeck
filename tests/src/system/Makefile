vpath application/%.cpp ../src
vpath common/%.cpp ../src
vpath modules/%.cpp ../

ifeq (,$(findstring USB_OVER_SERIAL_HOST,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) := \
    application/database/Database.cpp \
    application/database/CustomInit.cpp \
    application/io/common/Common.cpp \
    application/system/System.cpp \
    application/util/cinfo/CInfo.cpp \
    application/protocol/midi/MIDI.cpp \
    application/protocol/dmx/DMX.cpp \
    modules/sysex/src/SysExConf.cpp \
    modules/dmxusb/src/DMXUSBWidget.cpp \
    application/io/buttons/Buttons.cpp \
    application/io/encoders/Encoders.cpp \
    application/io/leds/LEDs.cpp \
    application/io/analog/Analog.cpp \

    ifneq (,$(findstring I2C_SUPPORTED,$(DEFINES)))
        SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
        application/io/i2c/I2C.cpp \
        application/io/i2c/peripherals/Builder.cpp \
        application/io/i2c/peripherals/display/Display.cpp \
        application/io/i2c/peripherals/display/strings/Strings.cpp
    endif

    ifneq (,$(findstring TOUCHSCREEN_SUPPORTED,$(DEFINES)))
        SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
        application/io/touchscreen/Touchscreen.cpp \
        application/io/touchscreen/model/Builder.cpp \
        application/io/touchscreen/model/nextion/Nextion.cpp \
        application/io/touchscreen/model/viewtech/Viewtech.cpp
    endif
endif