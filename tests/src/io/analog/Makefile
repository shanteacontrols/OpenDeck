vpath application/%.cpp ../src/firmware

ifeq (,$(findstring PROJECT_TARGET_USB_OVER_SERIAL_HOST,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) := \
    application/database/Database.cpp \
    application/database/CustomInit.cpp \
    application/io/buttons/Buttons.cpp \
    application/io/leds/LEDs.cpp \
    application/io/analog/Analog.cpp
endif