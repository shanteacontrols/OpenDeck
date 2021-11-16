vpath application/%.cpp ../src
vpath common/%.cpp ../src
vpath modules/%.cpp ../

ifeq (,$(findstring USB_LINK_MCU,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) := \
    stubs/database/DB_ReadWrite.cpp \
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

    ifneq (,$(findstring DISPLAY_SUPPORTED,$(DEFINES)))
        SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
        application/io/display/U8X8/U8X8.cpp \
        application/io/display/Display.cpp \
        application/io/display/strings/Strings.cpp
    endif

    ifneq (,$(findstring TOUCHSCREEN_SUPPORTED,$(DEFINES)))
        SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
        application/io/touchscreen/Touchscreen.cpp \
        application/io/touchscreen/model/Builder.cpp \
        application/io/touchscreen/model/nextion/Nextion.cpp \
        application/io/touchscreen/model/viewtech/Viewtech.cpp
    endif
endif