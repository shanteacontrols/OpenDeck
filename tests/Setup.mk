ROOT_MAKEFILE_DIR    := $(realpath $(dir $(realpath $(lastword $(MAKEFILE_LIST)))))
BUILD_DIR_BASE       := $(ROOT_MAKEFILE_DIR)/build
BUILD_DIR            := $(BUILD_DIR_BASE)/$(TARGET)
SCRIPTS_DIR          := $(ROOT_MAKEFILE_DIR)/../scripts
FW_ROOT_DIR          := $(ROOT_MAKEFILE_DIR)/../src
TEST_BINARIES        := $(addprefix -object ,$(shell $(FIND) $(BUILD_DIR_BASE) -name "*.out" 2>/dev/null))