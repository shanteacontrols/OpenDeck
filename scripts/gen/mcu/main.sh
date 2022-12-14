#!/usr/bin/env bash

{
    printf "%s%s\n" '-include $(BOARD_GEN_DIR_MCU_BASE)/' "$mcu/CoreMCUGenerated.mk"
} > "$out_makefile"

for FILE in "$script_dir"/mcu/*
do
    if [[ "$FILE" != *"$(basename "$BASH_SOURCE")"* ]]
    then
        source "$FILE"
    fi
done