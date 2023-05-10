ROOT_MAKEFILE_DIR          := $(realpath $(dir $(realpath $(lastword $(MAKEFILE_LIST)))))
PROJECT                    := $(shell basename $(shell dirname $(ROOT_MAKEFILE_DIR)))
BUILD_DIR_BASE             := $(ROOT_MAKEFILE_DIR)/build
CONFIG_DIR_BASE            := $(ROOT_MAKEFILE_DIR)/../config
TARGET_CONFIG_DIR_BASE     := $(CONFIG_DIR_BASE)/target
TSCREEN_CONFIG_DIR_BASE    := $(CONFIG_DIR_BASE)/touchscreen
TARGET                     := $(shell basename $(shell $(FIND) $(TARGET_CONFIG_DIR_BASE) -type f | sort | head -n 1) .yml)
DEF_FILE_TARGET            := $(TARGET_CONFIG_DIR_BASE)/$(TARGET).yml
DEF_FILE_TSCREEN           := $(TSCREEN_CONFIG_DIR_BASE)/$(TARGET).json
SCRIPTS_DIR                := $(ROOT_MAKEFILE_DIR)/../scripts
DEPS_SUBDIR_BASE           := deps
GENERATED_SUBDIR_BASE      := generated
MCU_GEN_DIR_BASE           := $(ROOT_MAKEFILE_DIR)/$(GENERATED_SUBDIR_BASE)/mcu
TARGET_GEN_DIR_BASE        := $(ROOT_MAKEFILE_DIR)/$(GENERATED_SUBDIR_BASE)/target
APPLICATION_GEN_DIR_BASE   := $(ROOT_MAKEFILE_DIR)/$(GENERATED_SUBDIR_BASE)/application
MCU_GEN_DIR                 = $(MCU_GEN_DIR_BASE)/$(CORE_MCU_MODEL)
TARGET_GEN_DIR             := $(TARGET_GEN_DIR_BASE)/$(TARGET)
APPLICATION_GEN_DIR_TARGET := $(APPLICATION_GEN_DIR_BASE)/$(TARGET)
CLANG_TIDY_OUT             := $(BUILD_DIR_BASE)/clang-tidy-fixes.yaml

ifeq (,$(wildcard $(DEF_FILE_TARGET)))
    $(error Target doesn't exist)
endif

ifeq ($(DEBUG),1)
    BUILD_TYPE := debug
else
    BUILD_TYPE := release
endif

BUILD_DIR := $(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)
TYPE      := application

ifneq (,$(filter $(MAKECMDGOALS),concat))
    BUILD_DIR := $(BUILD_DIR)/merged
else
    BUILD_DIR := $(BUILD_DIR)/$(TYPE)
endif

OUTPUT                     := $(BUILD_DIR)/$(TARGET)
BUILD_TIME_FILE            := $(BUILD_DIR_BASE)/lastbuild
LAST_BUILD_TIME            := $(shell cat $(BUILD_TIME_FILE) 2>/dev/null | awk '{print$$1}END{if(NR==0)print 0}')
FLASH_BINARY_DIR           := $(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged

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