#!/bin/bash

#exit on error
set -e

#firmare/bootloader
make clean && make fw_opendeck
make clean && make fw_pro_micro
make clean && make fw_leonardo
make clean && make fw_uno
make clean && make fw_mega
make clean && make fw_16u2
make clean && make fw_8u2
make clean && make fw_teensy2pp
make clean && make fw_tannin
make clean && make fw_kodama
make clean && make fw_bergamot
make clean && make boot_opendeck
make clean && make boot_leonardo
make clean && make boot_teensy2pp
make clean && make boot_kodama
make clean && make boot_bergamot
make clean && make boot_mega
make clean && make boot_uno
make clean && make boot_16u2 VARIANT=VARIANT_MEGA
make clean && make boot_16u2 VARIANT=VARIANT_UNO

#tests
cd tests/
chmod +x runtests.sh
echo "Running tests for OpenDeck board"
make clean && make all BOARD_TYPE=opendeck && make exec
echo "Running tests for Leonardo/Pro Micro board"
make clean && make all BOARD_TYPE=leonardo && make exec
echo "Running tests for Uno board"
make clean && make all BOARD_TYPE=uno && make exec
echo "Running tests for Mega board"
make clean && make all BOARD_TYPE=mega && make exec
echo "Running tests for Teensy++ 2.0 board"
make clean && make all BOARD_TYPE=teensy2pp && make exec
echo "Running tests for Tannin board"
make clean && make all BOARD_TYPE=tannin && make exec
echo "Running tests for Bergamot board"
make clean && make all BOARD_TYPE=bergamot && make exec
echo "Running tests for Kodama board"
make clean && make all BOARD_TYPE=kodama && make exec