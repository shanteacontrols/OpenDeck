#!/usr/bin/env bash

# Builds the validated app image used by DFU, direct flashing, and boot-time
# selector checks.
#
# The output binary layout is:
#
#   [raw app payload][0xFF padding to 4-byte alignment][validation record]
#
# The validation record is three little-endian uint32_t values:
# magic ("DFUC"), raw payload size, raw payload CRC32.
#
# Optional outputs:
# - --hex-output writes the same validated image as addressed Intel HEX, using
#   the app partition base resolved from Zephyr's generated devicetree data.
# - --info-output writes a tiny C header with the app/SRAM boundaries and ROM
#   start offset used by tests to run selector validation against this image.

set -euo pipefail

script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
payload_file=
output_file=
dts_file=
hex_output_file=
info_output_file=
objcopy=

readonly image_validation_magic=$((0x43554644))
readonly image_validation_record_size=12
readonly erased_byte=$((0xFF))

function parse_int
{
    local value=$1

    if [[ $value == 0x* || $value == 0X* ]]
    then
        printf '%u\n' "$((16#${value:2}))"
    else
        printf '%u\n' "$value"
    fi
}

function usage
{
    printf 'usage: gen_validated_app_image.sh --input=<payload.bin> --output=<output.bin> [--dts=<zephyr.dts> --hex-output=<output.hex> --info-output=<output.h> --objcopy=<objcopy>]\n' >&2
}

# Returns the generated devicetree header next to zephyr.dts.
function generated_header
{
    local dts_file=$1

    printf '%s\n' "${dts_file%/zephyr.dts}/include/generated/zephyr/devicetree_generated.h"
}

# Returns the Kconfig output next to zephyr.dts.
function config_file
{
    local dts_file=$1

    printf '%s\n' "${dts_file%/zephyr.dts}/.config"
}

# Resolves a devicetree node label to its generated header node identifier.
function node_id
{
    local header_file=$1
    local label=$2
    local id

    id=$(sed -n "s/^#define DT_N_NODELABEL_${label}[[:space:]]\\+\\(.*\\)$/\\1/p" "$header_file" | head -n 1)

    if [[ -z $id ]]
    then
        printf 'Unable to find node label: %s\n' "$label" >&2
        exit 1
    fi

    printf '%s\n' "$id"
}

# Resolves a /chosen property to its generated header node identifier.
function chosen_node_id
{
    local header_file=$1
    local chosen=$2
    local id

    id=$(sed -n "s/^#define DT_CHOSEN_${chosen}[[:space:]]\\+\\(.*\\)$/\\1/p" "$header_file" | head -n 1)

    if [[ -z $id ]]
    then
        printf 'Unable to find chosen node: %s\n' "$chosen" >&2
        exit 1
    fi

    printf '%s\n' "$id"
}

# Reads a generated REG_IDX_0 address or size value for a node.
function reg_value
{
    local header_file=$1
    local id=$2
    local field=$3
    local value

    value=$(sed -n "s/^#define ${id}_REG_IDX_0_VAL_${field}[[:space:]]\\+\\([0-9]\\+\\).*/\\1/p" "$header_file" | head -n 1)

    if [[ -z $value ]]
    then
        printf 'Unable to find %s for node: %s\n' "$field" "$id" >&2
        exit 1
    fi

    printf '%u\n' "$value"
}

# Reads and normalizes one Kconfig value from the app build .config.
function config_value
{
    local config_file=$1
    local key=$2
    local value

    value=$(sed -n "s/^${key}=\\(.*\\)$/\\1/p" "$config_file" | head -n 1)

    if [[ -z $value ]]
    then
        printf 'Unable to find config value: %s\n' "$key" >&2
        exit 1
    fi

    parse_int "$value"
}

