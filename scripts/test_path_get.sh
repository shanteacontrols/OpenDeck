#!/usr/bin/env bash

# Retrieve the path of the provided test name for a given target.
# Needed for VSCode to avoid having launch config for every test.

target=$1
test=$2
base_dir=$(make TARGET="$target" print-TARGET_BUILD_DIR)
test_dir="$base_dir"/tests
test_path=$(find "$test_dir" -type f -name "*$test.elf")

echo "$test_path"