#!/bin/bash

run_dir="OpenDeck"

if [[ $(basename "$(pwd)") != "$run_dir"* ]]
then
    echo This script must be run from $run_dir directory!
    exit 1
fi

if [[ "$(command -v avrdude)" == "" ]]
then
    echo "ERROR: avrdude not installed"
    exit 1
fi

if [ "$(uname)" == "Darwin" ]
then
    find="gfind"

    if [[ "$(command -v gfind)" == "" ]]
    then
        echo "ERROR: GNU find not installed (gfind)"
        exit 1
    fi

    port_list=$($find /dev -name "cu.*" -ls 2>/dev/null | sort | cut -d / -f3 | grep -v Bluetooth)
elif [ "$(uname -s)" == "Linux" ]
then
    find="find"
    port_list=$($find /dev/serial/by-id/ -type l -ls | grep -Eo '\btty\w+')
fi

if [[ "$port_list" == "" ]]
then
    echo "ERROR: No ports found. Please connect ArduinoISP before running the script."
    exit 1
fi

echo "Please select serial port on which ArduinoISP is connected:"
echo "$port_list" | cat -n
read -r port
port=$(echo "$port_list" | head -n "$port" | tail -n 1)

echo "Please select board you want to flash and then press enter:"

boards=$($find bin/compiled -type f -name "*.hex" -path "*merged/avr*" -printf '%f\n' | sort)
echo "$boards" | cut -d . -f1 | cat -n
printf "Board number: "
read -r board_nr

echo "Please wait..."

filename=$(echo "$boards" | head -n "$board_nr" | tail -n 1)
mcu=$($find bin/compiled -type f -name "*$filename" -path "*merged/avr*" | cut -d/ -f5)
path=$($find bin/compiled/merged -type f -name "$filename")

unlock_fuse=$(command < src/board/avr/variants/avr8/"$mcu"/fuses.txt grep ^unlock= | cut -d= -f2)
lock_fuse=$(command < src/board/avr/variants/avr8/"$mcu"/fuses.txt grep ^lock= | cut -d= -f2)
ext_fuse=$(command < src/board/avr/variants/avr8/"$mcu"/fuses.txt grep ^ext= | cut -d= -f2)
low_fuse=$(command < src/board/avr/variants/avr8/"$mcu"/fuses.txt grep ^low= | cut -d= -f2)
high_fuse=$(command < src/board/avr/variants/avr8/"$mcu"/fuses.txt grep ^high= | cut -d= -f2)

echo "Connect programmer to programming header on the board and then press enter."
read -rn1

avrdude -p "$mcu" -P /dev/"$port" -b 19200 -c avrisp -e -V -u -U lock:w:"$unlock_fuse":m -U efuse:w:"$ext_fuse":m -U hfuse:w:"$high_fuse":m -U lfuse:w:"$low_fuse":m
avrdude -p "$mcu" -P /dev/"$port" -b 19200 -c avrisp -U flash:w:"$path"
avrdude -p "$mcu" -P /dev/"$port" -b 19200 -c avrisp -V -u -U lock:w:"$lock_fuse":m