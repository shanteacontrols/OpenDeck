#!/usr/bin/env bash

run_dir="OpenDeck"

if [[ $(pwd) != *"$run_dir" ]]
then
    echo This script must be run from $run_dir directory!
    exit 1
fi

if [[ "$(command -v avrdude)" == "" ]]
then
    echo "ERROR: avrdude not installed"
    exit 1
fi

echo -n "Please type the serial port on which ArduinoISP is connected, without /dev/ part.
Port name usually has the following pattern, where X is number representing the port:
    * ttyACMX (or ttyUSBX)
If unsure, open Arduino IDE and check where your Arduino as an programmer is connected to under Tools/Port.
Port: "

read -r port

echo "Please select AVR MCU you want to flash and then press enter:"

mcus=$(find config/mcu -type f -name "at*" -exec basename -s '.yml' {} +)
echo "$mcus" | cat -n

echo -n "MCU number: "
read -r mcu_nr

echo "
Please wait...
"

mcu=$(echo "$mcus" | head -n "$mcu_nr" | tail -n 1)

# Don't use yaml parser (dasel) here so that this script can be run without any external tools
unlock_fuse=$(< config/mcu/"$mcu".yml awk '{$1=$1};1' | grep ^unlock | cut -d: -f2 | xargs)
lock_fuse=$(< config/mcu/"$mcu".yml awk '{$1=$1};1' | grep ^lock | cut -d: -f2 | xargs)
ext_fuse=$(< config/mcu/"$mcu".yml awk '{$1=$1};1' | grep ^ext | cut -d: -f2 | xargs)
low_fuse=$(< config/mcu/"$mcu".yml awk '{$1=$1};1' | grep ^low | cut -d: -f2 | xargs)
high_fuse=$(< config/mcu/"$mcu".yml awk '{$1=$1};1' | grep ^high | cut -d: -f2 | xargs)

echo -n "Type the path (location) of the binary you want to flash.
If you want to flash official binary from OpenDeck repository, go to the following link and
download appropriate .bin file: https://github.com/shanteacontrols/OpenDeck/releases
.bin file can be found in release assets.

Tip: you can also drag .bin file to the terminal - path to the binary will be printed.
Path: "

read -r path

# Remove single quotes if present
path=${path//\'/}

echo "Connect programmer to programming header on the board and then press enter."
read -rn1

avrdude -p "$mcu" -P /dev/"$port" -b 19200 -c avrisp -C /etc/avrdude.conf -e -V -u -U lock:w:"$unlock_fuse":m -U efuse:w:"$ext_fuse":m -U hfuse:w:"$high_fuse":m -U lfuse:w:"$low_fuse":m
avrdude -p "$mcu" -P /dev/"$port" -b 19200 -c avrisp -C /etc/avrdude.conf -U flash:w:"$path"
avrdude -p "$mcu" -P /dev/"$port" -b 19200 -c avrisp -C /etc/avrdude.conf -V -u -U lock:w:"$lock_fuse":m