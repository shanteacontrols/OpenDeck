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
#   the app partition base resolved from generated zephyr.dts metadata.
# - --info-output writes a tiny C header with the app/SRAM boundaries and ROM
#   start offset resolved from generated zephyr.dts and .config metadata.

set -euo pipefail

script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
metadata_query_script="${script_dir}/../query_metadata.sh"
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

# Returns the Kconfig output next to zephyr.dts.
function config_file
{
    local dts_file=$1

    printf '%s\n' "${dts_file%/zephyr.dts}/.config"
}

# Reads and normalizes one Kconfig value from the app build .config.
function config_value
{
    local config_file=$1
    local key=$2
    local value

    value=$(bash "$metadata_query_script" config --file "$config_file" --key "$key")

    if [[ -z $value ]]
    then
        printf 'Unable to find config value: %s\n' "$key" >&2
        exit 1
    fi

    parse_int "$value"
}

function dts_value
{
    local dts_file=$1
    local key=$2
    local value

    value=$(bash "$metadata_query_script" dts --file "$dts_file" --key "$key")

    if [[ -z $value ]]
    then
        printf 'Unable to find DTS metadata value: %s\n' "$key" >&2
        exit 1
    fi

    printf '%u\n' "$value"
}

# Writes the address ranges needed to validate the generated app image in tests.
function write_info_header
{
    local dts_file=$1
    local output_file=$2

    {
        printf '#pragma once\n\n'
        printf '#define OPENDECK_VALIDATED_APP_START %s\n' "$(dts_value "$dts_file" app_base)"
        printf '#define OPENDECK_VALIDATED_APP_SIZE %s\n' "$(dts_value "$dts_file" app_size)"
        printf '#define OPENDECK_VALIDATED_SRAM_START %s\n' "$(dts_value "$dts_file" sram_base)"
        printf '#define OPENDECK_VALIDATED_SRAM_SIZE %s\n' "$(dts_value "$dts_file" sram_size)"
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
    "$objcopy" -I binary -O ihex --change-addresses "$(dts_value "$dts_file" app_base)" "$output_file" "$hex_output_file"
fi

if [[ -n "$info_output_file" ]]
then
    write_info_header "$dts_file" "$info_output_file"
fi
