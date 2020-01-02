#!/bin/bash

function usage
{
    echo -e "Used to create hex file containing both the bootloader and application\n"
    echo -e "Usage: ./`basename "$0"` --target [--variant]\n"
    echo -e "  --target=    \tBoard and firmware type to compile. See targets.txt for list of supported targets. Prefix in target is ignored (fw_ and boot_)"
}

if [[ ("$*" == "--help") || ($# -eq 0) ]]
then
    usage
    exit 1
fi

for i in $*; do
    case "$i" in
        --target=*)
            TARGET=${i#--target=}
            ;;
    esac
done

VARIANT=$2

boot_target=${TARGET/fw_/boot_}
fw_target=${TARGET/boot_/fw_}

make clean

boot_dir=$(make TARGETNAME=$boot_target $VARIANT print-BUILD_DIR)
fw_dir=$(make TARGETNAME=$fw_target $VARIANT print-BUILD_DIR)

#build bootloader
make TARGETNAME=$boot_target $2

#build firmware
make TARGETNAME=$fw_target $2

#merge fw and boot hex and copy resulting file to build directory so that make upload command can work immediately after this script
srec_cat $fw_dir/$fw_target.hex -intel $boot_dir/$boot_target.hex -Intel -o $fw_dir/$fw_target.hex -Intel