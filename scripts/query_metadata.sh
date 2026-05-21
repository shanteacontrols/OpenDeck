#!/usr/bin/env bash

set -euo pipefail

yaml_parser="dasel -n -p yaml"

function usage
{
    echo -e "\nUsage: ./$(basename "$0") <mode> [options]"

    echo -e "
    This helper reads normalized OpenDeck metadata for scripts and CMake helpers.
    It can be launched in three modes:

    target
    Reads metadata from one OpenDeck target overlay.
    Required option:
      --target <target_name>
    Alternative option:
      --overlay <firmware.overlay>
    Optional:
      --key <target|board_name|bulk_app|bulk_lint|target_alias_overlay_line>

    dts
    Reads metadata from one generated zephyr.dts file by converting it to YAML.
    Required option:
      --file <zephyr.dts>
    Optional:
      --key <app_base|app_size|sram_base|sram_size>

    config
    Reads the effective value for one Kconfig symbol from an ordered list
    of .conf/.config files. Later files override earlier files.
    Required options:
      --key <CONFIG_NAME>
    Optional:
      --default <value>
      --file <config_file> (can be passed multiple times)

    target-config
    Exports all PROJECT_TARGET_* symbols declared in app/firmware/Kconfig
    from one resolved app .config file.
    Required options:
      --kconfig <app/firmware/Kconfig>
      --config <app/zephyr/.config>
      --output <target.conf>
      --kconfig-output <target.kconfig>
    "
}

function print_metadata
{
    local metadata=$1
    local key=${2:-}

    if [[ -n "$key" ]]
    then
        printf '%s\n' "$metadata" | sed -n "s/^${key}=//p"
        return
    fi

    printf '%s\n' "$metadata"
}

function target_alias_overlay_lines
{
    local firmware_overlay=$1

    if [[ ! -f "$firmware_overlay" ]]
    then
        return
    fi

    sed -n 's/^\(opendeck_\(uart_din_midi\|uart_touchscreen\|i2c_display\):.*\)$/target_alias_overlay_line=\1/p' "$firmware_overlay"
}

function config_metadata
{
    local key=
    local default_value=
    local config_files=()
    local config_file
    local value

    while [[ $# -gt 0 ]]
    do
        case "$1" in
            --file)
                config_files+=("$2")
                shift 2
                ;;
            --key)
                key=$2
                shift 2
                ;;
            --default)
                default_value=$2
                shift 2
                ;;
            *)
                echo "ERROR: Unknown config argument '$1'" >&2
                exit 1
                ;;
        esac
    done

    if [[ -z "$key" ]]
    then
        echo "ERROR: config mode requires --key." >&2
        exit 1
    fi

    value=$default_value

    for config_file in "${config_files[@]}"
    do
        if [[ ! -f "$config_file" ]]
        then
            continue
        fi

        local file_value

        file_value=$(sed -n "s/^${key}=\\(.*\\)$/\\1/p" "$config_file" | head -n1)
        file_value=${file_value#\"}
        file_value=${file_value%\"}

        if [[ -n "$file_value" ]]
        then
            value=$file_value
            continue
        fi

        if grep -q "^# ${key} is not set$" "$config_file"
        then
            value=n
        fi
    done

    printf '%s\n' "$value"
}

function target_config_metadata
{
    local kconfig_file=
    local config_file=
    local output_file=
    local kconfig_output_file=

    while [[ $# -gt 0 ]]
    do
        case "$1" in
            --kconfig)
                kconfig_file=$2
                shift 2
                ;;
            --config)
                config_file=$2
                shift 2
                ;;
            --output)
                output_file=$2
                shift 2
                ;;
            --kconfig-output)
                kconfig_output_file=$2
                shift 2
                ;;
            *)
                echo "ERROR: Unknown target-config argument '$1'" >&2
                exit 1
                ;;
        esac
    done

    if [[ -z "$kconfig_file" || -z "$config_file" || -z "$output_file" || -z "$kconfig_output_file" ]]
    then
        echo "ERROR: target-config mode requires --kconfig, --config, --output and --kconfig-output." >&2
        exit 1
    fi

    if [[ ! -f "$kconfig_file" ]]
    then
        echo "ERROR: Kconfig file '$kconfig_file' does not exist." >&2
        exit 1
    fi

    if [[ ! -f "$config_file" ]]
    then
        echo "ERROR: Config file '$config_file' does not exist." >&2
        exit 1
    fi

    mkdir -p "$(dirname "$output_file")"
    mkdir -p "$(dirname "$kconfig_output_file")"

    {
        sed -n 's/^config \(PROJECT_TARGET_[A-Z0-9_]*\)$/\1/p' "$kconfig_file" |
            while read -r name
            do
                local line

                line=$(grep -E "^(CONFIG_${name}=|# CONFIG_${name} is not set$)" "$config_file" | head -n1 || true)

                if [[ -z "$line" ]]
                then
                    line="# CONFIG_${name} is not set"
                fi

                printf '%s\n' "$line"
            done
    } > "$output_file"

    {
        printf 'menu "Imported OpenDeck target metadata"\n\n'

        awk '
            /^config PROJECT_TARGET_[A-Z0-9_]+$/ {
                name = $2
                next
            }

            name != "" && /^[[:space:]]+(bool|int)([[:space:]]|$)/ {
                type = $1
                printf("config %s\n    %s \"Imported target metadata\"\n\n", name, type)
                name = ""
            }
        ' "$kconfig_file"

        printf 'endmenu\n'
    } > "$kconfig_output_file"
}

