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
    -mcpu=$(CORE_MCU_CPU) \
    -mthumb \
    -mno-unaligned-access

    ifneq (,$(findstring clang,$(CC_ARM)))
        CPPFLAGS += -target arm-none-eabi
    endif

    LDFLAGS += \
    -mcpu=$(CORE_MCU_CPU) \
    -mthumb \
    -specs=nano.specs \
    -specs=nosys.specs

    LDFLAGS += \
    -Xlinker --defsym=CORE_MCU_FLASH_START_ADDR_USER=$(FLASH_START_ADDR)

    # bootloader fw is always set to 32k
    BOOT_SIZE := 32768

    ifeq ($(TYPE),boot)
        CORE_MCU_FLASH_SIZE_USER=$(BOOT_SIZE)
    else
        CORE_MCU_FLASH_SIZE_USER=$(shell echo $$(( $(CORE_MCU_FLASH_SIZE) - $(BOOT_SIZE) - $(EMU_EEPROM_FLASH_USAGE) )) )
    endif

    DEFINES += CORE_MCU_FLASH_SIZE_USER=$(CORE_MCU_FLASH_SIZE_USER)
    LDFLAGS += -Xlinker --defsym=CORE_MCU_FLASH_SIZE_USER=$(CORE_MCU_FLASH_SIZE_USER)

    ifeq ($(DEBUG), 1)
        CPPFLAGS += -g -gdwarf-2
        LDFLAGS  += -g -gdwarf-2
        OPT      := -Og
    endif

    ifneq ($(CORE_MCU_FPU),)
        CPPFLAGS += -mfpu=$(CORE_MCU_FPU)
        LDFLAGS  += -mfpu=$(CORE_MCU_FPU)
    endif

    ifneq ($(CORE_MCU_FLOAT_ABI),)
        CPPFLAGS += -mfloat-abi=$(CORE_MCU_FLOAT_ABI)
        LDFLAGS  += -mfloat-abi=$(CORE_MCU_FLOAT_ABI)
    endif
endif

DEFINES += \
EMUEEPROM_INCLUDE_CONFIG \
USE_FAST_SOFT_PWM_TIMER

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