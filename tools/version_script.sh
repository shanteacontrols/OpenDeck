#!/bin/bash

#read MAJOR file
major=`cat ../version/MAJOR`

#read MINOR file
minor=`cat ../version/MINOR`

#read REVISION
revision=`cat ../version/REVISION`

#output $major, $minor and $revision into separate files
echo "software version: $major.$minor.$revision"
echo "$major,$minor,$revision" > version

srec_cat FLASH.hex --intel --fill 0xFF 0x0000 0x67FE -b-e-crc16 0x67FE -XMODEM -o FLASH.bin -binary
rm FLASH.hex