vpath modules/%.cpp ../
vpath modules/%.c ../

#common include dirs
INCLUDE_DIRS := \
-I"../modules/" \
-I"board/$(ARCH)/variants/$(MCU_FAMILY)/$(MCU)/$(BOARD_DIR)/" \
-I"application/" \
-I"./"

INCLUDE_FILES += -include "board/$(ARCH)/variants/$(MCU_FAMILY)/$(MCU)/$(BOARD_DIR)/Hardware.h"

ifeq ($(BOOT),1)
    #bootloader only
    INCLUDE_DIRS += \
    -I"bootloader/"
endif

#architecture specific
ifeq ($(ARCH), avr)
    LINKER_FILE := board/$(ARCH)/variants/$(MCU_FAMILY)/$(MCU)/$(MCU).ld

    INCLUDE_DIRS += \
    -I"../modules/lufa/"

    ifneq ($(shell cat board/$(ARCH)/variants/$(MCU_FAMILY)/$(MCU)/$(BOARD_DIR)/Hardware.h | grep USB_MIDI_SUPPORTED), )
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
        modules/lufa/LUFA/Drivers/USB/Core/USBTask.c \
        modules/lufa/LUFA/Drivers/USB/Class/Device/AudioClassDevice.c \
        modules/lufa/LUFA/Drivers/USB/Class/Device/MIDIClassDevice.c
    endif
else ifeq ($(ARCH),stm32)
    LINKER_FILE := $(shell $(FIND) ./board/$(ARCH)/gen/$(MCU_FAMILY)/$(MCU) -name "*.ld" | head -n 1)

    SOURCES += $(shell $(FIND) ./board/stm32/gen/$(MCU_FAMILY)/common -regex '.*\.\(s\|c\)')
    SOURCES += $(shell $(FIND) ./board/stm32/gen/$(MCU_FAMILY)/$(MCU) -regex '.*\.\(s\|c\)')
    SOURCES += $(shell $(FIND) ./board/stm32/variants/$(MCU_FAMILY) -maxdepth 1 -name "*.cpp")
    SOURCES += $(shell $(FIND) ./board/stm32/eeprom -name "*.cpp")

    INCLUDE_DIRS += $(addprefix -I,$(shell $(FIND) ./board/stm32/gen/$(MCU_FAMILY)/common -type d -not -path "*Src*"))
    INCLUDE_DIRS += $(addprefix -I,$(shell $(FIND) ./board/stm32/gen/$(MCU_FAMILY)/$(MCU)/Drivers -type d -not -path "*Src*"))
    INCLUDE_DIRS += -I"./board/stm32/variants/$(MCU_FAMILY)/$(MCU)"
endif

#common for both bootloader and application
SOURCES += $(shell $(FIND) ./board/common -maxdepth 1 -type f -name "*.cpp")
SOURCES += $(shell $(FIND) ./board/$(ARCH)/variants/$(MCU_FAMILY)/$(MCU)/$(BOARD_DIR) -type f -name "*.cpp")

ifeq ($(BOOT),1)
    #bootloader sources
    #common
    SOURCES += \
    board/common/bootloader/Bootloader.cpp \
    board/common/io/Indicators.cpp \
    board/$(ARCH)/common/Bootloader.cpp \
    board/$(ARCH)/common/Init.cpp

    ifeq ($(ARCH),stm32)
        SOURCES += board/$(ARCH)/common/ISR.cpp
    endif

    SOURCES += $(shell find ./bootloader -type f -name "*.cpp")

    ifneq ($(shell cat board/$(ARCH)/variants/$(MCU_FAMILY)/$(MCU)/$(BOARD_DIR)/Hardware.h | grep USB_MIDI_SUPPORTED), )
        SOURCES += $(shell $(FIND) ./board/common/usb/descriptors/midi -type f -name "*.cpp")
        SOURCES += $(shell $(FIND) ./board/common/usb/descriptors/midi -type f -name "*.c")
        SOURCES += $(shell $(FIND) ./board/$(ARCH)/usb/midi -type f -name "*.cpp")

        ifneq ($(shell cat board/$(ARCH)/variants/$(MCU_FAMILY)/$(MCU)/$(BOARD_DIR)/Hardware.h | grep USB_LINK_MCU), )
            #for USB link MCUs, compile UART as well - needed to communicate with main MCU
            SOURCES += \
            board/$(ARCH)/uart/UART.cpp \
            board/common/uart/UART.cpp
        endif
    else
        SOURCES += \
        board/$(ARCH)/uart/UART.cpp \
        board/common/uart/UART.cpp

        SOURCES += $(shell $(FIND) ./common/OpenDeckMIDIformat -type f -name "*.cpp")
    endif
