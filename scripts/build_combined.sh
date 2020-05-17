#!/bin/bash

run_dir="src"

if [[ $(basename "$(pwd)") != "$run_dir" ]]
then
    echo This script must be run from $run_dir directory!
    exit 1
fi

function usage
{
    echo -e "Used to create hex file containing both the bootloader and application\n"
    echo -e "Usage: ./$(basename "$0") target\n"
    echo -e "target    \tBoard and firmware type to compile. See targets.json for list of supported targets."
}

if [[ ("$*" == "--help") || ($# -eq 0) ]]
then
    usage
    exit 1
fi

if [[ "$(command -v make)" == "" ]]
then
    echo "ERROR: make not installed"
    exit 1
fi

if [[ "$(command -v srec_cat)" == "" ]]
then
    echo "ERROR: srecord not installed"
    exit 1
fi

set -e

#always build release variant
echo Building application for "$1"...
fw_dir=$(make TARGETNAME="$1" BOOT=0 DEBUG=0 print-BUILD_DIR)
make TARGETNAME="$1" BOOT=0 DEBUG=0

echo Building bootloader for "$1"...
boot_dir=$(make TARGETNAME="$1" BOOT=1 DEBUG=0 print-BUILD_DIR)
make TARGETNAME="$1" BOOT=1 DEBUG=0

echo Merging bootloader and firmware...
combined_dir=$(make print-BUILD_DIR_BASE)/merged
mkdir -p "$combined_dir"

srec_cat "$fw_dir"/"$1".hex -intel "$boot_dir"/"$1".hex -Intel -o "$combined_dir"/"$1".hex -Intel

echo Firmware merged: "$combined_dir"/"$1".hex