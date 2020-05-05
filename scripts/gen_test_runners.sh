#!/bin/bash

run_dir="tests"

if [[ $(basename "$(pwd)") != "$run_dir" ]]
then
    echo This script must be run from $run_dir directory!
    exit 1
fi

if [ "$(uname)" == "Darwin" ]
then
    grep="ggrep"
    find="gfind"
elif [ "$(uname -s)" == "Linux" ]
then
    grep="grep"
    find="find"
fi

# first argument should be directory in which generated files should be stored

GEN_DIR=$1
mkdir -p "$GEN_DIR"/runners

#find all directories containing test source
#to do so, only take into account directories which contain Makefile
tests=$($find ./src -type f -name Makefile | rev | cut -d / -f 2 | rev | tr "\n" " ")

#use ctags to generate list of tests to be run by RUN_TEST unity macro
#this works by filtering all functions defined with TEST_CASE macro

for test in $tests
do
    printf '%s\n\n' '#pragma once' > "$GEN_DIR"/runners/runner_"${test}".h

    $find "$($find src -type d -name "*${test}")" -type f -regex '.*\.\(cpp\|c\)' -exec ctags -x --c-kinds=f {} ';' > "$GEN_DIR"/runners/table

    #make sure all tests can be compiled even when certain functions are behind #ifdef

    {
        $grep -oP '(?<=TEST_CASE\()(.*)(?=\))' "$GEN_DIR"/runners/table | sed 's/$/() {}/' | sed 's/^/__attribute__((weak)) void /'
        printf '%s\n' 'void TESTS_EXECUTE() {'
        $grep -oP '(?<=TEST_CASE\()(.*)(?=\))' "$GEN_DIR"/runners/table | sed 's/$/);/' | sed 's/^/RUN_TEST(/'
        printf '%s\n' '}'
    } > "$GEN_DIR"/runners/runner_"${test}".cpp

    $grep -oP '(?<=TEST_CASE\()(.*)(?=\))' "$GEN_DIR"/runners/table | sed 's/$/();/' | sed 's/^/void /' >> "$GEN_DIR"/runners/runner_"${test}".h

    {
        printf '%s\n' "#include \"$GEN_DIR/runners/runner_${test}.h\""
        printf '%s\n\n' '#include "unity/src/unity.h"'
        printf '%s\n' "#include \"$GEN_DIR/runners/runner_${test}.cpp\""
        cat unity/main.cpp
    } > "$GEN_DIR"/"${test}".cpp

    rm "$GEN_DIR"/runners/table
done