else
    #application sources
    #common for all targets
    SOURCES += $(shell $(FIND) ./board/$(ARCH)/common -type f -name "*.cpp")
    SOURCES += $(shell $(FIND) ./common/OpenDeckMIDIformat -type f -name "*.cpp")

    ifneq ($(shell cat board/$(ARCH)/variants/$(MCU_FAMILY)/$(MCU)/$(BOARD_DIR)/Hardware.h | grep USB_MIDI_SUPPORTED), )
        SOURCES += $(shell $(FIND) ./board/$(ARCH)/usb/midi -type f -name "*.cpp")
        SOURCES += $(shell $(FIND) ./board/common/usb/descriptors/midi -type f -name "*.cpp")
        SOURCES += $(shell $(FIND) ./board/common/usb/descriptors/midi -type f -name "*.c")
    endif

    ifneq ($(shell cat board/$(ARCH)/variants/$(MCU_FAMILY)/$(MCU)/$(BOARD_DIR)/Hardware.h | grep UART_INTERFACES), )
        SOURCES += \
        board/$(ARCH)/uart/UART.cpp \
        board/common/uart/UART.cpp
    endif

    ifneq ($(filter %16u2 %8u2, $(TARGETNAME)), )
        #fw for xu2 uses different set of sources than other targets
        SOURCES += \
        board/common/io/Indicators.cpp \
        usb-link/USBlink.cpp
    else
        SOURCES += $(shell $(FIND) ./board/common/io -type f -name "*.cpp" ! -name "*Bootloader*")
        SOURCES += $(shell $(FIND) ./application -maxdepth 1 -type f -name "*.cpp")
        SOURCES += $(shell $(FIND) ./application/database -type f -name "*.cpp")
        SOURCES += $(shell $(FIND) ./application/OpenDeck -type f -name "*.cpp")
        SOURCES += $(shell $(FIND) ./application/interface/analog -type f -name "*.cpp")
        SOURCES += $(shell $(FIND) ./application/interface/digital/input -type f -name "*.cpp")
        SOURCES += $(shell $(FIND) ./application/interface/digital/output/leds -maxdepth 1 -type f -name "*.cpp")
        SOURCES += $(shell $(FIND) ../modules/sysex/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
        SOURCES += $(shell $(FIND) ../modules/midi/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
        SOURCES += $(shell $(FIND) ../modules/dbms/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")

        #if a file named $(BOARD_DIR).cpp exists in ./application/interface/digital/output/leds/startup directory
        #add it to the sources
        ifneq (,$(wildcard ./application/interface/digital/output/leds/startup/$(BOARD_DIR).cpp))
            SOURCES += ./application/interface/digital/output/leds/startup/$(BOARD_DIR).cpp
        endif

        ifneq ($(shell cat board/$(ARCH)/variants/$(MCU_FAMILY)/$(MCU)/$(BOARD_DIR)/Hardware.h | grep DISPLAY_SUPPORTED), )
            SOURCES += $(shell $(FIND) ./application/interface/display -maxdepth 1 -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./application/interface/display/U8X8 -maxdepth 1 -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./application/interface/display/strings -maxdepth 1 -type f -name "*.cpp")

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

        ifneq ($(shell cat board/$(ARCH)/variants/$(MCU_FAMILY)/$(MCU)/$(BOARD_DIR)/Hardware.h | grep TOUCHSCREEN_SUPPORTED), )
            SOURCES += $(shell $(FIND) ./application/interface/display/touch -type f -name "*.cpp")
        endif
    endif
endif

#make sure all objects are located in build directory
OBJECTS := $(addprefix $(BUILD_DIR)/,$(SOURCES))
#also make sure objects have .o extension
OBJECTS := $(addsuffix .o,$(OBJECTS))

#include generated dependency files to allow incremental build when only headers change
-include $(OBJECTS:%.o=%.d)