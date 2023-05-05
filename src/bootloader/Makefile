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
-Xlinker --defsym=CORE_MCU_FLASH_START_ADDR_USER=$(PROJECT_MCU_FLASH_ADDR_BOOT_START) \
-Xlinker --defsym=CORE_MCU_FLASH_SIZE_USER=$(PROJECT_MCU_FLASH_BOOT_SIZE) \
-Wl,--print-memory-usage

# Optimization level
OPT := -Os

ifeq ($(DEBUG), 1)
    CPPFLAGS += -g -gdwarf-2
    LDFLAGS  += -g -gdwarf-2
    OPT      := -Og
endif

INCLUDE_DIRS += \
-I"application" \
-I"bootloader/"

ifneq (,$(findstring PROJECT_MCU_SUPPORT_BOOTLOADER,$(DEFINES)))
    SOURCES += $(shell find bootloader -type f -name "*.cpp")
    DEFINES += FW_BOOT
else
    SOURCES :=
endif