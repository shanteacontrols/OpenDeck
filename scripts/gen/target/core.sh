#!/usr/bin/env bash

mcu=$($yaml_parser "$yaml_file" mcu)
target_name=$(basename "$yaml_file" .yml)

# MCU processing first
mcu_gen_dir=$(dirname "$gen_dir")/../mcu/$mcu
mcu_yaml_file=$(dirname "$yaml_file")/../mcu/$mcu.yml
hw_test_yaml_file=$(dirname "$yaml_file")/../hw-test/$target_name.yml

if [[ ! -f $mcu_yaml_file ]]
then
    echo "$mcu_yaml_file doesn't exist"
    exit 1
fi

if [[ ! -d $mcu_gen_dir ]]
then
    core_mcu_config_path=$(find "$script_dir"/../../modules/core/src -type f -name "*$mcu.yml")

    if ! "$script_dir"/gen_mcu.sh "$core_mcu_config_path" "$mcu_yaml_file" "$mcu_gen_dir" "$extClockMhz"
    then
        exit 1
    fi
fi

if [[ -f $hw_test_yaml_file ]]
then
    # HW config files go into the same dir as target ones
    if ! "$script_dir"/gen_hwconfig.sh "$project" "$hw_test_yaml_file" "$gen_dir"
    then
        exit 1
    else
        printf "%s\n" "DEFINES += TESTS_HW_SUPPORT" >> "$out_makefile"
    fi
fi

{
    printf "%s%s\n" '-include $(MAKEFILE_INCLUDE_PREFIX)$(BOARD_GEN_DIR_MCU_BASE)/' "$mcu/MCU.mk"
} >> "$out_makefile"

# Normally, ext clock freq is generated in MCU.mk file by core module, however, various OpenDeck boards
# can use the same MCU with different clock speed. Therefore, define this in target makefile.
extClockMhz=$($yaml_parser "$yaml_file" extClockMhz)

if [[ "$extClockMhz" != "null" ]]
then
    printf "%s\n" "DEFINES += CORE_MCU_EXT_CLOCK_FREQ_MHZ=$extClockMhz" >> "$out_makefile"
fi

# If CORE_MCU_CPU_FREQ_MHZ symbol isn't defined, use external clock frequency as core frequency
{
    printf "%s\n" 'ifeq (,$(findstring CORE_MCU_CPU_FREQ_MHZ,$(DEFINES)))'
    printf "%s\n" "DEFINES += CORE_MCU_CPU_FREQ_MHZ=$extClockMhz"
    printf "%s\n" "endif"
} >> "$out_makefile"

board_name=$($yaml_parser "$yaml_file" boardNameOverride)

if [[ $board_name == "null" ]]
then
    board_name=$target_name
fi

printf "%s\n" "#define BOARD_STRING \"$board_name\"" >> "$out_header"
printf "%s\n" "DEFINES += FW_UID=$($script_dir/gen_fw_uid.sh "$target_name")" >> "$out_makefile"