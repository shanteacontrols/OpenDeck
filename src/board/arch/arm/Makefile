ifeq (,$(findstring gen,$(TYPE)))
    INCLUDE_DIRS += \
    -I"../modules/core/src/arch/arm/common/CMSIS" \
    -I"../modules/core/modules/tinyusb/lib/CMSIS_5/CMSIS/Core/Include"

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
endif

DEFINES += \
CORE_ARCH_ARM \
EMUEEPROM_INCLUDE_CONFIG \
USE_FAST_SOFT_PWM_TIMER

ifeq ($(CPU),cortex-m4)
    DEFINES += \
    CORE_ARM_M4
endif

CC        := arm-none-eabi-gcc
CXX       := arm-none-eabi-g++
LD        := arm-none-eabi-g++
AS        := $(CC)
SIZE_TOOL := size

REQ_PACKAGES := \
$(CC) \
$(CXX) \
$(LD) \
$(AS) \
$(SIZE_TOOL)

$(call CHECK_PACKAGES,$(REQ_PACKAGES))

CPPFLAGS += \
-mcpu=$(CPU) \
-mthumb \
-mno-unaligned-access

ifneq (,$(findstring clang,$(CC_ARM)))
    CPPFLAGS += -target arm-none-eabi
endif

LDFLAGS += \
-mcpu=$(CPU) \
-mthumb \
-specs=nano.specs \
-specs=nosys.specs

LDFLAGS += \
-Xlinker --defsym=CORE_FLASH_START_ADDR=$(FLASH_START_ADDR)

ifeq ($(DEBUG), 1)
    CPPFLAGS += -g -gdwarf-2
    LDFLAGS  += -g -gdwarf-2
    OPT      := -Og
endif

ifneq ($(FPU),)
    CPPFLAGS += -mfpu=$(FPU)
    LDFLAGS  += -mfpu=$(FPU)
endif

ifneq ($(FLOAT-ABI),)
    CPPFLAGS += -mfloat-abi=$(FLOAT-ABI)
    LDFLAGS  += -mfloat-abi=$(FLOAT-ABI)
endif