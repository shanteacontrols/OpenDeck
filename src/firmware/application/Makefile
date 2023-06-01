# Passed to both C and C++ compiler
CPPFLAGS += \
-Wall \
-fdata-sections \
-ffunction-sections \
-fmessage-length=0 \
-fno-strict-aliasing \
-fshort-enums \
-Wno-unused-function

# C++ compiler only
CXXFLAGS += \
-std=c++17 \
-fno-rtti \
-fno-exceptions \
-fpermissive \
-fno-threadsafe-statics

# C compiler only
CFLAGS += \
-std=gnu11 \
-Wno-parentheses-equality \
-Wno-unused-value \
-Wno-implicit-function-declaration

# Assembler only
ASFLAGS += \
-x assembler-with-cpp

# Linker flags
LDFLAGS += \
-Wl,--gc-sections \
-Wl,-Map="$(OUTPUT).map",--cref \
-Xlinker --defsym=CORE_MCU_FLASH_START_ADDR_USER=$(PROJECT_MCU_FLASH_ADDR_APP_START) \
-Xlinker --defsym=CORE_MCU_FLASH_SIZE_USER=$(PROJECT_MCU_FLASH_APP_SIZE) \
-Wl,--print-memory-usage

# Optimization level
OPT := -Os

ifeq ($(DEBUG), 1)
    CPPFLAGS += -g -gdwarf-2
    LDFLAGS  += -g -gdwarf-2
    OPT      := -Og
endif

ifneq (,$(findstring PROJECT_TARGET_USB_OVER_SERIAL_HOST,$(DEFINES)))
    # FW for USB hosts uses different set of sources than other targets
    SOURCES += $(shell find firmware/usb-link -type f -name "*.cpp")
else
    SOURCES += $(shell $(FIND) firmware/application -type f -not -path "*$(GENERATED_SUBDIR_BASE)*" -regex '.*\.\(s\|c\|cpp\)')
    SOURCES += $(shell $(FIND) $(APPLICATION_GEN_DIR_TARGET) -type f -regex '.*\.\(s\|c\|cpp\)')
    SOURCES += $(shell $(FIND) ../modules/dbms/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) ../modules/midi/src -type f -name "*.cpp" | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) ../modules/sysex/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
    ifneq (,$(findstring PROJECT_TARGET_SUPPORT_DISPLAY,$(DEFINES)))
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

DEFINES += FW_APP