vpath application/%.cpp ../src
vpath board/%.cpp ../src

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
    application/io/analog/Analog.cpp
endif