function find_app_base_from_dts
{
    local dts_file=$1
    local flash_base=
    local in_app=0
    local remaining=0
    local line

    while IFS= read -r line
    do
        if [[ $line =~ flash@([0-9A-Fa-f]+)[[:space:]]*\{ ]]
        then
            flash_base=$((16#${BASH_REMATCH[1]}))
        fi

        if [[ $line == *opendeck_partition_app:* ]]
        then
            in_app=1
            remaining=12
        fi

        if ((in_app))
        then
            if [[ $line =~ reg[[:space:]]*=[[:space:]]*\<[[:space:]]*([^[:space:]]+)[[:space:]]+([^[:space:]]+)[[:space:]]*\> ]]
            then
                if [[ -z $flash_base ]]
                then
                    printf 'Found app partition before flash base\n' >&2
                    exit 1
                fi

                local partition_offset
                partition_offset=$(parse_int "${BASH_REMATCH[1]}")
                printf '%u\n' "$((flash_base + partition_offset))"
                return
            fi

            remaining=$((remaining - 1))
            if ((remaining == 0))
            then
                printf 'Found app partition without reg property\n' >&2
                exit 1
            fi
        fi
    done < "$dts_file"

    printf 'Unable to find opendeck_partition_app in DTS\n' >&2
    exit 1
}

# Finds the absolute flash address of opendeck_partition_app.
#
# Prefer Zephyr's generated devicetree header because it already resolves
# ranges and mapped partitions. Fall back to the DTS scan for simpler boards
# whose partitions sit directly below a flash@... node.
function find_app_base
{
    local dts_file=$1
    local header_file

    header_file=$(generated_header "$dts_file")

    if [[ -f $header_file ]]
    then
        reg_value "$header_file" "$(node_id "$header_file" opendeck_partition_app)" ADDRESS
        return
    fi

    find_app_base_from_dts "$dts_file"
}

# Writes the address ranges needed to validate the generated app image in tests.
function write_info_header
{
    local dts_file=$1
    local output_file=$2
    local header_file
    local app_node
    local sram_node

    header_file=$(generated_header "$dts_file")

    app_node=$(node_id "$header_file" opendeck_partition_app)
    sram_node=$(chosen_node_id "$header_file" zephyr_sram)

    {
        printf '#pragma once\n\n'
        printf '#define OPENDECK_VALIDATED_APP_START %s\n' "$(reg_value "$header_file" "$app_node" ADDRESS)"
        printf '#define OPENDECK_VALIDATED_APP_SIZE %s\n' "$(reg_value "$header_file" "$app_node" SIZE)"
        printf '#define OPENDECK_VALIDATED_SRAM_START %s\n' "$(reg_value "$header_file" "$sram_node" ADDRESS)"
        printf '#define OPENDECK_VALIDATED_SRAM_SIZE %s\n' "$(reg_value "$header_file" "$sram_node" SIZE)"
        printf '#define OPENDECK_VALIDATED_ROM_START_OFFSET %s\n' "$(config_value "$(config_file "$dts_file")" CONFIG_ROM_START_OFFSET)"
    } > "$output_file"
}

function le32_hex
{
    local value=$1

    printf '%02X%02X%02X%02X' \
        "$((value & 0xFF))" \
        "$(((value >> 8) & 0xFF))" \
        "$(((value >> 16) & 0xFF))" \
        "$(((value >> 24) & 0xFF))"
}

for arg in "$@"
do
    case "$arg" in
        --input=*)
            payload_file=${arg#--input=}
            ;;

        --output=*)
            output_file=${arg#--output=}
            ;;

        --dts=*)
            dts_file=${arg#--dts=}
            ;;

        --hex-output=*)
            hex_output_file=${arg#--hex-output=}
            ;;

        --info-output=*)
            info_output_file=${arg#--info-output=}
            ;;

        --objcopy=*)
            objcopy=${arg#--objcopy=}
            ;;

        *)
            printf 'Unknown argument: %s\n' "$arg" >&2
            usage
            exit 2
            ;;
    esac
done

if [[ -z "$payload_file" || -z "$output_file" ]]
then
    usage
    exit 2
fi

if [[ -n "$hex_output_file" && ( -z "$dts_file" || -z "$objcopy" ) ]]
then
    printf '--hex-output requires --dts and --objcopy\n' >&2
    usage
    exit 2
fi

if [[ -n "$info_output_file" && -z "$dts_file" ]]
then
    printf '--info-output requires --dts\n' >&2
    usage
    exit 2
fi

payload_size=$(wc -c < "$payload_file")
payload_crc=$("${script_dir}/gen_crc32.sh" "$payload_file")
# The selector scans 4-byte aligned record locations, so pad the raw payload
# with erased flash bytes before appending the validation record.
record_offset=$(((payload_size + 3) & ~3))
padding_size=$((record_offset - payload_size))
record_hex="$(le32_hex "$image_validation_magic")$(le32_hex "$payload_size")$(le32_hex "$payload_crc")"
record_hex_file="${output_file}.record.hex"

cp "$payload_file" "$output_file"

for ((i = 0; i < padding_size; i++))
do
    printf "\\x%02X" "$erased_byte" >> "$output_file"
done

printf '%s' "$record_hex" > "$record_hex_file"
xxd -r -p "$record_hex_file" >> "$output_file"
rm -f "$record_hex_file"

actual_record_bytes=$(( ${#record_hex} / 2 ))

if ((actual_record_bytes != image_validation_record_size))
then
    printf 'Generated invalid validation record size: %u\n' "$actual_record_bytes" >&2
    exit 1
fi

if [[ -n "$hex_output_file" ]]
then
    # Raw binaries have no address; HEX output must be relocated to the app
    # partition base so merged flashing places the validation record correctly.
    "$objcopy" -I binary -O ihex --change-addresses "$(find_app_base "$dts_file")" "$output_file" "$hex_output_file"
fi

if [[ -n "$info_output_file" ]]
then
    write_info_header "$dts_file" "$info_output_file"
fi
