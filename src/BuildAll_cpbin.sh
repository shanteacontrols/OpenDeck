#!/bin/bash

#exit on error
set -e

mkdir -p ../bin/compiled
mkdir -p ../bin/compiled/opendeck
mkdir -p ../bin/compiled/arduino+teensy
mkdir -p ../bin/compiled/arduino+teensy/fw
mkdir -p ../bin/compiled/arduino+teensy/fw+boot

make clean && make fw_opendeck && cp build/fw_opendeck.hex ../bin/compiled/opendeck/fw_opendeck.hex && cp build/fw_opendeck.bin ../bin/compiled/opendeck/FLASH.bin

make clean && make fw_pro_micro && cp build/fw_pro_micro.hex ../bin/compiled/arduino+teensy/fw/fw_pro_micro.hex
make clean && make fw_leonardo && cp build/fw_leonardo.hex ../bin/compiled/arduino+teensy/fw/fw_leonardo.hex
make clean && make fw_uno && cp build/fw_uno.hex ../bin/compiled/arduino+teensy/fw/fw_uno.hex
make clean && make fw_mega && cp build/fw_mega.hex ../bin/compiled/arduino+teensy/fw/fw_mega.hex
make clean && make fw_teensy2pp && cp build/fw_teensy2pp.hex ../bin/compiled/arduino+teensy/fw/fw_teensy2pp.hex
make clean && make fw_16u2 && cp build/fw_16u2.hex ../bin/compiled/arduino+teensy/fw/fw_16u2.hex
make clean && make fw_8u2 && cp build/fw_8u2.hex ../bin/compiled/arduino+teensy/fw/fw_8u2.hex

. ./combine_fwbtldr leonardo
cp build/fw_leonardo.hex ../bin/compiled/arduino+teensy/fw+boot/fw_boot_leonardo.hex
. ./combine_fwbtldr pro_micro
cp build/fw_pro_micro.hex ../bin/compiled/arduino+teensy/fw+boot/fw_boot_pro_micro.hex
. ./combine_fwbtldr uno
cp build/fw_uno.hex ../bin/compiled/arduino+teensy/fw+boot/fw_boot_uno.hex
. ./combine_fwbtldr mega
cp build/fw_mega.hex ../bin/compiled/arduino+teensy/fw+boot/fw_boot_mega.hex
. ./combine_fwbtldr teensy2pp
cp build/fw_teensy2pp.hex ../bin/compiled/arduino+teensy/fw+boot/fw_boot_teensy2pp.hex
. ./combine_fwbtldr 16u2 MEGA
cp build/fw_16u2.hex ../bin/compiled/arduino+teensy/fw+boot/fw_boot_16u2_mega.hex
. ./combine_fwbtldr 16u2 UNO
cp build/fw_16u2.hex ../bin/compiled/arduino+teensy/fw+boot/fw_boot_16u2_uno.hex

make clean && make fw_tannin
make clean && make fw_kodama
make clean && make fw_bergamot
make clean && make boot_opendeck
make clean && make boot_leonardo
make clean && make boot_teensy2pp
make clean && make boot_kodama
make clean && make boot_bergamot