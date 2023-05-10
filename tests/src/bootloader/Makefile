vpath application/%.cpp ../src/firmware
vpath bootloader/%.cpp ../src/firmware/

ifeq (,$(findstring PROJECT_TARGET_USB_OVER_SERIAL_HOST,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) := \
    bootloader/SysExParser/SysExParser.cpp \
    bootloader/updater/Updater.cpp
endif