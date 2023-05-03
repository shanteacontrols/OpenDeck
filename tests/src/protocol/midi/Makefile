vpath application/%.cpp ../src
vpath common/%.cpp ../src

ifeq (,$(findstring PROJECT_TARGET_USB_OVER_SERIAL_HOST,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) := \
    application/database/Database.cpp \
    application/database/CustomInit.cpp \
    application/protocol/midi/MIDI.cpp
endif