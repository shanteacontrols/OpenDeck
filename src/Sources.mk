vpath modules/%.cpp ../
vpath modules/%.c ../

#common include dirs
INCLUDE_DIRS := \
-I"../modules/" \
-I"board/$(ARCH)/variants/$(MCU)/$(BOARD_DIR)/" \
-I"application/" \
-I"./"

INCLUDE_FILES += -include "board/$(ARCH)/variants/$(MCU)/$(BOARD_DIR)/Hardware.h"

ifeq ($(findstring boot_,$(TARGETNAME)), boot_)
    #bootloader only
    INCLUDE_DIRS += \
    -I"bootloader/mcu/"
endif

#architecture specific
ifeq ($(ARCH), avr)
    INCLUDE_DIRS += \
    -I"../modules/lufa/"

    ifneq ($(shell cat board/$(ARCH)/variants/$(MCU)/$(BOARD_DIR)/Hardware.h | grep USB_MIDI_SUPPORTED), )
        #common for bootloader and application
        SOURCES += \
        modules/lufa/LUFA/Drivers/USB/Core/AVR8/Device_AVR8.c \
        modules/lufa/LUFA/Drivers/USB/Core/AVR8/EndpointStream_AVR8.c \
        modules/lufa/LUFA/Drivers/USB/Core/AVR8/Endpoint_AVR8.c \
        modules/lufa/LUFA/Drivers/USB/Core/AVR8/PipeStream_AVR8.c \
        modules/lufa/LUFA/Drivers/USB/Core/AVR8/Pipe_AVR8.c \
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
            modules/lufa/LUFA/Drivers/USB/Class/Common/HIDParser.c \
            modules/lufa/LUFA/Drivers/USB/Class/Device/HIDClassDevice.c
        else
            #application
            SOURCES += \
            modules/lufa/LUFA/Drivers/USB/Class/Device/AudioClassDevice.c \
            modules/lufa/LUFA/Drivers/USB/Class/Device/MIDIClassDevice.c
        endif
    endif
else ifeq ($(ARCH),stm32)
    SOURCES += $(shell find ./board/stm32/gen/$(MCU)/Drivers/STM32F4xx_HAL_Driver/Src -name "*.c")
    SOURCES += $(shell find ./board/stm32/gen/$(MCU)/Middlewares/ST/STM32_USB_Device_Library/Core/Src -name "*.c")
    SOURCES += $(shell find ./board/stm32/eeprom -name "*.cpp")
    SOURCES += \
    ./board/stm32/gen/$(MCU)/startup_stm32f407xx.s \
    ./board/stm32/gen/$(MCU)/Src/system_stm32f4xx.c \
    ./board/stm32/gen/$(MCU)/Src/stm32f4xx_hal_msp.c \
    ./board/stm32/gen/$(MCU)/Src/usbd_conf.c

    INCLUDE_DIRS += \
    -I"./board/stm32/gen/$(MCU)/Inc" \
    -I"./board/stm32/gen/$(MCU)/Drivers/STM32F4xx_HAL_Driver/Inc" \
    -I"./board/stm32/gen/$(MCU)/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" \
    -I"./board/stm32/gen/$(MCU)/Drivers/CMSIS/Device/ST/STM32F4xx/Include" \
    -I"./board/stm32/gen/$(MCU)/Drivers/CMSIS/Include" \
    -I"./board/stm32/gen/$(MCU)/Middlewares/ST/STM32_USB_Device_Library/Core/Inc"
endif

ifeq ($(findstring boot,$(TARGETNAME)), boot)
    #bootloader sources
    #common
    SOURCES += \
    bootloader/mcu/main.cpp \
    board/$(ARCH)/Common.cpp \
    board/$(ARCH)/variants/$(MCU)/$(BOARD_DIR)/Setup.cpp \
    board/common/io/Bootloader.cpp

    ifneq ($(shell cat board/$(ARCH)/variants/$(MCU)/$(BOARD_DIR)/Hardware.h | grep USB_MIDI_SUPPORTED), )
        SOURCES += $(shell find ./board/common/usb/descriptors/hid -type f -name "*.cpp")
        SOURCES += $(shell find ./board/common/usb/descriptors/hid -type f -name "*.c")
        SOURCES += $(shell find ./board/$(ARCH)/usb/hid -type f -name "*.cpp")

        ifneq ($(filter %16u2 %8u2, $(TARGETNAME)), )
            SOURCES += \
            bootloader/mcu/variant/xu2.cpp \
            board/$(ARCH)/UART_LL.cpp \
            board/common/UART.cpp
        else
            SOURCES += \
            bootloader/mcu/variant/NativeUSB.cpp
        endif
    else
        SOURCES += \
        bootloader/mcu/variant/NoUSB.cpp \
        board/$(ARCH)/UART_LL.cpp \
        board/common/UART.cpp \
        common/OpenDeckMIDIformat/OpenDeckMIDIformat.cpp
    endif
