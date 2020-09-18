vpath application/%.cpp ../src
vpath common/%.cpp ../src

SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) := \
stubs/Core.cpp \
stubs/database/DB_ReadWrite.cpp \
application/database/Database.cpp \
application/database/CustomInit.cpp

ifneq (,$(findstring LEDS_SUPPORTED,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
    application/io/leds/LEDs.cpp
endif

ifneq (,$(findstring ANALOG_SUPPORTED,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
    application/io/analog/Analog.cpp \
    application/io/analog/Potentiometer.cpp \
    application/io/analog/FSR.cpp
endif

ifneq (,$(findstring DISPLAY_SUPPORTED,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
    application/io/display/U8X8/U8X8.cpp \
    application/io/display/Display.cpp \
    application/io/display/strings/Strings.cpp
endif