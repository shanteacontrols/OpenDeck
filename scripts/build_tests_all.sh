#!/bin/bash

#exit on error
set -e

#build all possible firmare/bootloader binaries
targets=targets.txt

make clean-all
make pre-build

while IFS= read -r line || [[ -n $line ]]
do
    printf "\n***Building tests for $line***\n"
    make TARGETNAME=$line
done < "$targets"