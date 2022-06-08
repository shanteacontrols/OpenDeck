vpath application/%.cpp ../src
vpath common/%.cpp ../src

ifeq (,$(findstring USB_OVER_SERIAL_HOST,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) := \
    application/database/Database.cpp \
    application/database/CustomInit.cpp \
    application/io/leds/LEDs.cpp
endif