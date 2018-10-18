#!/bin/bash

#exit on error
set -e

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