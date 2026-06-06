#!/usr/bin/env bash

set -euo pipefail

function usage
{
    echo -e "\nUsage: ./$(basename "$0") [options]"

    echo -e "
    Generates a merged OpenDeck BIN image for any target.

    Required options:
      --build-dir <sysbuild binary dir>

    Optional:
      --output <merged.bin>
    "
}

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
metadata_query=${script_dir}/query_metadata.sh

build_dir=
output=

while [[ $# -gt 0 ]]
do
    case "$1" in
        --build-dir)
            build_dir=$2
            shift 2
            ;;
        --output)
            output=$2
            shift 2
            ;;
        --help)
            usage
            exit 0
            ;;
        *)
            echo "ERROR: Unknown argument '$1'." >&2
            usage >&2
            exit 1
            ;;
    esac
done

function require_arg
{
    local name=$1
    local value=$2

    if [[ -z "$value" ]]
    then
        echo "ERROR: Missing ${name}." >&2
        usage >&2
        exit 1
    fi
}

function cache_value
{
    local cache_file=$1
    local key=$2

    sed -n "s/^${key}:[^=]*=//p" "$cache_file" | head -n1
}

function config_value
{
    local config_file=$1
    local key=$2

    sed -n "s/^${key}=//p" "$config_file" | head -n1 | sed 's/^"//; s/"$//'
}

function config_is_enabled
{
    local config_file=$1
    local key=$2

    grep -q "^${key}=y$" "$config_file"
}

function partition_offset
{
    local label=$1

    bash "$metadata_query" dts --file "$partition_dts" --key partition_offset --partition-label "$label"
}

require_arg "--build-dir" "$build_dir"

if [[ -z "$output" ]]
then
    output=${build_dir}/merged.bin
fi

app_cache=${build_dir}/app/CMakeCache.txt
app_config=${build_dir}/app/zephyr/.config
merged_hex=${build_dir}/merged.hex

require_arg "app CMake cache" "$app_cache"
require_arg "app .config" "$app_config"
require_arg "merged.hex" "$merged_hex"

if [[ ! -f "$app_cache" ]]
then
    echo "ERROR: Missing app CMake cache: ${app_cache}" >&2
    exit 1
fi

if [[ ! -f "$app_config" ]]
then
    echo "ERROR: Missing app .config: ${app_config}" >&2
    exit 1
fi

if [[ ! -f "$merged_hex" ]]
then
    echo "ERROR: Missing merged HEX image: ${merged_hex}" >&2
    exit 1
fi

if config_is_enabled "$app_config" CONFIG_SOC_FAMILY_ESPRESSIF_ESP32
then
    python=$(cache_value "${build_dir}/CMakeCache.txt" _Python3_EXECUTABLE)
    chip=$(config_value "$app_config" CONFIG_SOC)
    partition_dts=${build_dir}/mcuboot/zephyr/zephyr.dts
    mcuboot_bin=$(cache_value "${build_dir}/mcuboot/CMakeCache.txt" BYPRODUCT_KERNEL_BIN_NAME)
    bootloader_bin=$(cache_value "${build_dir}/opendeck_bootloader/CMakeCache.txt" BYPRODUCT_KERNEL_SIGNED_BIN_NAME)
    app_bin=$(cache_value "$app_cache" BYPRODUCT_KERNEL_SIGNED_BIN_NAME)

    require_arg "Python executable" "$python"
    require_arg "ESP chip" "$chip"
    require_arg "MCUboot devicetree" "$partition_dts"
    require_arg "MCUboot BIN" "$mcuboot_bin"
    require_arg "OpenDeck bootloader BIN" "$bootloader_bin"
    require_arg "application BIN" "$app_bin"

    "$python" -m esptool \
        --chip "$chip" \
        merge-bin \
        --format raw \
        --target-offset 0x0 \
        --output "$output" \
        "$(partition_offset boot)" "$mcuboot_bin" \
        "$(partition_offset image-1)" "$bootloader_bin" \
        "$(partition_offset image-0)" "$app_bin"
else
    objcopy=$(cache_value "$app_cache" CMAKE_OBJCOPY)
    require_arg "objcopy" "$objcopy"

    "$objcopy" \
        -I ihex \
        --gap-fill 0xFF \
        -O binary \
        "$merged_hex" \
        "$output"
fi
