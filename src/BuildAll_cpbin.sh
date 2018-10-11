#!/bin/bash

#exit on error
set -e

mkdir -p ../bin/compiled
mkdir -p ../bin/compiled/opendeck

make clean && make fw_opendeck && cp build/fw_opendeck.hex ../bin/compiled/opendeck/fw_opendeck.hex && cp build/fw_opendeck.bin ../bin/compiled/opendeck/FLASH.bin
make clean && make fw_pro_micro && cp build/fw_pro_micro.bin ../bin/compiled/fw_pro_micro.bin
make clean && make fw_leonardo && cp build/fw_leonardo.bin ../bin/compiled/fw_leonardo.bin
make clean && make fw_uno && cp build/fw_uno.bin ../bin/compiled/fw_uno.bin
make clean && make fw_mega && cp build/fw_mega.bin ../bin/compiled/fw_mega.bin
make clean && make fw_16u2 && cp build/fw_16u2.bin ../bin/compiled/fw_16u2.bin
make clean && make fw_8u2 && cp build/fw_8u2.bin ../bin/compiled/fw_8u2.bin
make clean && make fw_teensy2pp && cp build/fw_teensy2pp.bin ../bin/compiled/fw_teensy2pp.bin
make clean && make fw_tannin
make clean && make fw_kodama
make clean && make fw_bergamot
make clean && make boot_opendeck
make clean && make boot_leonardo
make clean && make boot_teensy2pp
make clean && make boot_kodama
make clean && make boot_bergamot
