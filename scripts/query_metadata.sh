#!/usr/bin/env bash

set -euo pipefail

yaml_parser="dasel -n -p yaml"

function usage
{
    echo -e "\nUsage: ./$(basename "$0") <mode> [options]"

    echo -e "
    This helper reads normalized OpenDeck metadata for scripts and CMake helpers.
    It can be launched in four modes:

    testcase
    Reads metadata from one testcase.yaml entry.
    Required options:
      --file <testcase.yaml>
      --name <test_name>
    Optional:
      --key <name|mode|bulk_mode>

    target
    Reads metadata from one OpenDeck target overlay.
    Required option:
      --target <target_name>
    Alternative option:
      --overlay <opendeck.overlay>
    Optional:
      --key <target|zephyr_board|zephyr_board_dir|zephyr_overlay_dir|board_name|emueeprom_page_size|test_host|test_hw|bulk_app|bulk_host_test|bulk_hw_test|bulk_lint|target_alias_overlay_line>

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

function parse_dts_int
{
    local value=$1

    case "$value" in
        0x*|0X*)
            printf '%u\n' "$((16#${value:2}))"
            ;;

        DT_SIZE_K\(*\))
            value=${value#DT_SIZE_K(}
            value=${value%)}
            printf '%u\n' "$((value * 1024))"
            ;;

        DT_SIZE_M\(*\))
            value=${value#DT_SIZE_M(}
            value=${value%)}
            printf '%u\n' "$((value * 1024 * 1024))"
            ;;

        *)
            printf '%u\n' "$value"
            ;;
    esac
}

