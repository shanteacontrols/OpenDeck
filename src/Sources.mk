vpath modules/%.cpp ../
vpath modules/%.c ../

#common include dirs
INCLUDE_DIRS := \
-I"../modules/" \
-I"board/gen/$(TARGET)/" \
-I"application/" \
-I"board/$(ARCH)/variants/$(MCU_FAMILY)" \
-I"board/$(ARCH)/variants/$(MCU_FAMILY)/$(MCU)" \
-I"./"

ifeq ($(TYPE),boot)
    #bootloader only
    INCLUDE_DIRS += \
    -I"bootloader/"
endif

LINKER_FILE := board/$(ARCH)/variants/$(MCU_FAMILY)/$(MCU)/$(MCU).ld

PINS_GEN_SOURCE := board/gen/$(TARGET)/Pins.cpp

ifneq (,$(wildcard application/io/touchscreen/design/$(TARGET).json))
    TSCREEN_GEN_SOURCE += application/io/touchscreen/design/$(TARGET).cpp
endif

ifeq (,$(findstring flashgen,$(MAKECMDGOALS)))
    SOURCES += $(PINS_GEN_SOURCE)
    SOURCES += $(TSCREEN_GEN_SOURCE)

    #architecture specific
    ifeq ($(ARCH), avr)
        INCLUDE_DIRS += \
        -I"../modules/lufa/"

        ifneq (,$(findstring USB_MIDI_SUPPORTED,$(DEFINES)))
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
        SOURCES += $(shell $(FIND) ./board/stm32/gen/common -regex '.*\.\(s\|c\)')
        SOURCES += $(shell $(FIND) ./board/stm32/gen/$(MCU_FAMILY)/common -regex '.*\.\(s\|c\)')
        SOURCES += $(shell $(FIND) ./board/stm32/gen/$(MCU_FAMILY)/$(MCU_BASE) -regex '.*\.\(s\|c\)')
        SOURCES += $(shell $(FIND) ./board/stm32/variants/$(MCU_FAMILY) -maxdepth 1 -name "*.cpp")
        SOURCES += modules/EmuEEPROM/src/EmuEEPROM.cpp
    
        INCLUDE_DIRS += $(addprefix -I,$(shell $(FIND) ./board/stm32/gen/common -type d -not -path "*Src*"))
        INCLUDE_DIRS += $(addprefix -I,$(shell $(FIND) ./board/stm32/gen/$(MCU_FAMILY)/common -type d -not -path "*Src*"))
        INCLUDE_DIRS += $(addprefix -I,$(shell $(FIND) ./board/stm32/gen/$(MCU_FAMILY)/$(MCU_BASE)/Drivers -type d -not -path "*Src*"))
        INCLUDE_DIRS += -I"./board/stm32/variants/$(MCU_FAMILY)/$(MCU)"

        TOOLCHAIN_DIR := $(shell dirname $(shell which arm-none-eabi-gcc) | rev | cut -c5- | rev)
        CPP_VER := $(shell $(FIND) $(TOOLCHAIN_DIR)/arm-none-eabi/include/c++ -mindepth 1 -maxdepth 1 | rev | cut -d/ -f 1 | rev)

        INCLUDE_DIRS += \
        -isystem"$(TOOLCHAIN_DIR)/arm-none-eabi/include" \
        -isystem"$(TOOLCHAIN_DIR)/arm-none-eabi/include/c++/$(CPP_VER)" \
        -isystem"$(TOOLCHAIN_DIR)/arm-none-eabi/include/c++/$(CPP_VER)/arm-none-eabi" \
        -isystem"$(TOOLCHAIN_DIR)arm-none-eabi/include/c++/$(CPP_VER)/backward" \
        -isystem"$(TOOLCHAIN_DIR)/lib/gcc/arm-none-eabi/$(CPP_VER)/include" \
        -isystem"$(TOOLCHAIN_DIR)/lib/gcc/arm-none-eabi/$(CPP_VER)/include-fixed"
    endif

    #common for both bootloader and application
    SOURCES += $(shell $(FIND) ./board/common -maxdepth 1 -type f -name "*.cpp")
    SOURCES += $(shell $(FIND) ./board/$(ARCH)/variants/$(MCU_FAMILY)/$(MCU) -maxdepth 1 -type f -regex '.*\.\(s\|c\|cpp\)')

    ifeq ($(TYPE),boot)
        #bootloader sources
        #common
        SOURCES += \
        board/common/bootloader/Bootloader.cpp \
        board/common/io/Indicators.cpp \
        board/$(ARCH)/common/Bootloader.cpp \
        board/$(ARCH)/common/Init.cpp \
        board/$(ARCH)/common/ShiftRegistersWait.cpp

        ifeq ($(ARCH),avr)
            SOURCES += board/$(ARCH)/common/Flash.cpp
            SOURCES += board/$(ARCH)/common/FlashPages.cpp
        else ifeq ($(ARCH),stm32)
            SOURCES += board/$(ARCH)/common/ISR.cpp
        endif

        SOURCES += $(shell find ./bootloader -type f -name "*.cpp")

        ifneq (,$(findstring USB_MIDI_SUPPORTED,$(DEFINES)))
            SOURCES += $(shell $(FIND) ./board/common/usb/descriptors/midi -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./board/common/usb/descriptors/midi -type f -name "*.c")
            SOURCES += $(shell $(FIND) ./board/$(ARCH)/usb/midi -type f -name "*.cpp")

            ifneq (,$(findstring USB_LINK_MCU,$(DEFINES)))
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
    else ifeq ($(TYPE),app)
        #application sources
        #common for all targets
        SOURCES += $(shell $(FIND) ./board/$(ARCH)/common -type f -name "*.cpp")
        SOURCES += $(shell $(FIND) ./common/OpenDeckMIDIformat -type f -name "*.cpp")

        ifneq (,$(findstring USB_MIDI_SUPPORTED,$(DEFINES)))
            SOURCES += $(shell $(FIND) ./board/$(ARCH)/usb/midi -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./board/common/usb/descriptors/midi -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./board/common/usb/descriptors/midi -type f -name "*.c")
        endif

        ifneq (,$(findstring USE_UART,$(DEFINES)))
            SOURCES += \
            board/$(ARCH)/uart/UART.cpp \
            board/common/uart/UART.cpp
        endif

        ifneq ($(filter %16u2 %8u2, $(TARGET)), )
            #fw for xu2 uses different set of sources than other targets
            SOURCES += \
            board/common/io/Indicators.cpp \
            usb-link/main.cpp
        else
            SOURCES += $(shell $(FIND) ./application -maxdepth 1 -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./application/database -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./application/system -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./application/io/common -maxdepth 1 -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ../modules/sysex/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
            SOURCES += $(shell $(FIND) ../modules/midi/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
            SOURCES += $(shell $(FIND) ../modules/dbms/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
            SOURCES += board/common/io/Stubs.cpp

            ifneq (,$(findstring ANALOG_SUPPORTED,$(DEFINES)))
                SOURCES += $(shell $(FIND) ./application/io/analog -type f -name "*.cpp")
                SOURCES += board/common/io/Analog.cpp
            endif

            ifneq (,$(findstring LEDS_SUPPORTED,$(DEFINES)))
                SOURCES += $(shell $(FIND) ./application/io/leds -maxdepth 1 -type f -name "*.cpp")
                SOURCES += board/common/io/Output.cpp
            endif

            ifneq (,$(findstring BUTTONS_SUPPORTED,$(DEFINES)))
                SOURCES += $(shell $(FIND) ./application/io/buttons -type f -name "*.cpp")
            endif

            ifneq (,$(findstring ENCODERS_SUPPORTED,$(DEFINES)))
                SOURCES += $(shell $(FIND) ./application/io/encoders -maxdepth 1 -type f -name "*.cpp")
            endif

            ifneq (,$(filter $(DEFINES),ENCODERS_SUPPORTED BUTTONS_SUPPORTED))
                SOURCES += board/common/io/Input.cpp
            endif

            ifneq (,$(findstring LED_INDICATORS,$(DEFINES)))
                SOURCES += board/common/io/Indicators.cpp
            endif

            #if a file named $(TARGET).cpp exists in ./application/io/leds/startup directory
            #add it to the sources
            ifneq (,$(wildcard ./application/io/leds/startup/$(TARGET).cpp))
                SOURCES += ./application/io/leds/startup/$(TARGET).cpp
            endif

            ifneq (,$(findstring TOUCHSCREEN_SUPPORTED,$(DEFINES)))
                SOURCES += $(shell $(FIND) ./application/io/touchscreen -maxdepth 1 -type f -name "*.cpp")
                SOURCES += $(shell $(FIND) ./application/io/touchscreen/model/sdw -maxdepth 1 -type f -name "*.cpp")
                SOURCES += $(shell $(FIND) ./application/io/touchscreen/model -type f -name "*.cpp")
            endif

            ifneq (,$(findstring DISPLAY_SUPPORTED,$(DEFINES)))
                SOURCES += $(shell $(FIND) ./application/io/display -type f -name "*.cpp")
                SOURCES += $(shell $(FIND) ./board/$(ARCH)/i2c -type f -name "*.cpp")

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
        endif
    else ifeq ($(TYPE),cdc)
        #cdc mode sources
        #stm32 only
        SOURCES += \
        board/$(ARCH)/common/Init.cpp \
        board/common/io/Indicators.cpp \
        board/$(ARCH)/common/ShiftRegistersWait.cpp \
        board/$(ARCH)/common/ISR.cpp \
        board/$(ARCH)/uart/UART.cpp \
        board/common/uart/UART.cpp

        SOURCES += $(shell $(FIND) ./board/common/usb/descriptors/cdc -type f -name "*.cpp")
        SOURCES += $(shell $(FIND) ./board/common/usb/descriptors/cdc -type f -name "*.c")
        SOURCES += $(shell $(FIND) ./board/$(ARCH)/usb/cdc -type f -name "*.cpp")
        SOURCES += $(shell find ./cdc -type f -name "*.cpp")
    endif
else
    SOURCES += $(shell $(FIND) ./application/database -type f -name "*.cpp")
    SOURCES += $(shell $(FIND) ../modules/dbms/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
    SOURCES += modules/EmuEEPROM/src/EmuEEPROM.cpp
    SOURCES += board/$(shell yq r ../targets/$(TARGET).yml arch)/variants/$(MCU_FAMILY)/$(MCU)/FlashPages.cpp
    SOURCES += $(TSCREEN_GEN_SOURCE)
    SOURCES += flashgen/main.cpp
endif

#make sure all objects are located in build directory
OBJECTS := $(addprefix $(BUILD_DIR)/,$(SOURCES))
#also make sure objects have .o extension
OBJECTS := $(addsuffix .o,$(OBJECTS))

#include generated dependency files to allow incremental build when only headers change
-include $(OBJECTS:%.o=%.d)