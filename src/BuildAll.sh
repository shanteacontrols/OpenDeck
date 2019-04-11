#!/bin/bash

#exit on error
set -e

#build all possible firmare/bootloader binaries
targets=targets.txt

while IFS= read -r line || [[ -n $line ]]
do
    make clean && make TARGETNAME=$line
done < "$targets"