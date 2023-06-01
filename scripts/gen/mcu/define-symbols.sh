#!/usr/bin/env bash

if [[ $($yaml_parser "$project_yaml_file" define-symbols) != "null" ]]
then
    total_symbols=$($yaml_parser "$project_yaml_file" define-symbols --length)

    for ((i=0;i<total_symbols;i++))
    do
        symbol=$($yaml_parser "$project_yaml_file" define-symbols.["$i"])
        printf "%s\n" "list(APPEND $cmake_mcu_defines_var $symbol)" >> "$out_cmakelists"
    done
fi