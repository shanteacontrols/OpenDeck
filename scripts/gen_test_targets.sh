#!/bin/bash

run_dir="tests"

if [[ $(basename "$(pwd)") != "$run_dir" ]]
then
    echo This script must be run from $run_dir directory!
    exit 1
fi

if [ "$(uname)" == "Darwin" ]
then
    find="gfind"
elif [ "$(uname -s)" == "Linux" ]
then
    find="find"
fi

# first argument should be directory in which generated files should be stored

GEN_DIR=$1
mkdir -p "$GEN_DIR"

#find all directories containing test source
#to do so, only take into account directories which contain Makefile
tests=$(find ./src -type f -name Makefile -print0 | xargs -0 dirname | xargs -n 1 basename | tr "\n" " ")

{
    printf '%s ' "TESTS := ${tests}" > Objects.mk
    printf '%s\n' '' >> Objects.mk
}

{
    printf '%s\n' 'COMMON_OBJECTS := $(addprefix $(BUILD_DIR_BASE)/,$(COMMON_SOURCES))'
    printf '%s\n\n' 'COMMON_OBJECTS := $(addsuffix .o,$(COMMON_OBJECTS))'
} >> Objects.mk

for test in $tests
do
    test_dir=$($find src -type d -name "*${test}")
    # echo "test dir is $test_dir"

    {
        printf '%s\n' '-include '${test_dir}'/Makefile'
        printf '%s\n' 'TEST_DIR_'${test}' := '${test_dir}''
        printf '%s\n' 'SOURCES_'${test}' += '$GEN_DIR'/'${test}'.cpp'
        printf '%s\n' 'SOURCES_'${test}' += $(shell find '${test_dir}' -type f -name "*.cpp")'
        printf '%s\n' 'OBJECTS_'${test}' := $(addprefix $(BUILD_DIR)/$(TEST_DIR_'${test}')/,$(SOURCES_'${test}'))'
        printf '%s\n' 'OBJECTS_'${test}' := $(addsuffix .o,$(OBJECTS_'${test}'))'
        printf '%s\n\n' '-include $(OBJECTS_'${test}':%.o=%.d)'

        printf '%s\n' '$(BUILD_DIR)/$(TEST_DIR_'${test}')/%.cpp.o: %.cpp'
        printf '\t%s\n' '@mkdir -p $(@D)'
        printf '\t%s\n' '@echo Building $<'
        printf '\t%s\n' '@$(CPP_COMPILER) $(COMMON_FLAGS) $(CPP_FLAGS) $(INCLUDE_FILES_COMMON) $(INCLUDE_DIRS_COMMON) $(addprefix -I$(FW_ROOT_DIR)/,$(INCLUDE_DIRS_'${test}')) $(addprefix -D,$(DEFINES_COMMON)) $(addprefix -D,$(DEFINES_'${test}')) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"'

        printf '%s\n' '$(BUILD_DIR)/$(TEST_DIR_'${test}')/%.c.o: %.c'
        printf '\t%s\n' '@mkdir -p $(@D)'
        printf '\t%s\n' '@echo Building $<'
        printf '\t%s\n' '@$(C_COMPILER) $(COMMON_FLAGS) $(C_FLAGS) $(INCLUDE_FILES_COMMON) $(INCLUDE_DIRS_COMMON) $(addprefix -I$(FW_ROOT_DIR)/,$(INCLUDE_DIRS_'${test}')) $(addprefix -D,$(DEFINES_COMMON)) $(addprefix -D,$(DEFINES_'${test}')) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"'

        printf '\n%s\n' 'ifeq ($(findstring '${test}',$(TESTS)), '${test}')'
        printf '    %s\n' 'TESTS_EXPANDED += $(BUILD_DIR)/$(TEST_DIR_'${test}')/'${test}'.out'
        printf '%s\n' 'endif'

        printf '\n%s\n' '$(BUILD_DIR)/$(TEST_DIR_'${test}')/'${test}'.out: $(OBJECTS_'${test}') $(COMMON_OBJECTS)'
        printf '\t%s\n' '$(LINK_OBJECTS)'
    } >> Objects.mk
done

printf '\n%s\n' 'test: $(TESTS_EXPANDED)' >> Objects.mk