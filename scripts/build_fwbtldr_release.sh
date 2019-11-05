#!/bin/bash

#note: must be run in src in order to work

#exit on error
set -e

#build only the binaries which are part of the release

mkdir -p ../bin/compiled
mkdir -p ../bin/compiled/opendeck
mkdir -p ../bin/compiled/arduino+teensy
mkdir -p ../bin/compiled/arduino+teensy/fw
mkdir -p ../bin/compiled/arduino+teensy/fw+boot

make clean && make TARGETNAME=fw_opendeck && cp build/fw_opendeck.hex ../bin/compiled/opendeck/fw_opendeck.hex && cp build/fw_opendeck.bin ../bin/compiled/opendeck/FLASH.bin

make clean && make TARGETNAME=fw_promicro && cp build/fw_promicro.hex ../bin/compiled/arduino+teensy/fw/fw_promicro.hex
make clean && make TARGETNAME=fw_leonardo && cp build/fw_leonardo.hex ../bin/compiled/arduino+teensy/fw/fw_leonardo.hex
make clean && make TARGETNAME=fw_uno && cp build/fw_uno.hex ../bin/compiled/arduino+teensy/fw/fw_uno.hex
make clean && make TARGETNAME=fw_mega && cp build/fw_mega.hex ../bin/compiled/arduino+teensy/fw/fw_mega.hex
make clean && make TARGETNAME=fw_mega6mux && cp build/fw_mega6mux.hex ../bin/compiled/arduino+teensy/fw/fw_mega6mux.hex
make clean && make TARGETNAME=fw_teensy2pp && cp build/fw_teensy2pp.hex ../bin/compiled/arduino+teensy/fw/fw_teensy2pp.hex
make clean && make TARGETNAME=fw_16u2 && cp build/fw_16u2.hex ../bin/compiled/arduino+teensy/fw/fw_16u2.hex
make clean && make TARGETNAME=fw_8u2 && cp build/fw_8u2.hex ../bin/compiled/arduino+teensy/fw/fw_8u2.hex

../scripts/build_fwbtldr_single_combine.sh leonardo
cp build/fw_leonardo.hex ../bin/compiled/arduino+teensy/fw+boot/fw_boot_leonardo.hex

../scripts/build_fwbtldr_single_combine.sh promicro
cp build/fw_promicro.hex ../bin/compiled/arduino+teensy/fw+boot/fw_boot_promicro.hex

../scripts/build_fwbtldr_single_combine.sh uno
cp build/fw_uno.hex ../bin/compiled/arduino+teensy/fw+boot/fw_boot_uno.hex

../scripts/build_fwbtldr_single_combine.sh mega
cp build/fw_mega.hex ../bin/compiled/arduino+teensy/fw+boot/fw_boot_mega.hex

../scripts/build_fwbtldr_single_combine.sh mega6mux
cp build/fw_mega6mux.hex ../bin/compiled/arduino+teensy/fw+boot/fw_boot_mega6mux.hex

../scripts/build_fwbtldr_single_combine.sh teensy2pp
cp build/fw_teensy2pp.hex ../bin/compiled/arduino+teensy/fw+boot/fw_boot_teensy2pp.hex

../scripts/build_fwbtldr_single_combine.sh 16u2 MEGA
cp build/fw_16u2.hex ../bin/compiled/arduino+teensy/fw+boot/fw_boot_16u2_mega.hex

../scripts/build_fwbtldr_single_combine.sh 16u2 UNO
cp build/fw_16u2.hex ../bin/compiled/arduino+teensy/fw+boot/fw_boot_16u2_uno.hex