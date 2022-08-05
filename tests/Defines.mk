BUILD_DIR_BASE  := ./build
TARGET          := discovery
DEF_FILE_TARGET := ../config/target/$(TARGET).yml
TYPE            := app
BUILD_DIR       := $(BUILD_DIR_BASE)/$(TARGET)
TEST_BINARIES   := $(addprefix -object ,$(shell $(FIND) $(BUILD_DIR_BASE) -name "*.out" 2>/dev/null))
FW_ROOT_DIR     := ../src
SCRIPTS_DIR     := ../scripts
CLANG_TIDY_OUT  := $(BUILD_DIR_BASE)/clang-tidy-fixes.yaml

ifeq (,$(wildcard $(DEF_FILE_TARGET)))
    $(error Target doesn't exist)
endif

DEFINES += \
TEST \
USE_LOGGER \
GLOG_CUSTOM_PREFIX_SUPPORT

TEST_DEFINES := 1

#override root path for includes in Makefiles located in src directory
MAKEFILE_INCLUDE_PREFIX := ../src/

include $(MAKEFILE_INCLUDE_PREFIX)Defines.mk
-include $(MAKEFILE_INCLUDE_PREFIX)$(BOARD_GEN_DIR_TARGET)/HWTestDefines.mk

#filter out arch symbols to avoid pulling MCU-specific headers
DEFINES := $(filter-out CORE_ARCH_%,$(DEFINES))
DEFINES := $(filter-out CORE_VENDOR_%,$(DEFINES))
DEFINES := $(filter-out DATABASE_INIT_DATA%,$(DEFINES))

DEFINES += DATABASE_INIT_DATA=1