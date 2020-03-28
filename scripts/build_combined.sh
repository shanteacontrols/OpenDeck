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

TARGET=$1

boot_target=${TARGET/fw_/boot_}
fw_target=${TARGET/boot_/fw_}
board=${fw_target/fw_/}

#bootloader target might not exist - verify
boot_dir=$(make TARGETNAME="$boot_target" print-BUILD_DIR)
result=$?

if [[ ($result -ne 0) ]]
then
    exit 1
fi

fw_dir=$(make TARGETNAME="$fw_target" print-BUILD_DIR)

#always build release fw type when running this script
combined_dir=$(make print-BUILD_DIR_BASE)/merged_$board/release
combined_filename="$combined_dir"/merged_"$board".hex

#build bootloader
echo Building "$boot_target"...
make TARGETNAME="$boot_target"

#build firmware
echo Building "$fw_target"...
make TARGETNAME="$fw_target"

echo Merging bootloader and firmware...
mkdir -p "$combined_dir"
srec_cat "$fw_dir"/"$fw_target".hex -intel "$boot_dir"/"$boot_target".hex -Intel -o "$combined_filename" -Intel
echo Firmware merged: "$combined_filename"