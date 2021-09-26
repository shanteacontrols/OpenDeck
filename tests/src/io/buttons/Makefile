vpath application/%.cpp ../src
vpath common/%.cpp ../src

ifeq (,$(findstring USB_LINK_MCU,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) := \
    stubs/database/DB_ReadWrite.cpp \
    application/database/Database.cpp \
    application/database/CustomInit.cpp \
    application/io/common/Common.cpp

    ifneq (,$(findstring LEDS_SUPPORTED,$(DEFINES)))
        SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
        application/io/leds/LEDs.cpp
    endif

    ifneq (,$(findstring BUTTONS_SUPPORTED,$(DEFINES)))
        SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
        application/io/buttons/Buttons.cpp
    endif

    ifneq (,$(findstring DISPLAY_SUPPORTED,$(DEFINES)))
        SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) += \
        application/io/display/U8X8/U8X8.cpp \
        application/io/display/Display.cpp \
        application/io/display/strings/Strings.cpp
    endif
endif