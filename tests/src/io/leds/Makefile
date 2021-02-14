vpath application/%.cpp ../src
vpath common/%.cpp ../src

SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) := \
stubs/Core.cpp \
stubs/database/DB_ReadWrite.cpp \
application/database/Database.cpp \
application/database/CustomInit.cpp \
application/io/common/Common.cpp

ifneq (,$(findstring LEDS_SUPPORTED,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
    application/io/leds/LEDs.cpp
endif