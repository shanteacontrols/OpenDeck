#!/usr/bin/env bash

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
project=$1
yaml_file=$2
gen_dir=$3
yaml_parser="dasel -n -p yaml --plain -f"
out_header="$gen_dir"/Target.h
out_makefile="$gen_dir"/Makefile

echo "Generating target definitions..."

mkdir -p "$gen_dir"
echo "" > "$out_header"
echo "" > "$out_makefile"

{
    printf "%s\n\n" "#pragma once"
    printf "%s\n" "#include \"board/Internal.h\""
    printf "%s\n" "#include \"core/src/MCU.h\""
    printf "%s\n" "#include \"core/src/arch/common/UART.h\""
} >> "$out_header"

source "$script_dir"/target/core.sh
source "$script_dir"/target/peripherals.sh
source "$script_dir"/target/digital_inputs.sh
source "$script_dir"/target/digital_outputs.sh
source "$script_dir"/target/analog_inputs.sh
source "$script_dir"/target/unused_io.sh

printf "\n%s" "#include \"board/common/Map.h.include\"" >> "$out_header"