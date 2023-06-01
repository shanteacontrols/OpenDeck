#!/usr/bin/env bash

# Firmware update via OpenDeck bootloader

set -e

target=$1

if [[ $(amidi -l | grep "OpenDeck DFU | $target") == "" ]]
then
    echo "Error: No OpenDeck DFU interface found"
fi

echo "Sending firmware to device..."
amidi_port=$(amidi -l | grep "OpenDeck DFU | $target" | grep -Eo 'hw:\S*')
amidi -p "$amidi_port" -s "$PWD"/sysexgen/firmware.raw
