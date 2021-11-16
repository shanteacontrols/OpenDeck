vpath modules/%.cpp ../
vpath modules/%.c ../

#common include dirs
INCLUDE_DIRS := \
-I"../modules/" \
-I"$(BOARD_MCU_BASE_DIR)/$(MCU)" \
-I"$(BOARD_TARGET_DIR)/" \
-I"application/" \
-I"board/arch/$(ARCH)/variants/$(MCU_FAMILY)" \
-I"board/arch/$(ARCH)" \
-I"board/common" \
-I"./"

ifeq (,$(findstring gen,$(TYPE)))
    ifeq ($(ARCH), avr)
        INCLUDE_DIRS += \
        -I"../modules/lufa/" \
        -I"../modules/avr-libstdcpp/include"
    else ifeq ($(ARCH),stm32)
        INCLUDE_DIRS += \
        $(addprefix -I,$(shell $(FIND) ./board/arch/$(ARCH)/gen/common -type d -not -path "*Src*")) \
        $(addprefix -I,$(shell $(FIND) ./board/arch/$(ARCH)/gen/$(MCU_FAMILY)/common -type d -not -path "*Src*")) \
        $(addprefix -I,$(shell $(FIND) ./board/arch/$(ARCH)/gen/$(MCU_FAMILY)/$(MCU_BASE)/Drivers -type d -not -path "*Src*"))
    endif

    ifeq ($(TYPE),boot)
        INCLUDE_DIRS += \
        -I"bootloader/"
    endif
endif

LINKER_FILE := $(MCU_DIR)/$(MCU).ld

TARGET_GEN_SOURCE := $(BOARD_TARGET_DIR)/$(TARGET).cpp

ifneq (,$(wildcard $(TOUCHSCREEN_DEF_FILE)))
    TSCREEN_GEN_SOURCE += $(TOUCHSCREEN_GEN_BASE_DIR)/$(TARGET).cpp
endif

