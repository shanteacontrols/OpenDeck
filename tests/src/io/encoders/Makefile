vpath application/%.cpp ../src
vpath common/%.cpp ../src

SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) := \
stubs/Core.cpp \
stubs/database/DB_ReadWrite.cpp \
application/database/Database.cpp \
application/io/common/Common.cpp \

ifneq (,$(findstring LEDS_SUPPORTED,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
    application/io/leds/LEDs.cpp
endif

ifneq (,$(findstring ENCODERS_SUPPORTED,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
    application/io/encoders/Encoders.cpp
endif

ifneq (,$(findstring DISPLAY_SUPPORTED,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
    application/io/display/U8X8/U8X8.cpp \
    application/io/display/UpdateLogic.cpp \
    application/io/display/TextBuild.cpp \
    application/io/display/strings/Strings.cpp
endif