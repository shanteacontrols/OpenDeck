#!/usr/bin/env bash

# Device Firmware Update utility

set -e

sudo dfu-util -d 0483:df11 -a 0 -i 0 -s 0x8000000:leave -D "$PWD"/merged.bin