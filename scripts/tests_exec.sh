#!/usr/bin/env bash

# first argument to the script should be path to the directory where all compiled binares are located
# if second argument is set to 1, run only hardware tests

BIN_DIR=$1
HW_TESTING=$2

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

if [ "$(uname)" == "Darwin" ]
then
    find="gfind"
elif [ "$(uname -s)" == "Linux" ]
then
    find="find"
fi

# clean up all temporary profraw files
$find "$BIN_DIR" -type f -name "*.profraw" -exec rm {} \;

if [ "$HW_TESTING" = "1" ]
then
    BINARIES=$($find "$BIN_DIR" -type f -name "*.out" -path "*hw*")
else
    BINARIES=$($find "$BIN_DIR" -type f -name "*.out" -not -path "*hw*")
fi

declare -i RESULT=0

export LLVM_PROFILE_FILE="$BIN_DIR/test-%p.profraw"

for test in $BINARIES
do
    if [ "$HW_TESTING" == "1" ]
    then
        board=$(echo "$test" | cut -d/ -f3)

        if [[ -f ../config/hw-test/"$board".yml ]]
        then
            echo "Running tests on $board board"
            $test
            RESULT+=$?
        fi
    else
        $test
        RESULT+=$?
    fi
done

exit $RESULT