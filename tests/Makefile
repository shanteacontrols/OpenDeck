include ../src/Toolchain.mk

.DEFAULT_GOAL   := test

BUILD_DIR_BASE  := ./build
TARGET          := discovery
TARGET_DEF_FILE := ../targets/$(TARGET).yml
TYPE            := app
BUILD_DIR       := $(BUILD_DIR_BASE)/$(TARGET)
TEST_BINARIES   := $(addprefix -object ,$(shell $(FIND) $(BUILD_DIR_BASE) -name "*.out" 2>/dev/null))
FW_ROOT_DIR     := ../src
SCRIPTS_DIR     := ../scripts

C_COMPILER      := clang
CPP_COMPILER    := clang++
LLVM_PROFDATA   := llvm-profdata
LLVM_COV        := llvm-cov
CLANG_FORMAT    := clang-format

include Defines.mk
include Sources.mk
-include Objects.mk

#passed to both c and c++ compiler
COMMON_FLAGS := \
-fprofile-instr-generate \
-fcoverage-mapping \
-O0 \
-g \
-Wall

#c++ compiler only
CPP_FLAGS := \
-std=c++17

#c compiler only
C_FLAGS := \
-std=c11

#linker
LDFLAGS :=

$(BUILD_DIR)/%.c.o $(BUILD_DIR_BASE)/%.c.o: %.c
	@mkdir -p $(@D)
	@echo Building: $<
	@$(C_COMPILER) $(COMMON_FLAGS) $(C_FLAGS) $(addprefix -D,$(DEFINES)) $(INCLUDE_DIRS_COMMON) $(INCLUDE_FILES_COMMON) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

$(BUILD_DIR)/%.cpp.o $(BUILD_DIR_BASE)/%.cpp.o: %.cpp
	@mkdir -p $(@D)
	@echo Building: $<
	@$(CPP_COMPILER) $(COMMON_FLAGS) $(CPP_FLAGS) $(addprefix -D,$(DEFINES)) $(INCLUDE_DIRS_COMMON) $(INCLUDE_FILES_COMMON) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

define LINK_OBJECTS
	@echo Creating executable: $@
	@$(CPP_COMPILER) $(LDFLAGS) $(COMMON_FLAGS) $(CPP_FLAGS) $^ -o $@
endef

pre-build:
	@$(SCRIPTS_DIR)/gen_test_targets.sh

exec:
	@echo Running all compiled tests.
	@$(SCRIPTS_DIR)/tests_exec.sh $(BUILD_DIR_BASE) $(HW_TESTING)

coverage:
	@echo Creating coverage report.
	@$(LLVM_PROFDATA) merge $(wildcard $(BUILD_DIR_BASE)/*.profraw) -o $(BUILD_DIR_BASE)/tests.profdata
	@$(LLVM_COV) report $(TEST_BINARIES) -instr-profile=$(BUILD_DIR_BASE)/tests.profdata -ignore-filename-regex="test|modules" -use-color
	@$(LLVM_COV) show $(TEST_BINARIES) -instr-profile=$(BUILD_DIR_BASE)/tests.profdata -ignore-filename-regex="test|modules" -use-color --format html > $(BUILD_DIR_BASE)/coverage.html
	@echo ***Generated HTML coverage report in $(BUILD_DIR_BASE)/coverage.html***

format:
	@echo Checking code formatting...
	@$(FIND) . -regex '.*\.\(cpp\|hpp\|h\|cc\|cxx\)' \
	-exec $(CLANG_FORMAT) -style=file -i {} \;
ifeq ($(CF_FAIL_ON_DIFF), 1)
	git diff
	git diff -s --exit-code
endif

clean:
	@echo Cleaning up.
	@rm -rf $(BUILD_DIR_BASE)/

#debugging
print-%:
	@echo '$($*)'
