#!/usr/bin/env bash

# core module already provides defines and Makefile via mcu Makefile target.
# This script is used to provide additional project-specific
# configuration on top of base configuration if required.

for arg in "$@"; do
    case "$arg" in
        --mcu=*)
            mcu=${arg#--mcu=}
            ;;

        --gen-dir=*)
            gen_dir=${arg#--gen-dir=}
            ;;

        --extClockMhz=*)
            extClockMhz=${arg#--extClockMhz=}
            ;;
    esac
done

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
yaml_parser="dasel -n -p yaml --plain -f"
core_yaml_file=$(make --no-print-directory -C "$script_dir"/../../modules/core MCU="$mcu" print-MCU_YML_FILE)
out_header="$gen_dir"/MCU.h
out_cmakelists="$gen_dir"/CMakeLists.txt
project_yaml_file=${script_dir}/../../config/mcu/$mcu.yml
cmake_mcu_defines_var=PROJECT_MCU_DEFINES
cmake_usb_defines_var=PROJECT_USB_DEFINES

if ! make --no-print-directory -C "$script_dir"/../../modules/core MCU="$mcu" MCU_GEN_DIR="$gen_dir" MCU_GEN_USB=1 MCU_EXT_CLOCK_MHZ="$extClockMhz"
then
    exit 1
fi

echo "Generating project MCU definitions..."

source "$script_dir"/mcu/main.sh