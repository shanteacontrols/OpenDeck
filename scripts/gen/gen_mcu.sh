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
core_yaml_file=$(make --no-print-directory -C "$WORKSPACE_DIR"/modules/libcore MCU="$mcu" print-MCU_YML_FILE)
out_cmakelists="$gen_dir"/CMakeLists.txt
project_yaml_file=${script_dir}/../../config/mcu/$mcu.yml
cmake_mcu_defines_var=PROJECT_MCU_DEFINES
cmake_usb_defines_var=PROJECT_USB_DEFINES

if [[ ! -d $gen_dir ]]
then
    if ! make --no-print-directory -C "$WORKSPACE_DIR"/modules/libcore MCU="$mcu" MCU_GEN_DIR="$gen_dir" MCU_GEN_USB=1 MCU_EXT_CLOCK_MHZ="$extClockMhz"
    then
        exit 1
    fi

    if [[ $mcu != "stub" ]]
    then
        echo "Generating project-specific configuration for MCU: $mcu"
        source "$script_dir"/mcu/main.sh
    fi
fi