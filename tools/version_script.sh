#!/bin/bash

srec_cat FLASH.hex --intel --fill 0xFF 0x0000 0x6800 -o FLASH.bin -binary
rm FLASH.hex