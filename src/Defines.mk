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
USB_OVER_SERIAL_BUFFER_SIZE ?= 16
I2C_TX_BUFFER_SIZE          ?= 64
UART_TX_BUFFER_SIZE         ?= 128
UART_RX_BUFFER_SIZE         ?= 128
MIDI_SYSEX_ARRAY_SIZE       ?= 100
MIDI_BLE_MAX_PACKET_SIZE    ?= 128
# tinyusb only
USB_MIDI_TX_BUFFER_SIZE     ?= 4096
USB_MIDI_RX_BUFFER_SIZE     ?= 4096
USB_CDC_TX_BUFFER_SIZE      ?= 2048
USB_CDC_RX_BUFFER_SIZE      ?= 2048

DEFINES += \
USB_OVER_SERIAL_BUFFER_SIZE=$(USB_OVER_SERIAL_BUFFER_SIZE) \
TSCREEN_CDC_PASSTHROUGH_BUFFER_SIZE=$(USB_OVER_SERIAL_BUFFER_SIZE) \
I2C_TX_BUFFER_SIZE=$(I2C_TX_BUFFER_SIZE) \
MIDI_SYSEX_ARRAY_SIZE=$(MIDI_SYSEX_ARRAY_SIZE) \
MIDI_BLE_MAX_PACKET_SIZE=$(MIDI_BLE_MAX_PACKET_SIZE) \
SW_VERSION_MAJOR=$(SW_VERSION_MAJOR) \
SW_VERSION_MINOR=$(SW_VERSION_MINOR) \
SW_VERSION_REVISION=$(SW_VERSION_REVISION) \
USB_MIDI_TX_BUFFER_SIZE=$(USB_MIDI_TX_BUFFER_SIZE) \
USB_MIDI_RX_BUFFER_SIZE=$(USB_MIDI_RX_BUFFER_SIZE) \
USB_CDC_TX_BUFFER_SIZE=$(USB_CDC_TX_BUFFER_SIZE) \
USB_CDC_RX_BUFFER_SIZE=$(USB_CDC_RX_BUFFER_SIZE) \
UART_TX_BUFFER_SIZE=$(UART_TX_BUFFER_SIZE) \
UART_RX_BUFFER_SIZE=$(UART_RX_BUFFER_SIZE) \
CORE_USE_C_TIMER_CALLBACK

ifeq ($(DEBUG), 1)
    DEFINES += DEBUG
endif

ifeq ($(LOG), 1)
    DEFINES += USE_LOGGER
endif

ifeq ($(TYPE),boot)
    DEFINES += FW_BOOT
    FLASH_START_ADDR := $(BOOT_START_ADDR)
else ifeq ($(TYPE),app)
    DEFINES += FW_APP
    FLASH_START_ADDR := $(APP_START_ADDR)
else ifeq ($(TYPE),flashgen)
    # Same as app
    DEFINES += FW_APP
    FLASH_START_ADDR := $(APP_START_ADDR)
    DEFINES := $(filter-out CORE_ARCH_%,$(DEFINES))
    DEFINES := $(filter-out CORE_VENDOR_%,$(DEFINES))
else ifeq ($(TYPE),sysexgen)
    DEFINES := $(filter-out CORE_ARCH_%,$(DEFINES))
    DEFINES := $(filter-out CORE_VENDOR_%,$(DEFINES))
else
    $(error Invalid firmware type specified)
endif

DEFINES += CORE_MCU_FLASH_START_ADDR_USER=$(FLASH_START_ADDR)
DEFINES += BOOT_START_ADDR=$(BOOT_START_ADDR)
DEFINES += APP_START_ADDR=$(APP_START_ADDR)
DEFINES += FW_METADATA_LOCATION=$(FW_METADATA_LOCATION)

# When set to 1, format target will fail if there are any changes to the repository after formatting.
CF_FAIL_ON_DIFF := 0

# When set to 1, lint target will fail if there are any changes to the repository after linting.
CL_FAIL_ON_DIFF := 0