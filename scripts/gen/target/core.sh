#!/usr/bin/env bash

mcu=$($yaml_parser "$yaml_file" mcu)
target_name=$(basename "$yaml_file" .yml)

# MCU processing first
mcu_gen_dir=$(dirname "$2")/../mcu/$mcu
mcu_yaml_file=$(dirname "$yaml_file")/../mcu/$mcu.yml
hw_test_yaml_file=$(dirname "$yaml_file")/../hw-test/$target_name.yml

if [[ ! -f $mcu_yaml_file ]]
then
    echo "$mcu_yaml_file doesn't exist"
    exit 1
fi

if [[ ! -d $mcu_gen_dir ]]
then
    if ! "$script_dir"/gen_mcu.sh "$mcu_yaml_file" "$mcu_gen_dir"
    then
        exit 1
    fi
fi

if [[ -f $hw_test_yaml_file ]]
then
    # hw config files go into the same dir as target ones
    if ! "$script_dir"/gen_hwconfig.sh "$hw_test_yaml_file" "$gen_dir"
    then
        exit 1
    else
        printf "%s\n" "DEFINES += HW_TESTS_SUPPORTED" >> "$out_makefile"
    fi
fi

{
    printf "%s%s\n" '-include $(MAKEFILE_INCLUDE_PREFIX)$(GEN_DIR_MCU_BASE)/' "$mcu/MCU.mk"
    printf "%s\n" "DEFINES += FW_UID=$($script_dir/gen_fw_uid.sh "$target_name")"
} >> "$out_makefile"

hse_val=$($yaml_parser "$yaml_file" extClockMhz)

if [[ $hse_val != "null" ]]
then
    printf "%s%s\n" "DEFINES += HSE_VALUE=$hse_val" "000000" >> "$out_makefile"
fi

board_name=$($yaml_parser "$yaml_file" boardNameOverride)

if [[ $board_name == "null" ]]
then
    board_name=$target_name
fi

printf "%s\n" "DEFINES += BOARD_STRING=\\\"$board_name\\\"" >> "$out_makefile"