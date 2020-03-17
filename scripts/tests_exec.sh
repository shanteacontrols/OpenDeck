#!/bin/bash

# first argument to the script should be path to the directory where all compiled binares are located

BIN_DIR=$1

run_dir="tests"

if [[ $(basename "$(pwd)") != "$run_dir" ]]
then
    echo This script must be run from $run_dir directory!
    exit 1
fi

if [[ "${1}" == "" ]]
then
    echo "Build directory not provided"
    exit 1
fi

if [[ ! -d "$BIN_DIR" ]]
then
    echo "Directory $BIN_DIR doesn't exist"
    exit 1
fi

# clean up all temporary profraw files
find "$BIN_DIR" -type f -name "*.profraw" -exec rm {} \;

BINARIES=$(find "$BIN_DIR" -type f -name "*.out")

declare -i RESULT=0

export LLVM_PROFILE_FILE="$BIN_DIR/test-%p.profraw"

for test in $BINARIES
do
    $test
    RESULT+=$?
done

exit $RESULT