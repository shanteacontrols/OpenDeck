#!/bin/bash

base_dir=$(make TARGETNAME="$1" print-BUILD_DIR)
test_dir_var=TEST_DIR_$2
test_dir=$(make print-"$test_dir_var")

echo "$base_dir"/"$test_dir"