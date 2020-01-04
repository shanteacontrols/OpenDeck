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
    port_list=$(ls /dev/cu.* | sort | cut -d / -f3 | grep -v Bluetooth)
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    grep=grep
    find=find
    port_list=$(ls /dev/tty.* | sort | cut -d / -f3)
fi

echo "Please select serial port on which ArduinoISP is connected:"
echo $port_list | cat -n
read port
port=$(echo "$port_list" | head -n $port | tail -n 1)

echo "Please select board you want to flash and then press enter:"

boards=$($find bin/compiled -type f -name "*.hex" -path "*fw_boot*" -printf '%f\n' | sort)
echo "$boards" | cut -d . -f1 | cat -n
printf "Board number: "
read board_nr

filename=$(echo "$boards" | head -n $board_nr | tail -n 1)
make_target=fw_$(echo $filename | cut -d . -f1)
path=$($find bin/compiled/fw_boot -type f -name $filename)

unlock_fuse=$(make -C src TARGETNAME=$make_target print-FUSE_UNLOCK)

#specified target might not exist - verify
if [[ ($? -ne 0) ]]
then
    exit 1
fi

lock_fuse=$(make -C src TARGETNAME=$make_target print-FUSE_LOCK)
low_fuse=$(make -C src TARGETNAME=$make_target print-FUSE_LOW)
high_fuse=$(make -C src TARGETNAME=$make_target print-FUSE_HIGH)
ext_fuse=$(make -C src TARGETNAME=$make_target print-FUSE_EXT)

mcu=$(make -C src TARGETNAME=$make_target print-MCU)

echo "Connect programmer to programming header on the board and then press enter."
read -n1 KEY

avrdude -p $mcu -P /dev/$port -b 19200 -c avrisp -e -V -u -U lock:w:$unlock_fuse:m -U efuse:w:$ext_fuse:m -U hfuse:w:$high_fuse:m -U lfuse:w:$low_fuse:m
avrdude -p $mcu -P /dev/$port -b 19200 -c avrisp -U flash:w:$path
avrdude -p $mcu -P /dev/$port -b 19200 -c avrisp -V -u -U lock:w:$lock_fuse:m