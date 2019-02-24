ifeq ($(BOARD_DIR),pro_micro)
    #pro micro is just a leonardo variant
    BOARD_DIR := leonardo
else ifneq ($(filter %16u2 %8u2, $(MAKECMDGOALS)), )
    #16u2 and 8u2 are basically same
    BOARD_DIR := xu2
endif

#common include dirs
INCLUDE_DIRS := \
-I"modules/lufa/" \
-I"modules/" \
-I"application/board/avr/variants/$(BOARD_DIR)/" \
-I"application/" \

ifeq ($(findstring fw_,$(MAKECMDGOALS)), fw_)
    ifneq ($(BOARD_DIR),xu2)
        INCLUDE_FILES += -include "application/board/avr/variants/$(BOARD_DIR)/Hardware.h"
    endif
else
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
    ifeq ($(findstring boot,$(MAKECMDGOALS)), boot)
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

ifeq ($(findstring boot,$(MAKECMDGOALS)), boot)
    #bootloader sources
    #common
    SOURCES += \
    bootloader/mcu/BootloaderHID.cpp

    ifneq ($(filter USB_SUPPORTED, $(DEFINES)), )
        ifeq ($(BOARD_DIR),xu2)
            SOURCES += \
            bootloader/mcu/variant/xu2.cpp \
            application/board/avr/uart/UART.cpp
        else
            SOURCES += \
            bootloader/mcu/variant/NativeUSB.cpp
        endif
    else
        SOURCES += \
        bootloader/mcu/variant/NoUSB.cpp \
        application/board/avr/uart/UART.cpp
    endif
else
    #application sources
    ifeq ($(BOARD_DIR),xu2)
        #fw for xu2 uses different set of sources than other targets
        SOURCES += \
        modules/core/src/HAL/avr/reset/Reset.cpp \
        application/board/avr/variants/Common.cpp \
        application/board/avr/variants/$(BOARD_DIR)/xu2.cpp \
        application/board/avr/variants/$(BOARD_DIR)/ISR.cpp \
        application/board/avr/uart/UART.cpp \
        application/board/avr/uart/OpenDeckMIDIFormat.cpp \
        application/board/avr/usb/USB_MIDI.cpp
    else
        SOURCES += \
        application/main.cpp \
        application/OpenDeck/OpenDeck.cpp \
        modules/core/src/HAL/avr/reset/Reset.cpp \
        application/database/Database.cpp \
        modules/dbms/src/DBMS.cpp \
        application/interface/analog/Analog.cpp \
        application/interface/analog/FSR.cpp \
        application/interface/analog/Potentiometer.cpp \
        application/interface/digital/input/Common.cpp \
        application/interface/digital/input/buttons/Buttons.cpp \
        application/interface/digital/input/buttons/Hooks.cpp \
        application/interface/digital/input/encoders/Encoders.cpp \
        application/OpenDeck/sysconfig/SysConfig.cpp \
        modules/midi/src/MIDI.cpp \
        modules/sysex/src/SysEx.cpp \
        application/board/avr/variants/Common.cpp \
        application/board/avr/variants/ISR.cpp \
        application/board/common/analog/input/Common.cpp \
        application/board/common/digital/input/encoders/Common.cpp \
        application/board/common/digital/input/DigitalIn.cpp \
        application/board/avr/uart/OpenDeckMIDIFormat.cpp

        ifneq ($(filter LEDS_SUPPORTED, $(DEFINES)), )
        SOURCES += \
        application/interface/digital/output/leds/LEDs.cpp \
        application/board/common/digital/output/DigitalOut.cpp \
        application/board/common/digital/output/leds/Common.cpp

            ifneq ($(filter OUT_MATRIX, $(DEFINES)), )
            SOURCES += \
            application/board/common/digital/output/leds/Matrix.cpp
            else
            SOURCES += \
            application/board/common/digital/output/leds/DirectOut.cpp
            endif
        endif

        #board specific
        ifneq ($(filter USE_MUX, $(DEFINES)), )
        SOURCES += \
        application/board/common/analog/input/Mux.cpp
        endif

        ifneq ($(filter IN_MATRIX, $(DEFINES)), )
        SOURCES += \
        application/board/common/digital/input/buttons/Matrix.cpp \
        application/board/common/digital/input/encoders/Matrix.cpp
        else
        SOURCES += \
        application/board/common/digital/input/buttons/DirectIn.cpp \
        application/board/common/digital/input/encoders/DirectIn.cpp
        endif

        SOURCES += \
        application/board/avr/usb/USB_MIDI.cpp

        SOURCES += \
        application/board/avr/uart/UART.cpp

        ifneq ($(filter DISPLAY_SUPPORTED, $(DEFINES)), )
            SOURCES += \
            application/interface/display/U8X8/U8X8.cpp \
            application/interface/display/UpdateLogic.cpp \
            application/interface/display/TextBuild.cpp

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

            #i2c for display communication
            SOURCES += modules/core/src/HAL/avr/i2c/I2C.cpp
        endif

        ifneq ($(filter TOUCHSCREEN_SUPPORTED, $(DEFINES)), )
            SOURCES += \
            application/interface/display/touch/Touchscreen.cpp \
            application/interface/display/touch/model/sdw/SDW.cpp
        endif
    endif
endif