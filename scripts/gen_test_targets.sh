#!/bin/bash

tests=$(find ./src -maxdepth 1 -name "test_*" -type d | cut -d/ -f3)

echo TESTS := $tests > Objects.mk
printf '%s\n' 'TESTS_EXT := $(addprefix $(BUILD_DIR)/,$(TESTS))' >> Objects.mk
printf '%s\n\n' 'TESTS_EXT := $(addsuffix .out,$(TESTS_EXT))' >> Objects.mk
printf '%s\n' '.DEFAULT_GOAL := all' >> Objects.mk
printf '%s\n\n' 'all: $(TESTS_EXT)' >> Objects.mk

printf '%s\n' 'TEST_FRAMEWORK_OBJECTS := $(addprefix $(BUILD_DIR)/,$(TEST_FRAMEWORK_SOURCES))' >> Objects.mk
printf '%s\n\n' 'TEST_FRAMEWORK_OBJECTS := $(addsuffix .o,$(TEST_FRAMEWORK_OBJECTS))' >> Objects.mk

for test in $tests
do
    printf '%s\n' '-include src/'${test}'/Sources.mk' >> Objects.mk
    printf '%s\n' 'SOURCES_'${test}' += gen/main/main_'${test}'.cpp' >> Objects.mk
    printf '%s\n' 'SOURCES_'${test}' += $(shell find ./src/'${test}'/ -type f -name "*.cpp")' >> Objects.mk
    printf '%s\n' 'OBJECTS_'${test}' := $(addprefix $(BUILD_DIR)/,$(SOURCES_'${test}'))' >> Objects.mk
    printf '%s\n' 'OBJECTS_'${test}' := $(addsuffix .o,$(OBJECTS_'${test}'))' >> Objects.mk
    printf '%s\n\n' '-include $(OBJECTS_'${test}':%.o=%.d)' >> Objects.mk
done

for test in $tests
do
    printf '%s\n' '$(BUILD_DIR)/'${test}'.out: $(OBJECTS_'${test}') $(TEST_FRAMEWORK_OBJECTS)' >> Objects.mk
    printf '\t%s\n' '@echo Linking' >> Objects.mk
    printf '\t%s\n' '@$(CPP_COMPILER) $(LDFLAGS) $(COMMON_FLAGS) $(CPP_FLAGS) $^ -o $@' >> Objects.mk
    printf '\t%s\n\n' '@echo Created executable: $@' >> Objects.mk
done