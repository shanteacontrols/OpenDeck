vpath modules/%.c ../
vpath modules/%.cpp ../
vpath modules/%.s ../
vpath modules/%.S ../

#common include dirs
INCLUDE_DIRS += \
-I"./" \
-I"$(BOARD_GEN_DIR_MCU_BASE)/$(MCU)" \
-I"$(BOARD_GEN_DIR_TARGET)/" \
-I"application/" \
-I"board/arch/$(ARCH)/$(VENDOR)/common" \
-I"board/arch/$(ARCH)/$(VENDOR)/variants/$(MCU_FAMILY)/common" \
-I"board/common" \
-I"bootloader/" \
-I"../modules/"

ifneq (,$(findstring USE_TINYUSB,$(DEFINES)))
    INCLUDE_DIRS += -I"board/common/communication/usb/tinyusb"
endif

LINKER_FILE       := ../modules/core/src/arch/$(ARCH)/$(VENDOR)/variants/$(MCU_FAMILY)/$(MCU)/$(MCU).ld
TARGET_GEN_HEADER := $(BOARD_GEN_DIR_TARGET)/Target.h

ifneq (,$(wildcard $(DEF_FILE_TSCREEN)))
    TSCREEN_GEN_SOURCE += $(APP_GEN_DIR_TARGET)/Touchscreen.cpp
endif

GEN_FILES += \
$(TARGET_GEN_HEADER) \
$(TSCREEN_GEN_SOURCE)

ifeq (,$(findstring gen,$(TYPE)))
    SOURCES += $(shell $(FIND) board/arch/$(ARCH)/$(VENDOR)/common/ -type f -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) board/arch/$(ARCH)/$(VENDOR)/variants/$(MCU_FAMILY)/common -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) board/arch/$(ARCH)/$(VENDOR)/variants/$(MCU_FAMILY)/$(MCU) -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) board/arch/$(ARCH)/$(VENDOR)/variants/common -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) board/arch/$(ARCH)/common -type f -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) board/common -type f -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")

    SOURCES += $(shell $(FIND) ../modules/core/src/arch/$(ARCH)/$(VENDOR)/common/ -type f -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) ../modules/core/src/arch/$(ARCH)/$(VENDOR)/variants/$(MCU_FAMILY)/common -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) ../modules/core/src/arch/$(ARCH)/$(VENDOR)/variants/$(MCU_FAMILY)/$(MCU) -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) ../modules/core/src/arch/$(ARCH)/$(VENDOR)/variants/common -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) ../modules/core/src/arch/$(ARCH)/common -type f -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) ../modules/core/src/arch/common -type f -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")

    ifeq ($(TYPE),boot)
        SOURCES += $(shell find bootloader -type f -name "*.cpp")
    else ifeq ($(TYPE),app)
        ifneq (,$(findstring USB_OVER_SERIAL_HOST,$(DEFINES)))
            #fw for usb hosts uses different set of sources than other targets
            SOURCES += $(shell find usb-link -type f -name "*.cpp")
        else
            SOURCES += $(shell $(FIND) application -type f -not -path "*$(APP_GEN_DIR_BASE)*" -regex '.*\.\(s\|c\|cpp\)')
            SOURCES += $(shell $(FIND) $(APP_GEN_DIR_TARGET) -type f -regex '.*\.\(s\|c\|cpp\)')
            SOURCES += $(shell $(FIND) ../modules/dbms/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
            SOURCES += $(shell $(FIND) ../modules/dmxusb/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
            SOURCES += $(shell $(FIND) ../modules/midi/src -type f -name "*.cpp" | sed "s|^\.\./||")
            SOURCES += $(shell $(FIND) ../modules/sysex/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")

            ifneq (,$(findstring I2C_SUPPORTED,$(DEFINES)))
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
        SOURCES += $(shell $(FIND) application/database -type f -name "*.cpp")
        SOURCES += $(shell $(FIND) ../modules/dbms/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
        SOURCES += $(TSCREEN_GEN_SOURCE)
        SOURCES += application/util/configurable/Configurable.cpp
        SOURCES += application/util/conversion/Conversion.cpp
        SOURCES += flashgen/main.cpp
        SOURCES += modules/EmuEEPROM/src/EmuEEPROM.cpp

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