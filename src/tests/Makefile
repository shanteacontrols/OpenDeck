include GTest.mk
include Defines.mk
include Sources.mk
-include Objects.mk

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS := -isystem $(GTEST_DIR)/include -isystem $(GTEST_DIR) --coverage -std=c++11 -Og

# Flags passed to the C++ compiler.
CXXFLAGS := -g -pthread -Og
ARFLAGS := r

TESTS := database encoders pots

TESTS_EXT := $(addprefix build/,$(TESTS))
TESTS_EXT := $(addsuffix .out,$(TESTS_EXT))

.DEFAULT_GOAL := all

all: $(TESTS_EXT)

#doesn't remove gtest sources
clean:
	@echo Cleaning up.
	@if [ "$(wildcard build/)" != "" ]; then\
		find ./build ! -name 'gtest_main.a' ! -name 'gtest-all.o' ! -name 'gtest_main.o' -type f -exec rm -f {} +;\
	fi

clean-all:
	@echo Cleaning up.
	@rm -rf build/

build/%.o: %.cpp $(GTEST_HEADERS)
	@mkdir -p $(@D)
	@echo Building $<
	@$(CXX) $(CPPFLAGS) $(addprefix -D,$(DEFINES)) $(CXXFLAGS) $(INCLUDE_DIRS) $(INCLUDE_FILES) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

pre-build:
	@chmod +x ../../scripts/gen_tests_targets.sh && ../../scripts/gen_tests_targets.sh

exec:
	@echo Running all compiled tests.
	@chmod +x ../../scripts/execute_tests.sh && ../../scripts/execute_tests.sh

coverage:
	@echo Creating coverage report.
	@lcov --directory build/ --capture --output-file build/coverage.info > /dev/null 2>&1
	@lcov --remove build/coverage.info '/usr/*' '*crypto*' '*gtest*' '*stubs*' '*modules*' '*tests*' '*googletest*' --output-file build/coverage.info 2>&1
	@lcov --list build/coverage.info > /dev/null 2>&1
	@genhtml -o build/ build/coverage.info > /dev/null 2>&1

#debugging
print-%:
	@echo '$*=$($*)'
