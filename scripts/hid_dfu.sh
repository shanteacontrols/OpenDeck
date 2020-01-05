#!/bin/bash

run_dir="OpenDeck"

if [[ $(basename "`pwd`") != $run_dir ]]
then
    echo This script must be run from $run_dir directory!
    exit 1
fi

if [ "$(uname)" == "Darwin" ]; then
    grep=ggrep
    find=gfind
    dfu=bin/dfu/hid_bootloader_loader_mac
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    grep=grep
    find=find
    dfu=bin/dfu/hid_bootloader_loader_linux
fi

echo "Info: In case of errors please run this script with super-user rights."
echo "Please select the board for which you want to update firmware:"

boards=$($find bin/compiled -type f -not -path "*boot*" -name "*.hex" -printf '%f\n' | cut -f 1 -d '.' | sort)
echo $boards | tr " " "\n" | cat -n
printf "Board number: "
read board_nr

filename=$(echo "$boards" | head -n $board_nr | tail -n 1)
path=$($find -type f -not -path "*boot*" -name "${filename}.hex")
mcu=$(echo $path| cut -d / -f6)

$dfu $mcu $path