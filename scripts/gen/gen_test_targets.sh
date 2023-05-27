#!/usr/bin/env bash

run_dir="OpenDeck/tests"

if [[ $(pwd) != *"$run_dir" ]]
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

# Find all directories containing test sources.
# To do so, only take into account directories which contain Makefile,
# but also ignore directories containing .testignore.

tests=$($find ./src -type d '!' -exec test -e "{}/.testignore" ';' -exec test -e "{}/Makefile" ';' -print | rev | cut -d/ -f1 | rev | tr "\n" " ")

{
    printf '%s ' "TESTS := ${tests}" > Objects.mk
    printf '%s\n' '' >> Objects.mk
}

{
    printf '%s\n' 'OBJECTS_COMMON := $(addprefix $(BUILD_DIR)/,$(SOURCES_COMMON))'
    printf '%s\n\n' 'OBJECTS_COMMON := $(addsuffix .o,$(OBJECTS_COMMON))'
} >> Objects.mk

for test in $tests
do
    test_dir=$($find src -type d -name "*${test}")

    {
        printf '%s\n' '-include '${test_dir}'/Makefile'
        printf '%s\n' 'TEST_DIR_'${test}' := '${test_dir}''
        printf '%s\n' 'SOURCES_'${test}' += src/main.cpp'
        printf '%s\n' 'SOURCES_'${test}' += $(shell $(FIND) '${test_dir}' -type f -maxdepth 1 -name "*.cpp")'
        printf '%s\n' 'OBJECTS_'${test}' := $(addprefix $(BUILD_DIR)/$(TEST_DIR_'${test}')/,$(SOURCES_'${test}'))'
        printf '%s\n' 'OBJECTS_'${test}' := $(addsuffix .o,$(OBJECTS_'${test}'))'
        printf '%s\n\n' '-include $(OBJECTS_'${test}':%.o=%.d)'

        printf '%s\n' '$(BUILD_DIR)/$(TEST_DIR_'${test}')/%.cpp.o: %.cpp'
        printf '\t%s\n' '@mkdir -p $(@D)'
        printf '\t%s\n' '@echo Building: $<'
        printf '\t%s\n' '@$(CCACHE) $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDE_FILES_COMMON) $(INCLUDE_DIRS_COMMON) $(addprefix -I$(FW_ROOT_DIR)/,$(INCLUDE_DIRS_'${test}')) $(addprefix -D,$(DEFINES)) $(addprefix -D,$(DEFINES_'${test}')) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"'

        printf '%s\n' '$(BUILD_DIR)/$(TEST_DIR_'${test}')/%.c.o: %.c'
        printf '\t%s\n' '@mkdir -p $(@D)'
        printf '\t%s\n' '@echo Building: $<'
        printf '\t%s\n' '@$(CCACHE) $(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDE_FILES_COMMON) $(INCLUDE_DIRS_COMMON) $(addprefix -I$(FW_ROOT_DIR)/,$(INCLUDE_DIRS_'${test}')) $(addprefix -D,$(DEFINES)) $(addprefix -D,$(DEFINES_'${test}')) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"'

        printf '\n%s\n' 'ifeq ($(findstring '${test}',$(TESTS)), '${test}')'
        printf '    %s\n' 'TESTS_EXPANDED += $(BUILD_DIR)/$(TEST_DIR_'${test}')/'${test}'.out'
        printf '%s\n' 'endif'

        printf '\n%s\n' '$(BUILD_DIR)/$(TEST_DIR_'${test}')/'${test}'.out: $(OBJECTS_'${test}') $(OBJECTS_COMMON)'
        printf '\t%s\n' '$(LINK_OBJECTS)'
    } >> Objects.mk
done

printf '\n%s\n' 'test: $(TESTS_EXPANDED)' >> Objects.mk