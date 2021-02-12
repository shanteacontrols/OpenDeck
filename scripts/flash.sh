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

echo "Please type the serial port on which ArduinoISP is connected, without /dev/ part:"
read -r port

echo "Please select AVR MCU you want to flash and then press enter:"

boards=$(find src/board/avr/variants/avr8 -mindepth 1 -type d | awk -F/ '{ print $NF }')
echo "$boards" | cut -d . -f1 | cat -n
printf "Board number: "
read -r board_nr

echo "Please wait..."

mcu=$(echo "$boards" | head -n "$board_nr" | tail -n 1)

unlock_fuse=$(command < src/board/avr/variants/avr8/"$mcu"/fuses.txt grep ^unlock= | cut -d= -f2)
lock_fuse=$(command < src/board/avr/variants/avr8/"$mcu"/fuses.txt grep ^lock= | cut -d= -f2)
ext_fuse=$(command < src/board/avr/variants/avr8/"$mcu"/fuses.txt grep ^ext= | cut -d= -f2)
low_fuse=$(command < src/board/avr/variants/avr8/"$mcu"/fuses.txt grep ^low= | cut -d= -f2)
high_fuse=$(command < src/board/avr/variants/avr8/"$mcu"/fuses.txt grep ^high= | cut -d= -f2)

echo "Type the path to the binary you want to flash"
read -r path

echo "Connect programmer to programming header on the board and then press enter."
read -rn1

avrdude -p "$mcu" -P /dev/"$port" -b 19200 -c avrisp -e -V -u -U lock:w:"$unlock_fuse":m -U efuse:w:"$ext_fuse":m -U hfuse:w:"$high_fuse":m -U lfuse:w:"$low_fuse":m
avrdude -p "$mcu" -P /dev/"$port" -b 19200 -c avrisp -U flash:w:"$path"
avrdude -p "$mcu" -P /dev/"$port" -b 19200 -c avrisp -V -u -U lock:w:"$lock_fuse":m