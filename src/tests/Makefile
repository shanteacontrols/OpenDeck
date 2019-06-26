include GTest.mk
include Defines.mk
include Sources.mk
-include Objects.mk

CPP := clang
CXX := clang++

#passed to both c and c++ compiler
COMMON_FLAGS := \
-isystem $(GTEST_DIR)/include \
-isystem $(GTEST_DIR) \
-fprofile-arcs \
-ftest-coverage \
-Og \
-g \
-pthread \
-Wall

#c++ compiler only
CPP_FLAGS := \
-std=c++11

#c compiler only
C_FLAGS := \
-std=c11

#linker
LDFLAGS := \
-lgcov \
--coverage \
-lpthread

#doesn't remove gtest sources
clean:
	@echo Cleaning up.
	@if [ "$(wildcard build/)" != "" ]; then\
		find ./build ! -name 'gtest_main.a' ! -name 'gtest-all.o' ! -name 'gtest_main.o' -type f -exec rm -f {} +;\
	fi

clean-all:
	@echo Cleaning up.
	@rm -rf build/

build/%.o: %.c $(GTEST_HEADERS)
	@mkdir -p $(@D)
	@echo Building $<
	@$(CPP) $(COMMON_FLAGS) $(C_FLAGS) $(addprefix -D,$(DEFINES)) $(INCLUDE_DIRS) $(INCLUDE_FILES) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

build/%.o: %.cpp $(GTEST_HEADERS)
	@mkdir -p $(@D)
	@echo Building $<
	@$(CXX) $(COMMON_FLAGS) $(CPP_FLAGS) $(addprefix -D,$(DEFINES)) $(INCLUDE_DIRS) $(INCLUDE_FILES) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

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
