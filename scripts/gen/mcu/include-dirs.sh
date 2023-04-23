#!/usr/bin/env bash

if [[ $($yaml_parser "$project_yaml_file" include-dirs) != "null" ]]
then
    total_include_dirs=$($yaml_parser "$project_yaml_file" include-dirs --length)

    for ((i=0;i<total_include_dirs;i++))
    do
        dir=$($yaml_parser "$project_yaml_file" include-dirs.["$i"])
        printf "%s\n" "INCLUDE_DIRS += -I\"${script_dir}/../../$dir\"" >> "$out_makefile"
    done
fi