function target_metadata
{
    local target=
    local overlay=
    local key=
    local project_root=${ZEPHYR_PROJECT:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}

    while [[ $# -gt 0 ]]
    do
        case "$1" in
            --target)
                target=$2
                shift 2
                ;;
            --overlay)
                overlay=$2
                shift 2
                ;;
            --key)
                key=$2
                shift 2
                ;;
            *)
                echo "ERROR: Unknown target argument '$1'" >&2
                exit 1
                ;;
        esac
    done

    if [[ -z "$overlay" && -n "$target" ]]
    then
        overlay="${project_root}/app/boards/opendeck/${target}/firmware.overlay"
    fi

    if [[ -z "$overlay" ]]
    then
        echo "ERROR: target mode requires --target or --overlay." >&2
        exit 1
    fi

    if [[ ! -f "$overlay" ]]
    then
        echo "ERROR: Overlay '$overlay' does not exist." >&2
        exit 1
    fi

    if [[ -z "$target" ]]
    then
        target=$(basename "$(dirname "$overlay")")
    fi

    local board_name

    board_name=$(sed -n 's/.*board-name[[:space:]]*=[[:space:]]*"\([^"]*\)".*/\1/p' "$overlay" | head -n1)

    print_metadata "$(awk -v target="$target" -v board_name="$board_name" '
        BEGIN {
            in_bulk = 0;
            bulk_app = "false";
            bulk_lint = "false";
        }

        /opendeck_bulk_build:[[:space:]]+opendeck-bulk-build/ {
            in_bulk = 1;
            next;
        }

        in_bulk && /^[[:space:]]*};/ {
            in_bulk = 0;
            next;
        }

        in_bulk && /^[[:space:]]*app;/ {
            bulk_app = "true";
        }

        in_bulk && /^[[:space:]]*lint;/ {
            bulk_lint = "true";
        }

        END {
            printf "target=%s\n", target;
            printf "board_name=%s\n", board_name;
            printf "bulk_app=%s\n", bulk_app;
            printf "bulk_lint=%s\n", bulk_lint;
        }
    ' "$overlay")
$(target_alias_overlay_lines "$overlay")" "$key"
}