ifeq (,$(findstring gen,$(TYPE)))
    SOURCES += $(TARGET_GEN_SOURCE)
    SOURCES += $(TSCREEN_GEN_SOURCE)

    #architecture specific
    ifeq ($(ARCH), avr)
        ifneq (,$(findstring USB_SUPPORTED,$(DEFINES)))
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
            modules/lufa/LUFA/Drivers/USB/Class/Device/MIDIClassDevice.c \
            modules/lufa/LUFA/Drivers/USB/Class/Device/CDCClassDevice.c
        endif
    else ifeq ($(ARCH),stm32)
        SOURCES += $(shell $(FIND) ./board/arch/$(ARCH)/gen/common -regex '.*\.\(s\|c\)')
        SOURCES += $(shell $(FIND) ./board/arch/$(ARCH)/gen/$(MCU_FAMILY)/common -regex '.*\.\(s\|c\)')
        SOURCES += $(shell $(FIND) ./board/arch/$(ARCH)/gen/$(MCU_FAMILY)/$(MCU_BASE) -regex '.*\.\(s\|c\)')
        SOURCES += $(shell $(FIND) ./board/arch/$(ARCH)/variants/$(MCU_FAMILY) -maxdepth 1 -name "*.cpp")
        SOURCES += modules/EmuEEPROM/src/EmuEEPROM.cpp
    endif

    SOURCES += $(shell $(FIND) ./board/common -maxdepth 1 -type f -name "*.cpp")
    SOURCES += $(shell $(FIND) ./$(MCU_DIR) -maxdepth 1 -type f -regex '.*\.\(s\|c\|cpp\)')
    SOURCES += board/common/io/Stubs.cpp

    ifeq ($(TYPE),boot)
        #bootloader sources
        #common
        SOURCES += \
        board/common/bootloader/Bootloader.cpp \
        board/common/io/Indicators.cpp \
        board/arch/$(ARCH)/common/Bootloader.cpp \
        board/arch/$(ARCH)/common/Init.cpp \
        board/arch/$(ARCH)/common/ShiftRegistersWait.cpp \
        board/arch/$(ARCH)/common/ISR.cpp

        ifeq ($(ARCH),avr)
            SOURCES += board/arch/$(ARCH)/common/Flash.cpp
        endif

        SOURCES += $(shell find ./bootloader -type f -name "*.cpp")

        ifneq (,$(findstring USB_SUPPORTED,$(DEFINES)))
            SOURCES += $(shell $(FIND) ./board/common/comm/usb/descriptors/midi -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./board/common/comm/usb/descriptors/midi -type f -name "*.c")
            SOURCES += $(shell $(FIND) ./board/arch/$(ARCH)/comm/usb/midi -type f -name "*.cpp")

            ifneq (,$(findstring USB_LINK_MCU,$(DEFINES)))
                #for USB link MCUs, compile UART as well - needed to communicate with main MCU
                SOURCES += \
                board/arch/$(ARCH)/comm/uart/UART.cpp \
                board/common/comm/uart/UART.cpp
            endif
        else
            SOURCES += \
            board/arch/$(ARCH)/comm/uart/UART.cpp \
            board/common/comm/uart/UART.cpp

            SOURCES += $(shell $(FIND) ./board/common/comm/USBOverSerial -type f -name "*.cpp")
        endif
    else ifeq ($(TYPE),app)
        #application sources
        #common for all targets
        SOURCES += $(shell $(FIND) ./board/arch/$(ARCH)/common -type f -name "*.cpp")
        SOURCES += $(shell $(FIND) ./board/common/comm/USBOverSerial -type f -name "*.cpp")

        ifneq (,$(findstring USB_SUPPORTED,$(DEFINES)))
            SOURCES += $(shell $(FIND) ./board/arch/$(ARCH)/comm/usb/midi_cdc_dual -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./board/common/comm/usb/descriptors/midi_cdc_dual -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./board/common/comm/usb/descriptors/midi_cdc_dual -type f -name "*.c")
        endif

        ifneq (,$(findstring USE_UART,$(DEFINES)))
            SOURCES += \
            board/arch/$(ARCH)/comm/uart/UART.cpp \
            board/common/comm/uart/UART.cpp
        endif

        ifneq (,$(findstring USB_LINK_MCU,$(DEFINES)))
            #fw for usb links uses different set of sources than other targets
            SOURCES += \
            board/common/io/Indicators.cpp \
            usb-link/main.cpp
        else
            SOURCES += $(shell $(FIND) ./application -maxdepth 1 -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./application/database -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./application/system -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./application/protocol -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./application/util -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./application/io/common -maxdepth 1 -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ../modules/sysex/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
            SOURCES += $(shell $(FIND) ../modules/midi/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
            SOURCES += $(shell $(FIND) ../modules/dbms/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
            SOURCES += $(shell $(FIND) ../modules/dmxusb/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
            SOURCES += $(shell $(FIND) ./application/io/analog -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./application/io/buttons -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./application/io/encoders -maxdepth 1 -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./application/io/leds -maxdepth 1 -type f -name "*.cpp")

            ifneq (,$(findstring ADC_SUPPORTED,$(DEFINES)))
                SOURCES += board/common/io/Analog.cpp
            endif

            ifneq (,$(findstring DIGITAL_INPUTS_SUPPORTED,$(DEFINES)))
                SOURCES += board/common/io/Input.cpp
            endif

            ifneq (,$(findstring DIGITAL_OUTPUTS_SUPPORTED,$(DEFINES)))
                SOURCES += board/common/io/Output.cpp
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
                SOURCES += $(shell $(FIND) ./application/io/touchscreen/model -type f -name "*.cpp")
            endif

            ifneq (,$(findstring DISPLAY_SUPPORTED,$(DEFINES)))
                SOURCES += $(shell $(FIND) ./application/io/display -type f -name "*.cpp")
                SOURCES += $(shell $(FIND) ./board/arch/$(ARCH)/comm/i2c -type f -name "*.cpp")

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
    endif
else ifeq ($(TYPE),flashgen)
    ifeq ($(ARCH),stm32)
        SOURCES += $(shell $(FIND) ./application/database -type f -name "*.cpp")
        SOURCES += $(shell $(FIND) ../modules/dbms/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
        SOURCES += modules/EmuEEPROM/src/EmuEEPROM.cpp
        SOURCES += $(TSCREEN_GEN_SOURCE)
        SOURCES += flashgen/main.cpp
        SOURCES += application/util/configurable/Configurable.cpp
        SOURCES += application/util/conversion/Conversion.cpp

        INCLUDE_DIRS += -I"flashgen/"
    endif
else ifeq ($(TYPE),sysexgen)
    SOURCES += sysexgen/main.cpp
endif

#make sure all objects are located in build directory
OBJECTS := $(addprefix $(BUILD_DIR)/,$(SOURCES))
#also make sure objects have .o extension
OBJECTS := $(addsuffix .o,$(OBJECTS))

#include generated dependency files to allow incremental build when only headers change
-include $(OBJECTS:%.o=%.d)