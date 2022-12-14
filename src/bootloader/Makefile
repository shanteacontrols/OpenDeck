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
-T $(LINKER_FILE) \
-Xlinker --defsym=CORE_MCU_FLASH_START_ADDR_USER=$(FLASH_START_ADDR) \
-Xlinker --defsym=CORE_MCU_FLASH_SIZE_USER=$(CORE_MCU_FLASH_SIZE_USER) \
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

ifneq (,$(findstring HW_SUPPORT_BOOTLOADER,$(DEFINES)))
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

    SOURCES += $(shell find bootloader -type f -name "*.cpp")

    DEFINES += FW_BOOT
    FLASH_START_ADDR := $(FLASH_ADDR_BOOT_START)
    CORE_MCU_FLASH_SIZE_USER := $(BOOT_SIZE)
else
    SOURCES :=
endif