function dts_path_to_selector
{
    local dts_path=$1
    local selector=".[0]"
    local path_part

    dts_path=${dts_path#/}

    IFS='/' read -ra path_parts <<< "$dts_path"

    for path_part in "${path_parts[@]}"
    do
        if [[ -n "$path_part" ]]
        then
            selector="${selector}.${path_part}"
        fi
    done

    printf '%s\n' "$selector"
}

function dts_yaml_value
{
    local yaml_file=$1
    local selector=$2
    local value

    value=$(dasel -n -p yaml --plain -f "$yaml_file" "$selector")

    if [[ -z "$value" || "$value" == "null" ]]
    then
        echo "ERROR: Unable to query DTS YAML selector '${selector}'." >&2
        exit 1
    fi

    printf '%s\n' "$value"
}

function dts_app_base
{
    local yaml_file=$1
    local flash_path
    local app_partition_path
    local flash_selector
    local app_partition_selector
    local flash_base
    local app_partition_offset

    flash_path=$(dts_yaml_value "$yaml_file" ".[0].chosen.zephyr,flash.[0]")
    app_partition_path=$(dts_yaml_value "$yaml_file" ".[0].chosen.zephyr,code-partition.[0]")

    flash_selector=$(dts_path_to_selector "$flash_path")
    app_partition_selector=$(dts_path_to_selector "$app_partition_path")

    flash_base=$(dts_yaml_value "$yaml_file" "${flash_selector}.reg.[0].[0]")
    app_partition_offset=$(dts_yaml_value "$yaml_file" "${app_partition_selector}.reg.[0].[0]")

    printf '%u\n' "$((flash_base + app_partition_offset))"
}

function dts_app_size
{
    local yaml_file=$1
    local app_partition_path
    local app_partition_selector

    app_partition_path=$(dts_yaml_value "$yaml_file" ".[0].chosen.zephyr,code-partition.[0]")
    app_partition_selector=$(dts_path_to_selector "$app_partition_path")

    dts_yaml_value "$yaml_file" "${app_partition_selector}.reg.[0].[1]"
}

function dts_sram_value
{
    local yaml_file=$1
    local index=$2
    local sram_path
    local sram_selector

    sram_path=$(dts_yaml_value "$yaml_file" ".[0].chosen.zephyr,sram.[0]")
    sram_selector=$(dts_path_to_selector "$sram_path")

    dts_yaml_value "$yaml_file" "${sram_selector}.reg.[0].[${index}]"
}

function dts_metadata_from_yaml
{
    local dts_file=$1
    local yaml_file

    yaml_file=$(mktemp)
    trap 'rm -f "$yaml_file"' RETURN

    dtc -q -I dts -O yaml "$dts_file" > "$yaml_file"

    printf 'app_base=%s\n' "$(dts_app_base "$yaml_file")"
    printf 'app_size=%s\n' "$(dts_app_size "$yaml_file")"
    printf 'sram_base=%s\n' "$(dts_sram_value "$yaml_file" 0)"
    printf 'sram_size=%s\n' "$(dts_sram_value "$yaml_file" 1)"
}

function dts_metadata
{
    local dts_file=
    local key=

    while [[ $# -gt 0 ]]
    do
        case "$1" in
            --file)
                dts_file=$2
                shift 2
                ;;
            --key)
                key=$2
                shift 2
                ;;
            *)
                echo "ERROR: Unknown dts argument '$1'" >&2
                exit 1
                ;;
        esac
    done

    if [[ -z "$dts_file" ]]
    then
        echo "ERROR: dts mode requires --file." >&2
        exit 1
    fi

    if [[ ! -f "$dts_file" ]]
    then
        echo "ERROR: DTS file '$dts_file' does not exist." >&2
        exit 1
    fi

    local metadata

    metadata=$(dts_metadata_from_yaml "$dts_file")

    if [[ -n "$key" ]]
    then
        case "$key" in
            app_base|app_size|sram_base|sram_size)
                print_metadata "$metadata" "$key"
                ;;
            *)
                echo "ERROR: Unknown dts metadata key '$key'." >&2
                exit 1
                ;;
        esac

        return
    fi

    print_metadata "$metadata"
}

if [[ $# -lt 1 ]]
then
    usage >&2
    exit 1
fi

mode=$1
shift

case "$mode" in
    target)
        target_metadata "$@"
        ;;
    dts)
        dts_metadata "$@"
        ;;
    config)
        config_metadata "$@"
        ;;
    target-config)
        target_config_metadata "$@"
        ;;
    *)
        echo "ERROR: Unknown mode '$mode'" >&2
        usage >&2
        exit 1
        ;;
esac
