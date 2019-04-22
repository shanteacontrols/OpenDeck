#!/bin/bash

#used to create hex file containing both the bootloader and application
#example for opendeck: ./build_single_combine_fw_btldr.sh opendeck
#variant (second command line argument) can be specified only as "UNO" or "MEGA"
#used only for 16u2 target, ignored otherwise
#example: ./build_single_combine_fw_btldr 16u2 UNO

#build bootloader first
make clean && make TARGETNAME=boot_$1 VARIANT=VARIANT_$2
#copy bootloader hex to current dir to avoid file being deleted after second make clean
cp build/boot_$1.hex boot_$1.hex
#build firmware
make clean && make TARGETNAME=fw_$1
#copy firmware hex to current dir
cp build/fw_$1.hex fw_$1.hex
#delete both hex and bin from build dir
rm build/fw_$1.hex
rm build/fw_$1.bin
#merge fw and boot hex and copy resulting file to build/ so that make upload command can work immediately after this script
srec_cat fw_$1.hex -intel boot_$1.hex -Intel -o build/fw_$1.hex -Intel
#delete temporary files
rm fw_$1.hex boot_$1.hex
