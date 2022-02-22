#!/usr/bin/env bash

run_dir="OpenDeck/tests"

if [[ $(pwd) != *"$run_dir" ]]
then
    echo This script must be run from $run_dir directory!
    exit 1
fi

base_dir=$(make TARGET="$1" print-BUILD_DIR)
test_dir_var=TEST_DIR_$2
test_dir=$(make print-"$test_dir_var")

echo "$base_dir"/"$test_dir"