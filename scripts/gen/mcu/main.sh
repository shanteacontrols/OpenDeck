#!/usr/bin/env bash

# Use dedicated generated CMakeLists.txt file and then include it in
# CMakeLists.txt from project so that the file from core can be optionally
# included (might be required for tests or other tools).
generated_cmakelists_renamed=$gen_dir/core.cmake

mv "$out_cmakelists" "$generated_cmakelists_renamed"

{
    printf "%s\n" "if(CMAKE_USE_STUB_MCU)"
    printf "%s\n" "include($gen_dir/../stub/CMakeLists.txt)"
    printf "%s\n" "else()"
    printf "%s\n" "include($generated_cmakelists_renamed)"
    printf "%s\n" "endif()"
    printf "%s\n" "set(${cmake_mcu_defines_var} \"\")"
    printf "%s\n" "set(${cmake_usb_defines_var} \"\")"
} >> "$out_cmakelists"

for FILE in "$script_dir"/mcu/*
do
    if [[ "$FILE" != *"$(basename "$BASH_SOURCE")"* ]]
    then
        source "$FILE"
    fi
done