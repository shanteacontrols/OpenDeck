ifeq (,$(findstring gen,$(TYPE)))
    SOURCES += modules/EmuEEPROM/src/EmuEEPROM.cpp

    # clang-specific defines
    ifneq (,$(findstring clang,$(CC_ARM)))
        TOOLCHAIN_DIR := $(shell dirname $(shell which arm-none-eabi-gcc) | rev | cut -c5- | rev)
        CPP_VER := $(shell $(FIND) $(TOOLCHAIN_DIR)/arm-none-eabi/include/c++ -mindepth 1 -maxdepth 1 | rev | cut -d/ -f 1 | rev)

        INCLUDE_DIRS += \
        -isystem"$(TOOLCHAIN_DIR)/lib/gcc/arm-none-eabi/$(CPP_VER)/include" \
        -isystem"$(TOOLCHAIN_DIR)/lib/gcc/arm-none-eabi/$(CPP_VER)/include-fixed" \
        -isystem"$(TOOLCHAIN_DIR)/arm-none-eabi/include/c++/$(CPP_VER)" \
        -isystem"$(TOOLCHAIN_DIR)/arm-none-eabi/include/c++/$(CPP_VER)/arm-none-eabi" \
        -isystem"$(TOOLCHAIN_DIR)/arm-none-eabi/include/c++/$(CPP_VER)/backward" \
        -isystem"$(TOOLCHAIN_DIR)/arm-none-eabi/include"
    endif

    CPPFLAGS += \
    -mthumb \
    -mno-unaligned-access

    ifneq (,$(findstring clang,$(CC_ARM)))
        CPPFLAGS += -target arm-none-eabi
    endif
endif

DEFINES += \
BOARD_USE_UPDATE_HOOKS \
BOARD_USE_TINYUSB \
EMU_EEPROM_INCLUDE_CONFIG \
BOARD_USE_FAST_SOFT_PWM_TIMER \
CORE_MCU_BTLDR_CACHE_WORDS_USER=8192 \
CFG_TUD_MSC=0 \
CFG_TUD_HID=0 \
CFG_TUD_MIDI=1 \
CFG_TUD_CDC=1 \
CFG_TUD_VENDOR=0

EMU_EEPROM_INCLUDE_CONFIG_GEN := $(MCU_GEN_DIR)/EmuEEPROMConfig.h

pre-build:: $(EMU_EEPROM_INCLUDE_CONFIG_GEN)

$(EMU_EEPROM_INCLUDE_CONFIG_GEN):
	@echo "#pragma once" > $(EMU_EEPROM_INCLUDE_CONFIG_GEN)
	@echo "#define EMU_EEPROM_PAGE_SIZE $(PROJECT_MCU_EMU_EEPROM_PAGE_SIZE)" >> $(EMU_EEPROM_INCLUDE_CONFIG_GEN)

CC        := arm-none-eabi-gcc
CXX       := arm-none-eabi-g++
LD        := arm-none-eabi-g++
AS        := $(CC)
SIZE_TOOL := 

REQ_PACKAGES := \
$(CC) \
$(CXX) \
$(LD) \
$(AS) \
$(SIZE_TOOL)

$(call CHECK_PACKAGES,$(REQ_PACKAGES))