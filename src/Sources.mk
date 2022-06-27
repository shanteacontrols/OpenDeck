vpath modules/%.c ../
vpath modules/%.cpp ../
vpath modules/%.s ../
vpath modules/%.S ../

# Common include dirs
INCLUDE_DIRS += \
-I"./" \
-I"$(BOARD_GEN_DIR_MCU_BASE)/$(CORE_MCU_MODEL)" \
-I"$(BOARD_GEN_DIR_TARGET)/" \
-I"application/" \
-I"board/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/common" \
-I"board/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/$(CORE_MCU_FAMILY)/common" \
-I"../modules/core/src/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/common" \
-I"../modules/core/src/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/$(CORE_MCU_FAMILY)/common" \
-I"board/common" \
-I"bootloader/" \
-I"../modules/"

ifneq (,$(findstring USE_TINYUSB,$(DEFINES)))
    INCLUDE_DIRS += \
    -I"board/common/communication/usb/tinyusb" \
    -I"../modules/core/modules/tinyusb/hw" \
    -I"../modules/core/modules/tinyusb/src"
endif

LINKER_FILE       := ../modules/core/src/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/$(CORE_MCU_FAMILY)/$(CORE_MCU_MODEL)/$(CORE_MCU_MODEL).ld
TARGET_GEN_HEADER := $(BOARD_GEN_DIR_TARGET)/Target.h

ifneq (,$(wildcard $(DEF_FILE_TSCREEN)))
    TSCREEN_GEN_SOURCE += $(APP_GEN_DIR_TARGET)/Touchscreen.cpp
endif

GEN_FILES += \
$(TARGET_GEN_HEADER) \
$(TSCREEN_GEN_SOURCE)

ifeq (,$(findstring gen,$(TYPE)))
    SOURCES += $(shell $(FIND) board/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/common/ -type f -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) board/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/$(CORE_MCU_FAMILY)/common -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) board/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/$(CORE_MCU_FAMILY)/$(CORE_MCU_MODEL) -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) board/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/common -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) board/arch/$(CORE_MCU_ARCH)/common -type f -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) board/common -type f -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")

    SOURCES += $(shell $(FIND) ../modules/core/src/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/common/ -type f -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) ../modules/core/src/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/$(CORE_MCU_FAMILY)/common -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) ../modules/core/src/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/$(CORE_MCU_FAMILY)/$(CORE_MCU_MODEL) -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) ../modules/core/src/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/common -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) ../modules/core/src/arch/$(CORE_MCU_ARCH)/common -type f -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) ../modules/core/src/arch/common -type f -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")

    ifeq ($(TYPE),boot)
        SOURCES += $(shell find bootloader -type f -name "*.cpp")
    else ifeq ($(TYPE),app)
        ifneq (,$(findstring USB_OVER_SERIAL_HOST,$(DEFINES)))
            # FW for USB hosts uses different set of sources than other targets
            SOURCES += $(shell find usb-link -type f -name "*.cpp")
        else
            SOURCES += $(shell $(FIND) application -type f -not -path "*$(APP_GEN_DIR_BASE)*" -regex '.*\.\(s\|c\|cpp\)')
            SOURCES += $(shell $(FIND) $(APP_GEN_DIR_TARGET) -type f -regex '.*\.\(s\|c\|cpp\)')
            SOURCES += $(shell $(FIND) ../modules/dbms/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
            SOURCES += $(shell $(FIND) ../modules/dmxusb/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
            SOURCES += $(shell $(FIND) ../modules/midi/src -type f -name "*.cpp" | sed "s|^\.\./||")
            SOURCES += $(shell $(FIND) ../modules/sysex/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")

            ifneq (,$(findstring DISPLAY_SUPPORTED,$(DEFINES)))
                # u8x8 sources
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
    ifeq ($(CORE_MCU_ARCH),arm)
        SOURCES += $(shell $(FIND) application/database -type f -name "*.cpp")
        SOURCES += $(shell $(FIND) ../modules/dbms/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
        SOURCES += $(TSCREEN_GEN_SOURCE)
        SOURCES += application/util/configurable/Configurable.cpp
        SOURCES += flashgen/main.cpp
        SOURCES += modules/EmuEEPROM/src/EmuEEPROM.cpp
    endif
else ifeq ($(TYPE),sysexgen)
    SOURCES += sysexgen/main.cpp
endif

# Make sure all objects are located in build directory
OBJECTS := $(addprefix $(BUILD_DIR)/,$(SOURCES))
# Also make sure objects have .o extension
OBJECTS := $(addsuffix .o,$(OBJECTS))

# Include generated dependency files to allow incremental build when only headers change
-include $(OBJECTS:%.o=%.d)