#!/usr/bin/env bash

run_dir="OpenDeck/tests"

if [[ $(pwd) != *"$run_dir" ]]
then
    echo This script must be run from $run_dir directory!
    exit 1
fi

# First argument to the script should be path to the directory where all compiled binares are located.
# If second argument is set to 1, run only hardware tests.

bin_dir=$1
hw_testing=$2


if [[ "${1}" == "" ]]
then
    echo "Build directory not provided"
    exit 1
fi

if [[ ! -d "$bin_dir" ]]
then
    echo "Directory $bin_dir doesn't exist"
    exit 1
fi

if [ "$(uname)" == "Darwin" ]
then
    find="gfind"
elif [ "$(uname -s)" == "Linux" ]
then
    find="find"
fi

# Clean up all temporary profraw files
$find "$bin_dir" -type f -name "*.profraw" -exec rm {} \;

if [ "$hw_testing" = "1" ]
then
    binaries=$($find "$bin_dir" -type f -name "*.out" -path "*hw*" | sort)
else
    binaries=$($find "$bin_dir" -type f -name "*.out" -not -path "*hw*" | sort)
fi

declare -i result=0

export LLVM_PROFILE_FILE="$bin_dir/test-%p.profraw"

for test in $binaries
do
    if [ "$hw_testing" == "1" ]
    then
        board=$(echo "$test" | cut -d/ -f3)

        if [[ -f ../config/hw-test/"$board".yml ]]
        then
            $test
            result+=$?
        fi
    else
        $test
        result+=$?
    fi
done

exit $result