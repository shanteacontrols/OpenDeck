#!/bin/bash

run_dir="OpenDeck"

if [[ $(basename "$(pwd)") != "$run_dir"* ]]
then
    echo This script must be run from $run_dir directory!
    exit 1
fi

if [ "$(uname)" == "Darwin" ]; then
    dfu="bin/dfu/hid_bootloader_loader_mac"
elif [ "$(uname -s)" == "Linux" ]
then
    dfu="bin/dfu/hid_bootloader_loader_linux"
fi

echo "Info: In case of errors please run this script with super-user rights."
echo "Please select the board for which you want to update firmware:"

boards=$(find bin/compiled/fw -type f -name "*.hex" -exec basename {} .hex \; | sort)
echo "$boards" | tr " " "\n" | cat -n
printf "Board number: "
read -r board_nr

filename=$(echo "$boards" | head -n "$board_nr" | tail -n 1)
path=$(find bin/compiled/fw -type f -name "${filename}.hex")
mcu=$(echo "$path" | cut -d / -f5)

"$dfu" "$mcu" "$path"