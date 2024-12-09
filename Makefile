FIND                       := find 2>/dev/null
ROOT_MAKEFILE_DIR          := $(realpath $(dir $(realpath $(lastword $(MAKEFILE_LIST)))))
BUILD_DIR_BASE             := $(ROOT_MAKEFILE_DIR)/build
GENERATED_DIR_BASE         := $(ROOT_MAKEFILE_DIR)/src/generated
CONFIG_DIR_BASE            := $(ROOT_MAKEFILE_DIR)/config
TARGET_CONFIG_DIR_BASE     := $(CONFIG_DIR_BASE)/target
TARGET                     := $(shell basename $(shell $(FIND) $(TARGET_CONFIG_DIR_BASE) -type f | sort | head -n 1) .yml)
TARGET_BUILD_DIR           := $(BUILD_DIR_BASE)/$(TARGET)
TEST_EXECUTE               := 1
CMAKE_CONFIG_ARGS          :=
WEST_UPDATED_FILE          := .west-updated
WEST_MANIFESTS             := west.yml

ifneq (,$(shell which ninja))
    CMAKE_CONFIG_ARGS += -GNinja
endif

ifeq ($(DEBUG),1)
    CMAKE_CONFIG_ARGS += -DCMAKE_BUILD_TYPE=Debug
    TARGET_BUILD_DIR := $(TARGET_BUILD_DIR)/debug
else
    TARGET_BUILD_DIR := $(TARGET_BUILD_DIR)/release
endif

CMAKE_CONFIG_ARGS += \
-B $(TARGET_BUILD_DIR) \
-S $(ROOT_MAKEFILE_DIR) \
-DTARGET=$(TARGET)

.DEFAULT_GOAL := all

$(WEST_UPDATED_FILE): $(WEST_MANIFESTS)
	@echo "Running west update..."
	@west update
	@touch $@

cmake_config: $(WEST_UPDATED_FILE)
	@if [ ! -d $(TARGET_BUILD_DIR) ]; then \
		echo "Generating CMake files"; \
		cmake $(CMAKE_CONFIG_ARGS); \
	fi

all: cmake_config
	@cmake --build $(TARGET_BUILD_DIR)

test: cmake_config
	@cmake --build $(TARGET_BUILD_DIR) --target tests
ifeq ($(TEST_EXECUTE),1)
	@ctest --test-dir $(TARGET_BUILD_DIR)/tests --exclude-regex hw --verbose
endif

hw-test: cmake_config
	@cmake --build $(TARGET_BUILD_DIR) --target tests
ifeq ($(TEST_EXECUTE),1)
	ctest --test-dir $(TARGET_BUILD_DIR)/tests --tests-regex hw --verbose
endif

flash: cmake_config
	@PROBE_ID=$(PROBE_ID) PORT=$(PORT) FLASH_TOOL=$(FLASH_TOOL) FLASH_BINARY_DIR=$(FLASH_BINARY_DIR) cmake --build $(TARGET_BUILD_DIR) --target flash

format: cmake_config
	@cmake --build $(TARGET_BUILD_DIR) --target format

lint: cmake_config
	@cmake --build $(TARGET_BUILD_DIR) --target lint

clean:
	@echo Cleaning up.
	@rm -rf $(BUILD_DIR_BASE) $(GENERATED_DIR_BASE)

# Debugging
print-%:
	@echo '$($*)'

.PHONY: cmake_config all test hw-test flash format lint clean