#!/usr/bin/env bash

# core module already provides defines and Makefile via mcu Makefile target.
# This script is used to provide additional project-specific
# configuration on top of base configuration if required.

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
mcu=$1
core_yaml_file=$(make --no-print-directory -C "$script_dir"/../../modules/core/src/core MCU="$mcu" print-MCU_YML_FILE)
project_yaml_file=${script_dir}/../../config/mcu/$mcu.yml
gen_dir=$2
extClockMhz=$3
yaml_parser="dasel -n -p yaml --plain -f"
out_header="$gen_dir"/MCU.h
out_makefile="$gen_dir"/MCU.mk

if ! make --no-print-directory -C "$script_dir"/../../modules/core/src/core MCU="$mcu" MCU_GEN_DIR="$gen_dir" MCU_GEN_USB=1 MCU_EXT_CLOCK_MHZ="$extClockMhz"
then
    exit 1
fi

echo "Generating MCU configuration..."

mkdir -p "$gen_dir"

source "$script_dir"/mcu/main.sh