PROJECT              := OpenDeck
BUILD_DIR_BASE       := ./build
TARGET               := discovery
DEF_FILE_TARGET      := ../config/target/$(TARGET).yml
DEF_FILE_TSCREEN     := ../config/touchscreen/$(TARGET).json
TYPE                 := application
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

BUILD_DIR := $(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)

ifneq (,$(filter $(MAKECMDGOALS),concat))
    BUILD_DIR := $(BUILD_DIR)/merged
else
    BUILD_DIR := $(BUILD_DIR)/$(TYPE)
endif

OUTPUT                      := $(BUILD_DIR)/$(TARGET)
BUILD_TIME_FILE             := $(BUILD_DIR_BASE)/lastbuild
LAST_BUILD_TIME             := $(shell cat $(BUILD_TIME_FILE) 2>/dev/null | awk '{print$$1}END{if(NR==0)print 0}')
FLASH_BINARY_DIR            := $(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged
SYSEX_BINARY_SUFFIX         := _sysex

BOARD_GEN_DIR_BASE          := board/gen
BOARD_GEN_DIR_TARGET_BASE   := $(BOARD_GEN_DIR_BASE)/target
BOARD_GEN_DIR_MCU_BASE      := $(BOARD_GEN_DIR_BASE)/mcu
BOARD_GEN_DIR_VENDOR_BASE   := $(BOARD_GEN_DIR_BASE)/vendor
BOARD_GEN_DIR_ARCH_BASE     := $(BOARD_GEN_DIR_BASE)/arch
BOARD_GEN_DIR_TARGET        := $(BOARD_GEN_DIR_TARGET_BASE)/$(TARGET)
APP_GEN_DIR_BASE            := application/gen
APP_GEN_DIR_TARGET          := $(APP_GEN_DIR_BASE)/$(TARGET)

# Verbose builds
ifeq ($(V),1)
    Q :=
else
    Q := @
endif

# When set to 1, format target will fail if there are any changes to the repository after formatting.
CF_FAIL_ON_DIFF := 0

# When set to 1, lint target will fail if there are any changes to the repository after linting.
CL_FAIL_ON_DIFF := 0