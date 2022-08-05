PROJECT              := OpenDeck
BUILD_DIR_BASE       := ./build
TARGET               := discovery
DEF_FILE_TARGET      := ../config/target/$(TARGET).yml
DEF_FILE_TSCREEN     := ../config/touchscreen/$(TARGET).json
TYPE                 := app
SCRIPTS_DIR          := ../scripts
DEPS_DIR             := deps
CLANG_TIDY_OUT       := $(BUILD_DIR_BASE)/clang-tidy-fixes.yaml

ifeq (,$(wildcard $(DEF_FILE_TARGET)))
    $(error Target doesn't exist)
endif

ifeq ($(DEBUG),1)
    BUILD_TYPE := debug
else
    BUILD_TYPE := release
endif

# Verbose builds
ifeq ($(V),1)
    Q :=
else
    Q := @
endif

BUILD_DIR := $(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)

ifneq (,$(filter $(MAKECMDGOALS),concat))
    BUILD_DIR := $(BUILD_DIR)/merged
else
    BUILD_DIR := $(BUILD_DIR)/$(TYPE)
endif

OUTPUT           := $(BUILD_DIR)/$(TARGET)
BUILD_TIME_FILE  := $(BUILD_DIR_BASE)/lastbuild
LAST_BUILD_TIME  := $(shell cat $(BUILD_TIME_FILE) 2>/dev/null | awk '{print$$1}END{if(NR==0)print 0}')
FLASH_BINARY_DIR := $(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged

SYSEX_BINARY_SUFFIX := _sysex

SW_VERSION_MAJOR            := $(shell git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f1)
SW_VERSION_MINOR            := $(shell git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f2)
SW_VERSION_REVISION         := $(shell git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f3)

BOARD_GEN_DIR_BASE          := board/gen
BOARD_GEN_DIR_TARGET_BASE   := $(BOARD_GEN_DIR_BASE)/target
BOARD_GEN_DIR_MCU_BASE      := $(BOARD_GEN_DIR_BASE)/mcu
BOARD_GEN_DIR_VENDOR_BASE   := $(BOARD_GEN_DIR_BASE)/vendor
BOARD_GEN_DIR_ARCH_BASE     := $(BOARD_GEN_DIR_BASE)/arch
BOARD_GEN_DIR_TARGET        := $(BOARD_GEN_DIR_TARGET_BASE)/$(TARGET)
APP_GEN_DIR_BASE            := application/gen
APP_GEN_DIR_TARGET          := $(APP_GEN_DIR_BASE)/$(TARGET)

# The MAKEFILE_INCLUDE_PREFIX prefix is used to specify the start directory makefiles.
# This is needed for tests since they are outside of src/.
-include $(MAKEFILE_INCLUDE_PREFIX)$(BOARD_GEN_DIR_TARGET)/Makefile
-include $(MAKEFILE_INCLUDE_PREFIX)board/arch/$(CORE_MCU_ARCH)/Makefile
-include $(MAKEFILE_INCLUDE_PREFIX)board/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/Makefile
-include $(MAKEFILE_INCLUDE_PREFIX)board/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/$(CORE_MCU_FAMILY)/Makefile
-include $(MAKEFILE_INCLUDE_PREFIX)board/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/$(CORE_MCU_FAMILY)/$(CORE_MCU_MODEL)/Makefile

FW_METADATA_SIZE            := 4

# These can be overriden by target/vendor/arch/mcu etc.
BUFFER_SIZE_USB_OVER_SERIAL ?= 16
BUFFER_SIZE_I2C_TX          ?= 64
BUFFER_SIZE_UART_TX         ?= 128
BUFFER_SIZE_UART_RX         ?= 128
BUFFER_SIZE_BLE_MIDI_PACKET ?= 128
MIDI_SYSEX_ARRAY_SIZE       ?= 100
# tinyusb only
BUFFER_SIZE_USB_MIDI_TX     ?= 4096
BUFFER_SIZE_USB_MIDI_RX     ?= 4096
BUFFER_SIZE_USB_CDC_TX      ?= 2048
BUFFER_SIZE_USB_CDC_RX      ?= 2048

DEFINES += \
BUFFER_SIZE_USB_OVER_SERIAL=$(BUFFER_SIZE_USB_OVER_SERIAL) \
BUFFER_SIZE_TSCREEN_CDC_PASSTHROUGH=$(BUFFER_SIZE_USB_OVER_SERIAL) \
BUFFER_SIZE_I2C_TX=$(BUFFER_SIZE_I2C_TX) \
MIDI_SYSEX_ARRAY_SIZE=$(MIDI_SYSEX_ARRAY_SIZE) \
BUFFER_SIZE_BLE_MIDI_PACKET=$(BUFFER_SIZE_BLE_MIDI_PACKET) \
SW_VERSION_MAJOR=$(SW_VERSION_MAJOR) \
SW_VERSION_MINOR=$(SW_VERSION_MINOR) \
SW_VERSION_REVISION=$(SW_VERSION_REVISION) \
BUFFER_SIZE_USB_MIDI_TX=$(BUFFER_SIZE_USB_MIDI_TX) \
BUFFER_SIZE_USB_MIDI_RX=$(BUFFER_SIZE_USB_MIDI_RX) \
BUFFER_SIZE_USB_CDC_TX=$(BUFFER_SIZE_USB_CDC_TX) \
BUFFER_SIZE_USB_CDC_RX=$(BUFFER_SIZE_USB_CDC_RX) \
BUFFER_SIZE_UART_TX=$(BUFFER_SIZE_UART_TX) \
BUFFER_SIZE_UART_RX=$(BUFFER_SIZE_UART_RX) \
CORE_USE_C_TIMER_CALLBACK

ifeq ($(DEBUG), 1)
    DEFINES += DEBUG
endif

ifeq ($(LOG), 1)
    DEFINES += USE_LOGGER
endif

ifneq (,$(findstring USE_TINYUSB,$(DEFINES)))
    DEFINES += \
    BOARD_USE_UPDATE_HOOKS
endif

ifeq ($(TYPE),boot)
    DEFINES += FW_BOOT
    FLASH_START_ADDR := $(FLASH_ADDR_BOOT_START)
else ifeq ($(TYPE),app)
    DEFINES += FW_APP
    FLASH_START_ADDR := $(FLASH_ADDR_APP_START)
else ifeq ($(TYPE),flashgen)
    # Same as app
    DEFINES += FW_APP
    FLASH_START_ADDR := $(FLASH_ADDR_APP_START)
    DEFINES := $(filter-out CORE_ARCH_%,$(DEFINES))
    DEFINES := $(filter-out CORE_VENDOR_%,$(DEFINES))
else ifeq ($(TYPE),sysexgen)
    DEFINES := $(filter-out CORE_ARCH_%,$(DEFINES))
    DEFINES := $(filter-out CORE_VENDOR_%,$(DEFINES))
else
    $(error Invalid firmware type specified)
endif

DEFINES += CORE_MCU_FLASH_START_ADDR_USER=$(FLASH_START_ADDR)
DEFINES += FLASH_ADDR_BOOT_START=$(FLASH_ADDR_BOOT_START)
DEFINES += FLASH_ADDR_APP_START=$(FLASH_ADDR_APP_START)
DEFINES += FLASH_ADDR_FW_METADATA=$(FLASH_ADDR_FW_METADATA)

# When set to 1, format target will fail if there are any changes to the repository after formatting.
CF_FAIL_ON_DIFF := 0

# When set to 1, lint target will fail if there are any changes to the repository after linting.
CL_FAIL_ON_DIFF := 0