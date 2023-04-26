# Important - do not change the order of inclusion!

include ../src/Toolchain.mk
include ../src/Setup.mk
-include $(BOARD_GEN_DIR)/Makefile
-include ../src/board/src/arch/$(CORE_MCU_ARCH)/Makefile
-include ../src/board/src/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/Makefile
-include ../src/board/src/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/$(CORE_MCU_FAMILY)/Makefile
-include ../src/board/src/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/$(CORE_MCU_FAMILY)/$(CORE_MCU_MODEL)/Makefile
-include ../src/$(TYPE)/Makefile
include ../src/Defines.mk
include Setup.mk
-include $(BOARD_GEN_DIR)/HWTestDefines.mk
include Defines.mk
include Sources.mk
-include Objects.mk

CC              := clang
CXX             := clang++
LLVM_PROFDATA   := llvm-profdata
LLVM_COV        := llvm-cov
CLANG_FORMAT    := clang-format

# Passed to both C and C++ compiler
CPPFLAGS := \
-fprofile-instr-generate \
-fcoverage-mapping \
-O0 \
-g \
-Wall \
-fdata-sections \
-ffunction-sections \
-Wsign-compare \
-Werror

# C++ compiler only
CXXFLAGS := \
-std=c++20 \
-Wpedantic \
-Wno-inconsistent-missing-override

# C compiler only
CFLAGS := \
-std=c11

# Linker flags
LDFLAGS := \
-lglog \
-lpthread \
-lgmock \
-lgtest \
-no-pie

.DEFAULT_GOAL := test

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(@D)
	@echo Building: $<
	@$(CCACHE) $(CC) $(CPPFLAGS) $(CFLAGS) $(addprefix -D,$(DEFINES)) $(INCLUDE_DIRS_COMMON) $(INCLUDE_FILES_COMMON) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(@D)
	@echo Building: $<
	@$(CCACHE) $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(addprefix -D,$(DEFINES)) $(INCLUDE_DIRS_COMMON) $(INCLUDE_FILES_COMMON) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

define LINK_OBJECTS
	@echo Creating executable: $@
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
endef

generate:
	@$(SCRIPTS_DIR)/gen/gen_test_targets.sh

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

lint:
	@infer capture -- $(MAKE) --no-print-directory TARGET=$(TARGET) DEBUG=$(DEBUG) CCACHE=
	@infer analyze

clean:
	@echo Cleaning up.
	@rm -rf $(BUILD_DIR_BASE)/ infer-out

# Debugging
print-%:
	@echo '$($*)'
