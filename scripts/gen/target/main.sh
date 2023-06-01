#!/usr/bin/env bash

{
    printf "%s\n\n" "#pragma once"
    printf "%s\n" "#include \"Internal.h\""
    printf "%s\n" "#include \"core/MCU.h\""
    printf "%s\n" "#include \"core/arch/common/UART.h\""
    printf "%s\n" "#include \"core/arch/common/I2C.h\""
} >> "$out_header"

target_name_string=$($yaml_parser "$yaml_file" targetNameOverride)

if [[ $target_name_string == "null" ]]
then
    target_name_string=$target_name
fi

{
    printf "%s\n" "include(${mcu_gen_dir}/CMakeLists.txt)"
    printf "%s\n" "set(PROJECT_TARGET_MCU ${mcu})"
    printf "%s\n" "set(${cmake_defines_var} \"\")"
    printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_NAME=\\\"$target_name_string\\\")"
    printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_UID=$($script_dir/gen_fw_uid.sh "$target_name"))"
}  > "$out_cmakelists"

for FILE in "$script_dir"/target/*
do
    if [[ "$FILE" != *"$(basename "$BASH_SOURCE")"* ]]
    then
        source "$FILE"
    fi
done

printf "\n%s" "#include \"common/Map.h.include\"" >> "$out_header"