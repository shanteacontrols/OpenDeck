ifeq (,$(findstring gen,$(TYPE)))
    INCLUDE_DIRS += \
    -I"board/arch/arm/common/CMSIS"

    SOURCES += modules/EmuEEPROM/src/EmuEEPROM.cpp

    #clang-specific defines
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
EMUEEPROM_INCLUDE_CONFIG

ifeq ($(CPU),cortex-m4)
    DEFINES += \
    CORE_ARM_M4
endif