include GTest.mk
include Defines.mk
include Sources.mk

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include -isystem $(GTEST_DIR) --coverage -std=c++11

# Flags passed to the C++ compiler.
CXXFLAGS += -g -pthread
ARFLAGS := r

TESTS := database

TESTS_EXT := $(addprefix build/,$(TESTS))
TESTS_EXT := $(addsuffix .out,$(TESTS_EXT))

.DEFAULT_GOAL := all

all: $(TESTS_EXT)

clean:
	@echo Cleaning up.
	@rm -rf build/

build/%.o: %.cpp $(GTEST_HEADERS)
	@mkdir -p $(@D)
	@echo Building $<
	@$(CXX) $(CPPFLAGS) $(addprefix -D,$(DEFINES)) $(CXXFLAGS) $(INCLUDE_DIRS) $(INCLUDE_FILES) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

build/database.out: $(OBJECTS_database) build/gtest_main.a
	@echo Linking.
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@
	@echo Created executable: $@

exec:
	@echo Running all compiled tests.
	@chmod +x RunTests.sh && ./RunTests.sh

coverage:
	@echo Creating coverage report.
	@lcov --directory build/ --capture --output-file build/coverage.info > /dev/null 2>&1
	@lcov --remove build/coverage.info '/usr/*' '*crypto*' '*gtest*' '*stubs*' '*tests*' '*googletest*' --output-file build/coverage.info 2>&1
	@lcov --list build/coverage.info > /dev/null 2>&1
	@genhtml -o build/ build/coverage.info > /dev/null 2>&1

#debugging
print-%:
	@echo '$*=$($*)'
