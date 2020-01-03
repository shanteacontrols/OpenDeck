#!/bin/bash

#exit on error
set -e

TARGETS=targets.txt

for i in $*; do
    case "$i" in
        --release)
            TARGETS=targets_release.txt
            ;;
    esac
done

make clean

while IFS= read -r line || [[ -n $line ]]
do
    make TARGETNAME=$line
done < "$TARGETS"