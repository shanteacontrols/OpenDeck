#!/usr/bin/env bash

{
    printf "%s\n\n" "#pragma once"
    printf "%s\n" "#include \"board/src/Internal.h\""
    printf "%s\n" "#include \"core/MCU.h\""
    printf "%s\n" "#include \"core/arch/common/UART.h\""
    printf "%s\n" "#include \"core/arch/common/I2C.h\""
} >> "$out_header"

{
    printf "%s%s\n" '-include $(MCU_GEN_DIR_BASE)/' "$mcu/CoreMCUGenerated.mk"
} >> "$out_makefile"

target_name_string=$($yaml_parser "$yaml_file" targetNameOverride)

if [[ $target_name_string == "null" ]]
then
    target_name_string=$target_name
fi

{
    printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_NAME=\\\"$target_name_string\\\""
    printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_UID=$($script_dir/gen_fw_uid.sh "$target_name")"
} >> "$out_makefile"

for FILE in "$script_dir"/target/*
do
    if [[ "$FILE" != *"$(basename "$BASH_SOURCE")"* ]]
    then
        source "$FILE"
    fi
done

printf "\n%s" "#include \"board/src/common/Map.h.include\"" >> "$out_header"