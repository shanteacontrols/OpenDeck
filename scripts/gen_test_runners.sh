#!/bin/bash

if [ "$(uname)" == "Darwin" ]; then
    grep=ggrep
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    grep=grep
fi

mkdir -p gen/runners
mkdir -p gen/main

tests=$(find ./src -maxdepth 1 -name "test_*" -type d | cut -d/ -f3)

#use ctags to generate list of tests to be run by RUN_TEST unity macro
#this works by filtering all functions defined with TEST_CASE macro

for test in $tests
do
    printf '%s\n\n' '#pragma once' > gen/runners/runner_${test}.h

    find ./src/${test} -type f -regex '.*\.\(cpp\|c\)' -exec ctags -x --c-kinds=f {} ';' > gen/runners/table

    #some test functions, depending on the target, aren't actually compiled
    #ie. this parsing will find test functions for ODMIDIformat tests but some
    #targets do not support DIN MIDI, therefore, those functions aren't
    #going to be compiled
    #in that case compiling wouldn't pass due to the linker errors: TESTS_EXECUTE
    #tries to call functions which aren't preset in any source file
    #for that reason, to avoid compile errors, define each found test with
    #__attribute__((weak)) to make sure the test actually exists
    $grep -oP '(?<=TEST_CASE\()(.*)(?=\))' gen/runners/table | sed 's/$/() {}/' | sed 's/^/__attribute__((weak)) void /' > gen/runners/runner_${test}.cpp

    printf '%s\n' 'void TESTS_EXECUTE() {' >> gen/runners/runner_${test}.cpp

    $grep -oP '(?<=TEST_CASE\()(.*)(?=\))' gen/runners/table | sed 's/$/);/' | sed 's/^/RUN_TEST(/' >> gen/runners/runner_${test}.cpp
    $grep -oP '(?<=TEST_CASE\()(.*)(?=\))' gen/runners/table | sed 's/$/();/' | sed 's/^/void /' >> gen/runners/runner_${test}.h

    printf '%s\n' '}' >> gen/runners/runner_${test}.cpp

    printf '%s\n' '#include "gen/runners/runner_'${test}'.h"' > gen/main/main_${test}.cpp
    printf '%s\n\n' '#include "unity/src/unity.h"' >> gen/main/main_${test}.cpp
    printf '%s\n' '#include "gen/runners/runner_'${test}'.cpp"' >> gen/main/main_${test}.cpp
    printf '%s' '#include "unity/main.cpp"' >> gen/main/main_${test}.cpp
done