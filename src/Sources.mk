ifeq ($(BOARD_DIR),pro_micro)
    #pro micro is just a leonardo variant
    BOARD_DIR := leonardo
else ifneq ($(filter %16u2 %8u2, $(TARGETNAME)), )
    #16u2 and 8u2 are basically same
    BOARD_DIR := xu2
endif

#common include dirs
INCLUDE_DIRS := \
-I"modules/lufa/" \
-I"modules/" \
-I"application/board/avr/variants/$(BOARD_DIR)/" \
-I"application/" \

INCLUDE_FILES += -include "application/board/avr/variants/$(BOARD_DIR)/Hardware.h"

ifeq ($(findstring boot_,$(TARGETNAME)), boot_)
    #bootloader only
    INCLUDE_DIRS += \
    -I"bootloader/mcu/"
endif

SOURCES :=

#lufa sources
ifneq ($(filter USB_SUPPORTED, $(DEFINES)), )
    #common for bootloader and application
    SOURCES += \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/Device_AVR8.c \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/EndpointStream_AVR8.c \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/Endpoint_AVR8.c \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/PipeStream_AVR8.c \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/Pipe_AVR8.c \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/Template/Template_Endpoint_Control_R.c \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/Template/Template_Endpoint_Control_W.c \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/Template/Template_Endpoint_RW.c \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/Template/Template_Pipe_RW.c \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/USBController_AVR8.c \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/USBInterrupt_AVR8.c \
    modules/lufa/LUFA/Drivers/USB/Core/ConfigDescriptors.c \
    modules/lufa/LUFA/Drivers/USB/Core/DeviceStandardReq.c \
    modules/lufa/LUFA/Drivers/USB/Core/Events.c \
    modules/lufa/LUFA/Drivers/USB/Core/USBTask.c

    #additional sources differ for application and bootloader
    ifeq ($(findstring boot,$(TARGETNAME)), boot)
        #bootloader
        SOURCES += \
        bootloader/mcu/Descriptors.c \
        modules/lufa/LUFA/Drivers/USB/Class/Common/HIDParser.c \
        modules/lufa/LUFA/Drivers/USB/Class/Device/HIDClassDevice.c
    else
        #application
        SOURCES += \
        application/board/avr/usb/Descriptors.c \
        modules/lufa/LUFA/Drivers/USB/Class/Device/AudioClassDevice.c \
        modules/lufa/LUFA/Drivers/USB/Class/Device/MIDIClassDevice.c
    endif
endif

ifeq ($(findstring boot,$(TARGETNAME)), boot)
    #bootloader sources
    #common
    SOURCES += \
    bootloader/mcu/BootloaderHID.cpp

    ifneq ($(filter USB_SUPPORTED, $(DEFINES)), )
        ifeq ($(BOARD_DIR),xu2)
            SOURCES += \
            bootloader/mcu/variant/xu2.cpp \
            common/board/avr/uart/UART.cpp
        else
            SOURCES += \
            bootloader/mcu/variant/NativeUSB.cpp
        endif
    else
        SOURCES += \
        bootloader/mcu/variant/NoUSB.cpp \
        common/board/avr/uart/UART.cpp \
        common/OpenDeckMIDIformat/OpenDeckMIDIformat.cpp
    endif
else
    #application sources
    ifeq ($(BOARD_DIR),xu2)
        #fw for xu2 uses different set of sources than other targets
        SOURCES += \
        application/board/common/digital/Output.cpp

        SOURCES += $(shell find ./application/board/avr -maxdepth 1 -type f -name "*.cpp")
        SOURCES += $(shell find ./application/board/avr/usb -type f -name "*.cpp")
        SOURCES += $(shell find ./application/board/avr/variants/$(BOARD_DIR) -type f -name "*.cpp")
        SOURCES += $(shell find ./common/OpenDeckMIDIformat -type f -name "*.cpp")
        SOURCES += $(shell find ./common/board/avr -type f -name "*.cpp")
    else
        SOURCES += $(shell find ./application -maxdepth 1 -type f -name "*.cpp")
        SOURCES += $(shell find ./application/database -type f -name "*.cpp")
        SOURCES += $(shell find ./application/OpenDeck -type f -name "*.cpp")
        SOURCES += $(shell find ./application/interface/analog -type f -name "*.cpp")
        SOURCES += $(shell find ./application/interface/digital -type f -name "*.cpp")
        SOURCES += $(shell find ./application/board/common/digital -type f -name "*.cpp")
        SOURCES += $(shell find ./application/board/common/analog -type f -name "*.cpp")
        SOURCES += $(shell find ./application/board/avr -maxdepth 1 -type f -name "*.cpp")
        SOURCES += $(shell find ./application/board/avr/variants/$(BOARD_DIR) -type f -name "*.cpp")
        SOURCES += $(shell find ./modules/sysex/src -maxdepth 1 -type f -name "*.cpp")
        SOURCES += $(shell find ./modules/midi/src -maxdepth 1 -type f -name "*.cpp")
        SOURCES += $(shell find ./modules/dbms/src -maxdepth 1 -type f -name "*.cpp")
        SOURCES += $(shell find ./common/OpenDeckMIDIformat -type f -name "*.cpp")
        SOURCES += $(shell find ./common/board/avr -type f -name "*.cpp")

        ifneq ($(filter USB_SUPPORTED, $(DEFINES)), )
            SOURCES += $(shell find ./application/board/avr/usb -type f -name "*.cpp")
        endif

        ifneq ($(shell cat application/board/avr/variants/$(BOARD_DIR)/Hardware.h | grep DISPLAY_SUPPORTED), )
            SOURCES += $(shell find ./application/interface/display -maxdepth 1 -type f -name "*.cpp")
            SOURCES += $(shell find ./application/interface/display/U8X8 -maxdepth 1 -type f -name "*.cpp")
            SOURCES += $(shell find ./application/interface/display/strings -maxdepth 1 -type f -name "*.cpp")

            #u8x8 sources
            SOURCES += \
            modules/u8g2/csrc/u8x8_string.c \
            modules/u8g2/csrc/u8x8_setup.c \
            modules/u8g2/csrc/u8x8_u8toa.c \
            modules/u8g2/csrc/u8x8_8x8.c \
            modules/u8g2/csrc/u8x8_u16toa.c \
            modules/u8g2/csrc/u8x8_display.c \
            modules/u8g2/csrc/u8x8_fonts.c \
            modules/u8g2/csrc/u8x8_byte.c \
            modules/u8g2/csrc/u8x8_cad.c \
            modules/u8g2/csrc/u8x8_gpio.c \
            modules/u8g2/csrc/u8x8_d_ssd1306_128x64_noname.c \
            modules/u8g2/csrc/u8x8_d_ssd1306_128x32.c
        endif

        ifneq ($(shell cat application/board/avr/variants/$(BOARD_DIR)/Hardware.h | grep TOUCHSCREEN_SUPPORTED), )
            SOURCES += $(shell find ./application/interface/display/touch -type f -name "*.cpp")
        endif
    endif
endif

#make sure all objects are located in build directory
OBJECTS := $(addprefix build/,$(SOURCES))
#also make sure objects have .o extension
OBJECTS := $(OBJECTS:.c=.o)
OBJECTS := $(OBJECTS:.cpp=.o)

#include generated dependency files to allow incremental build when only headers change
-include $(OBJECTS:%.o=%.d)