else
    #application sources
    #common for all targets
    SOURCES += $(shell find ./board/$(ARCH) -maxdepth 1 -type f -name "*.cpp")
    SOURCES += $(shell find ./board/$(ARCH)/variants/$(MCU)/$(BOARD_DIR) -type f -name "*.cpp")
    SOURCES += $(shell find ./common/OpenDeckMIDIformat -type f -name "*.cpp")

    ifneq ($(shell cat board/$(ARCH)/variants/$(MCU)/$(BOARD_DIR)/Hardware.h | grep USB_MIDI_SUPPORTED), )
        SOURCES += $(shell find ./board/$(ARCH)/usb/midi -type f -name "*.cpp")
        SOURCES += $(shell find ./board/common/usb/descriptors/midi -type f -name "*.cpp")
        SOURCES += $(shell find ./board/common/usb/descriptors/midi -type f -name "*.c")
    endif

    ifneq ($(filter %16u2 %8u2, $(TARGETNAME)), )
        #fw for xu2 uses different set of sources than other targets
        SOURCES += \
        board/common/io/Indicators.cpp \
        board/common/UART.cpp \
        usb-link/USBlink.cpp
    else
        SOURCES += $(shell find ./board/common -maxdepth 1 -type f -name "*.cpp")
        SOURCES += $(shell find ./board/common/io -type f -name "*.cpp" ! -name "*Bootloader*")
        SOURCES += $(shell find ./application -maxdepth 1 -type f -name "*.cpp")
        SOURCES += $(shell find ./application/database -type f -name "*.cpp")
        SOURCES += $(shell find ./application/OpenDeck -type f -name "*.cpp")
        SOURCES += $(shell find ./application/interface/analog -type f -name "*.cpp")
        SOURCES += $(shell find ./application/interface/digital/input/ -type f -name "*.cpp")
        SOURCES += $(shell find ./application/interface/digital/output/leds/ -maxdepth 1 -type f -name "*.cpp")
        SOURCES += $(shell find ../modules/sysex/src -maxdepth 1 -type f -name "*.cpp")
        SOURCES += $(shell find ../modules/midi/src -maxdepth 1 -type f -name "*.cpp")
        SOURCES += $(shell find ../modules/dbms/src -maxdepth 1 -type f -name "*.cpp")

        #if a file named $(BOARD_DIR).cpp exists in ./application/interface/digital/output/leds/startup directory
        #add it to the sources
        ifneq (,$(wildcard ./application/interface/digital/output/leds/startup/$(BOARD_DIR).cpp))
            SOURCES += ./application/interface/digital/output/leds/startup/$(BOARD_DIR).cpp
        endif

        ifneq ($(shell cat board/$(ARCH)/variants/$(MCU)/$(BOARD_DIR)/Hardware.h | grep DISPLAY_SUPPORTED), )
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

        ifneq ($(shell cat board/$(ARCH)/variants/$(MCU)/$(BOARD_DIR)/Hardware.h | grep TOUCHSCREEN_SUPPORTED), )
            SOURCES += $(shell find ./application/interface/display/touch -type f -name "*.cpp")
        endif
    endif
endif

#make sure all objects are located in build directory
OBJECTS := $(addprefix $(BUILD_DIR)/,$(SOURCES))
#also make sure objects have .o extension
OBJECTS := $(OBJECTS:.c=.o)
OBJECTS := $(OBJECTS:.cpp=.o)
OBJECTS := $(OBJECTS:.s=.o)

#include generated dependency files to allow incremental build when only headers change
-include $(OBJECTS:%.o=%.d)