vpath modules/%.cpp ../
vpath modules/%.c ../
vpath modules/%.s ../
vpath modules/%.S ../

#common include dirs
INCLUDE_DIRS += \
-I"../modules/" \
-I"$(GEN_DIR_MCU_BASE)/$(MCU)" \
-I"$(GEN_DIR_TARGET)/" \
-I"application/" \
-I"bootloader/" \
-I"board/arch/$(ARCH)/$(VENDOR)/variants/$(MCU_FAMILY)" \
-I"board/arch/$(ARCH)/$(VENDOR)" \
-I"board/common" \
-I"./"

ifneq (,$(findstring USE_TINYUSB,$(DEFINES)))
    INCLUDE_DIRS += -I"board/common/comm/usb/tinyusb"
endif

LINKER_FILE       := $(MCU_DIR)/$(MCU).ld
TARGET_GEN_HEADER := $(GEN_DIR_TARGET)/Target.h

ifneq (,$(wildcard $(DEF_FILE_TSCREEN)))
    TSCREEN_GEN_SOURCE += $(GEN_DIR_TSCREEN_BASE)/$(TARGET).cpp
endif

PRE_BUILD_FILES += \
$(TARGET_GEN_HEADER) \
$(TSCREEN_GEN_SOURCE)

ifeq (,$(findstring gen,$(TYPE)))
    SOURCES += $(TSCREEN_GEN_SOURCE)

    SOURCES += $(shell $(FIND) ./board/common -maxdepth 1 -type f -name "*.cpp")
    SOURCES += $(shell $(FIND) ./board/arch/$(ARCH)/common/ -type f -name "*.cpp")
    SOURCES += $(shell $(FIND) ./board/arch/$(ARCH)/$(VENDOR)/common/ -type f -name "*.cpp")
    SOURCES += $(shell $(FIND) ./$(MCU_DIR) -maxdepth 1 -type f -regex '.*\.\(s\|c\|cpp\)')
    SOURCES += board/common/Stubs.cpp

    ifeq ($(TYPE),boot)
        #bootloader sources
        #common
        SOURCES += board/common/io/Indicators.cpp
        SOURCES += $(shell find ./bootloader -type f -name "*.cpp")

        ifneq (,$(findstring USB_SUPPORTED,$(DEFINES)))
            SOURCES += $(shell $(FIND) ./board/common/comm/usb/descriptors/midi -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./board/common/comm/usb/descriptors/midi -type f -name "*.c")
            SOURCES += $(shell $(FIND) ./board/arch/$(ARCH)/$(VENDOR)/comm/usb/midi -type f -name "*.cpp")

            ifneq (,$(findstring USE_TINYUSB,$(DEFINES)))
                SOURCES += board/common/comm/usb/tinyusb/Common.cpp
                SOURCES += board/common/comm/usb/tinyusb/MIDI.cpp
            endif

            ifneq (,$(findstring USB_LINK_MCU,$(DEFINES)))
                #for USB link MCUs, compile UART as well - needed to communicate with main MCU
                SOURCES += \
                board/arch/$(ARCH)/$(VENDOR)/comm/uart/UART.cpp \
                board/common/comm/uart/UART.cpp
            endif
        else
            SOURCES += \
            board/arch/$(ARCH)/$(VENDOR)/comm/uart/UART.cpp \
            board/common/comm/uart/UART.cpp

            SOURCES += $(shell $(FIND) ./board/common/comm/USBOverSerial -type f -name "*.cpp")
        endif
    else ifeq ($(TYPE),app)
        #application sources
        #common for all targets
        SOURCES += $(shell $(FIND) ./board/common/comm/USBOverSerial -type f -name "*.cpp")

        ifneq (,$(findstring USB_SUPPORTED,$(DEFINES)))
            SOURCES += $(shell $(FIND) ./board/arch/$(ARCH)/$(VENDOR)/comm/usb/midi_cdc_dual -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./board/common/comm/usb/descriptors/midi_cdc_dual -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./board/common/comm/usb/descriptors/midi_cdc_dual -type f -name "*.c")

            ifneq (,$(findstring USE_TINYUSB,$(DEFINES)))
                SOURCES += board/common/comm/usb/tinyusb/Common.cpp
                SOURCES += board/common/comm/usb/tinyusb/CDC.cpp
                SOURCES += board/common/comm/usb/tinyusb/MIDI.cpp
            endif
        endif

        ifneq (,$(findstring UART_SUPPORTED,$(DEFINES)))
            SOURCES += \
            board/arch/$(ARCH)/$(VENDOR)/comm/uart/UART.cpp \
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
            SOURCES += $(shell $(FIND) ../modules/midi/src -type f -name "*.cpp" | sed "s|^\.\./||")
            SOURCES += $(shell $(FIND) ../modules/dbms/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
            SOURCES += $(shell $(FIND) ../modules/dmxusb/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
            SOURCES += $(shell $(FIND) ./application/io/analog -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./application/io/buttons -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./application/io/encoders -maxdepth 1 -type f -name "*.cpp")
            SOURCES += $(shell $(FIND) ./application/io/leds -maxdepth 1 -type f -name "*.cpp")

            ifneq (,$(findstring USE_LOGGER,$(DEFINES)))
                SOURCES += $(shell $(FIND) ./application/logger -maxdepth 1 -type f -name "*.cpp")
            endif

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

            ifneq (,$(findstring I2C_SUPPORTED,$(DEFINES)))
                SOURCES += $(shell $(FIND) ./application/io/i2c -type f -name "*.cpp")
                SOURCES += $(shell $(FIND) ./board/arch/$(ARCH)/$(VENDOR)/comm/i2c -type f -name "*.cpp")

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
    ifeq ($(ARCH),arm)
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