function partition_size
{
    local partitions_overlay=$1
    local partition_label=$2
    local in_partition=false
    local line

    while IFS= read -r line
    do
        line="${line#"${line%%[![:space:]]*}"}"
        line="${line%"${line##*[![:space:]]}"}"

        if [[ $line == "${partition_label}:"* ]]
        then
            in_partition=true
            continue
        fi

        if [[ $in_partition != true ]]
        then
            continue
        fi

        if [[ $line == "};" ]]
        then
            break
        fi

        if [[ $line =~ reg[[:space:]]*=[[:space:]]*\<[^[:space:]]+[[:space:]]+([^[:space:]]+)\>\; ]]
        then
            parse_dts_int "${BASH_REMATCH[1]}"
            return
        fi
    done < "$partitions_overlay"

    echo "ERROR: Unable to find ${partition_label} size in ${partitions_overlay}." >&2
    exit 1
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

function testcase_metadata
{
    local testcase_file=
    local test_name=
    local key=

    while [[ $# -gt 0 ]]
    do
        case "$1" in
            --file)
                testcase_file=$2
                shift 2
                ;;
            --name)
                test_name=$2
                shift 2
                ;;
            --key)
                key=$2
                shift 2
                ;;
            *)
                echo "ERROR: Unknown testcase argument '$1'" >&2
                exit 1
                ;;
        esac
    done

    if [[ -z "$testcase_file" || -z "$test_name" ]]
    then
        echo "ERROR: testcase mode requires --file and --name." >&2
        exit 1
    fi

    local escaped_test_name=${test_name//./\\.}
    local tags=()
    local mode=unknown
    local bulk_mode=shared
    local tag

    mapfile -t tags < <($yaml_parser -m --plain -f "$testcase_file" "tests.${escaped_test_name}.tags.[*]")

    if [[ ${#tags[@]} -eq 1 && "${tags[0]}" == "null" ]]
    then
        tags=()
    fi

    for tag in "${tags[@]}"
    do
        case "$tag" in
            host|hw)
                mode=$tag
                ;;
            preset)
                bulk_mode=preset
                ;;
        esac
    done

    print_metadata "$(printf 'name=%s\nmode=%s\nbulk_mode=%s\n' "$test_name" "$mode" "$bulk_mode")" "$key"
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
        fi
    done

    printf '%s\n' "$value"
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
        overlay="${project_root}/app/boards/opendeck/${target}/opendeck.overlay"
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

    local zephyr_board
    local zephyr_board_dir
    local zephyr_overlay_dir
    local partitions_overlay
    local target_firmware_overlay
    local emueeprom_page_size
    local board_name

    zephyr_board=$(sed -n 's/.*zephyr-board[[:space:]]*=[[:space:]]*"\([^"]*\)".*/\1/p' "$overlay" | head -n1)
    board_name=$(sed -n 's/.*board-name[[:space:]]*=[[:space:]]*"\([^"]*\)".*/\1/p' "$overlay" | head -n1)
    zephyr_board_dir=${zephyr_board//\//_}
    zephyr_overlay_dir="${project_root}/app/boards/zephyr/${zephyr_board_dir}"
    partitions_overlay="${zephyr_overlay_dir}/partitions.overlay"
    target_firmware_overlay="${project_root}/app/boards/opendeck/${target}/firmware.overlay"

    if [[ -n "$zephyr_board" && -f "$partitions_overlay" ]]
    then
        emueeprom_page_size=$(partition_size "$partitions_overlay" "emueeprom_page1_partition")
    fi

    print_metadata "$(awk -v target="$target" -v zephyr_board="$zephyr_board" -v zephyr_board_dir="$zephyr_board_dir" -v board_name="$board_name" -v zephyr_overlay_dir="$zephyr_overlay_dir" -v emueeprom_page_size="$emueeprom_page_size" '
        BEGIN {
            in_tests = 0;
            in_bulk = 0;
            test_host = "false";
            test_hw = "false";
            bulk_app = "false";
            bulk_host_test = "false";
            bulk_hw_test = "false";
            bulk_lint = "false";
        }

        /opendeck_tests:[[:space:]]+opendeck-tests/ {
            in_tests = 1;
            next;
        }

        /opendeck_bulk_build:[[:space:]]+opendeck-bulk-build/ {
            in_bulk = 1;
            next;
        }

        in_tests && /^[[:space:]]*};/ {
            in_tests = 0;
            next;
        }

        in_bulk && /^[[:space:]]*};/ {
            in_bulk = 0;
            next;
        }

        in_tests && /^[[:space:]]*host;/ {
            test_host = "true";
        }

        in_tests && /^[[:space:]]*hw;/ {
            test_hw = "true";
        }

        in_bulk && /^[[:space:]]*app;/ {
            bulk_app = "true";
        }

        in_bulk && /^[[:space:]]*host-test;/ {
            bulk_host_test = "true";
        }

        in_bulk && /^[[:space:]]*hw-test;/ {
            bulk_hw_test = "true";
        }

        in_bulk && /^[[:space:]]*lint;/ {
            bulk_lint = "true";
        }

        END {
            printf "target=%s\n", target;
            printf "zephyr_board=%s\n", zephyr_board;
            printf "zephyr_board_dir=%s\n", zephyr_board_dir;
            printf "zephyr_overlay_dir=%s\n", zephyr_overlay_dir;
            printf "board_name=%s\n", board_name;
            printf "emueeprom_page_size=%s\n", emueeprom_page_size;
            printf "test_host=%s\n", test_host;
            printf "test_hw=%s\n", test_hw;
            printf "bulk_app=%s\n", bulk_app;
            printf "bulk_host_test=%s\n", bulk_host_test;
            printf "bulk_hw_test=%s\n", bulk_hw_test;
            printf "bulk_lint=%s\n", bulk_lint;
        }
    ' "$overlay")
$(target_alias_overlay_lines "$target_firmware_overlay")" "$key"
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
    testcase)
        testcase_metadata "$@"
        ;;
    target)
        target_metadata "$@"
        ;;
    dts)
        dts_metadata "$@"
        ;;
    config)
        config_metadata "$@"
        ;;
    *)
        echo "ERROR: Unknown mode '$mode'" >&2
        usage >&2
        exit 1
        ;;
esac
