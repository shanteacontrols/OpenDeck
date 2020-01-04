#!/bin/bash

copy()
{
    for binary in $1
    do
        #to enable usage of fw update script without invoking make,
        #copy binaries to bin/compiled with arch and mcu name in directory names
        make_target=$(echo $binary | cut -d / -f4 | cut -d . -f1)

        #no need to copy firmware for 8u2 and 16u2 variants
        #check this only when copying firmware only
        if [[ $2 == "fw" ]]
        then
            if [[ $make_target == *"u2"* ]]
            then
                continue
            fi
        else
            #in this case received name is fw_boot_<board>
            #drop _boot from name so that make target is valid
            make_target=${make_target/boot_/}
        fi

        board=${make_target/fw_/}
        arch=$(make TARGETNAME=$make_target print-ARCH)
        mcu=$(make TARGETNAME=$make_target print-MCU)

        cp_dir=../bin/compiled/$2/$arch/$mcu
        mkdir -p $cp_dir

        cp $binary $cp_dir/$board.hex
    done
}

#build only the binaries which are part of the release
../scripts/build_targets.sh --release

binaries=$(find build -type f -name "*.hex")
copy "${binaries[@]}" "fw"

#now also copy merged app+bootloader to bin/compiled
cat targets_release.txt | while read line ; do ../scripts/build_combined.sh $line ; done

binaries=$(find build -type f -name "fw_boot*.hex")
copy "${binaries[@]}" "fw_boot"