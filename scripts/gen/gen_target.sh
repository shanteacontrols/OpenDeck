#!/usr/bin/env bash

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
project=$1
yaml_file=$2
gen_dir=$3
yaml_parser="dasel -n -p yaml --plain -f"
out_header="$gen_dir"/Target.h
out_makefile="$gen_dir"/Makefile
mcu=$($yaml_parser "$yaml_file" mcu)
target_name=$(basename "$yaml_file" .yml)
mcu_gen_dir=$(dirname "$gen_dir")/../mcu/$mcu
hw_test_yaml_file=$(dirname "$yaml_file")/../hw-test/$target_name.yml
extClockMhz=$($yaml_parser "$yaml_file" extClockMhz)

mkdir -p "$gen_dir"
echo "" > "$out_header"
echo "" > "$out_makefile"

if [[ ! -d $mcu_gen_dir ]]
then
    if ! "$script_dir"/gen_mcu.sh "$mcu" "$mcu_gen_dir" "$extClockMhz"
    then
        exit 1
    fi
fi

echo "Generating target definitions..."

source "$script_dir"/target/main.sh

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