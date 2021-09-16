vpath application/%.cpp ../src
vpath bootloader/%.cpp ../src
vpath common/%.cpp ../src

ifeq (,$(findstring USB_LINK_MCU,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) := \
    bootloader/SysExParser/SysExParser.cpp \
    bootloader/updater/Updater.cpp
endif