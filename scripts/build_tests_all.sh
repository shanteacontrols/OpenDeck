#!/bin/bash

run_dir="tests"

if [[ $(basename "$(pwd)") != "$run_dir" ]]
then
    echo This script must be run from $run_dir directory!
    exit 1
fi

#exit on error
set -e

#build all possible firmare/bootloader binaries
targets=targets.txt

make clean
make pre-build

while IFS= read -r line || [[ -n $line ]]
do
    printf '\n%s\n' "***Building tests for $line***"
    make TARGETNAME="$line"
done < "$targets"