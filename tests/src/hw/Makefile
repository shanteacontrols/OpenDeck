vpath application/%.cpp ../src/firmware
vpath board/%.cpp ../src/firmware

ifeq (,$(findstring PROJECT_TARGET_USB_OVER_SERIAL_HOST,$(DEFINES)))
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
    application/io/analog/Analog.cpp
endif