#!/bin/bash

run_dir="src"

if [[ $(basename "$(pwd)") != "$run_dir" ]]
then
    echo This script must be run from $run_dir directory!
    exit 1
fi

#exit on error
set -e

TARGETS=targets.txt

for i in "$@"; do
    case "$i" in
        --release)
            TARGETS=targets_release.txt
            ;;
    esac
done

make clean

while IFS= read -r line || [[ -n $line ]]
do
    printf '\n%s\n' "***Building $line***"
    make TARGETNAME="$line"
done < "$TARGETS"