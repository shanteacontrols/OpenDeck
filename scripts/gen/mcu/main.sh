#!/usr/bin/env bash

for FILE in "$script_dir"/mcu/*
do
    if [[ "$FILE" != *"$(basename "$BASH_SOURCE")"* ]]
    then
        source "$FILE"
    fi
done