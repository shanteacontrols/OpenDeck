TEST_BINARIES   := $(addprefix -object ,$(shell $(FIND) $(BUILD_DIR_BASE) -name "*.out" 2>/dev/null))
FW_ROOT_DIR     := ../src
SCRIPTS_DIR     := ../scripts

# Override root path for includes in Makefiles located in src directory
MAKEFILE_INCLUDE_PREFIX := ../src/