vpath application/%.cpp ../src
vpath common/%.cpp ../src
vpath modules/%.cpp ../

SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) := \
stubs/Core.cpp \
stubs/database/DB_ReadWrite.cpp \
application/database/Database.cpp \
application/database/CustomInit.cpp \
application/io/common/Common.cpp \
application/system/System.cpp \
application/system/Get.cpp \
application/system/Set.cpp \
modules/sysex/src/SysExConf.cpp

ifneq (,$(findstring BUTTONS_SUPPORTED,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
    application/io/buttons/Buttons.cpp
endif

ifneq (,$(findstring ENCODERS_SUPPORTED,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
    application/io/encoders/Encoders.cpp
endif

ifneq (,$(findstring LEDS_SUPPORTED,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
    application/io/leds/LEDs.cpp
endif

ifneq (,$(findstring ANALOG_SUPPORTED,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
    application/io/analog/Analog.cpp
endif

ifneq (,$(findstring DISPLAY_SUPPORTED,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
    application/io/display/U8X8/U8X8.cpp \
    application/io/display/Display.cpp \
    application/io/display/strings/Strings.cpp
endif

ifneq (,$(findstring TOUCHSCREEN_SUPPORTED,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
    application/io/touchscreen/Touchscreen.cpp
endif