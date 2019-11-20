TARGETNAME := fw_opendeck
BUILD_DIR_BASE := ./build
BUILD_DIR := $(BUILD_DIR_BASE)/$(TARGETNAME)

include Defines.mk
include Sources.mk
-include Objects.mk

CPP := clang
CXX := clang++
LLVM_PROFDATA := llvm-profdata-9
LLVM_COV := llvm-cov-9

TEST_BINARIES := $(addprefix -object ,$(shell find $(BUILD_DIR_BASE) -name "*.out" 2>/dev/null))

#passed to both c and c++ compiler
COMMON_FLAGS := \
-fprofile-instr-generate \
-fcoverage-mapping \
-Og \
-g \
-Wall

#c++ compiler only
CPP_FLAGS := \
-std=c++11

#c compiler only
C_FLAGS := \
-std=c11

#linker
LDFLAGS :=

clean:
	@echo Cleaning up.
	@rm -rf $(BUILD_DIR_BASE)/

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(@D)
	@echo Building $<
	@$(CPP) $(COMMON_FLAGS) $(C_FLAGS) $(addprefix -D,$(DEFINES)) $(INCLUDE_DIRS) $(INCLUDE_FILES) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(@D)
	@echo Building $<
	@$(CXX) $(COMMON_FLAGS) $(CPP_FLAGS) $(addprefix -D,$(DEFINES)) $(INCLUDE_DIRS) $(INCLUDE_FILES) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

pre-build:
	@chmod +x ../scripts/gen_test_targets.sh && ../scripts/gen_test_targets.sh
	@chmod +x ../scripts/gen_test_runners.sh && ../scripts/gen_test_runners.sh

exec:
	@echo Running all compiled tests.
	@rm -rf $(wildcard $(BUILD_DIR_BASE)/*.profraw)
	@export LLVM_PROFILE_FILE="$(BUILD_DIR_BASE)/test-%p.profraw" && \
	find $(BUILD_DIR_BASE)/ -type f -name "*.out" -printf 'Running %p\n' -exec {} \;

coverage:
	@echo Creating coverage report.
	@$(LLVM_PROFDATA) merge $(wildcard $(BUILD_DIR_BASE)/*.profraw) -o $(BUILD_DIR_BASE)/tests.profdata
	@$(LLVM_COV) report $(TEST_BINARIES) -instr-profile=$(BUILD_DIR_BASE)/tests.profdata -ignore-filename-regex="test|modules" -use-color
	@$(LLVM_COV) show $(TEST_BINARIES) -instr-profile=$(BUILD_DIR_BASE)/tests.profdata -ignore-filename-regex="test|modules" -use-color --format html > $(BUILD_DIR_BASE)/coverage.html
	@echo ***Generated HTML coverage report in $(BUILD_DIR_BASE)/coverage.html***

format:
	@make -C ../src format --no-print-directory

#debugging
print-%:
	@echo '$*=$($*)'
