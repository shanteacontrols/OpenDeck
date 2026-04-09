#!/usr/bin/env bash

set -euo pipefail

yaml_parser="dasel -n -p yaml"

function usage
{
    echo -e "\nUsage: ./$(basename "$0") <mode> [options]"

    echo -e "
    This helper reads normalized OpenDeck test and target metadata for scripts and CMake helpers.
    It can be launched in two modes:

    testcase
    Reads metadata from one testcase.yaml entry.
    Required options:
      --file <testcase.yaml>
      --name <test_name>
    Optional:
      --key <metadata_key>

    target
    Reads metadata from one OpenDeck target overlay.
    Required option:
      --target <target_name>
    Alternative option:
      --overlay <opendeck.overlay>
    Optional:
      --key <metadata_key>
    "
}

function print_metadata
{
    local metadata=$1
    local key=${2:-}

    if [[ -n "$key" ]]
    then
        printf '%s\n' "$metadata" | sed -n "s/^${key}=//p" | head -n1
        return
    fi

    printf '%s\n' "$metadata"
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
    local board_name

    zephyr_board=$(sed -n 's/.*zephyr-board[[:space:]]*=[[:space:]]*"\([^"]*\)".*/\1/p' "$overlay" | head -n1)
    board_name=$(sed -n 's/.*board-name[[:space:]]*=[[:space:]]*"\([^"]*\)".*/\1/p' "$overlay" | head -n1)

    print_metadata "$(awk -v target="$target" -v zephyr_board="$zephyr_board" -v board_name="$board_name" '
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
            printf "board_name=%s\n", board_name;
            printf "test_host=%s\n", test_host;
            printf "test_hw=%s\n", test_hw;
            printf "bulk_app=%s\n", bulk_app;
            printf "bulk_host_test=%s\n", bulk_host_test;
            printf "bulk_hw_test=%s\n", bulk_hw_test;
            printf "bulk_lint=%s\n", bulk_lint;
        }
    ' "$overlay")" "$key"
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
    *)
        echo "ERROR: Unknown mode '$mode'" >&2
        usage >&2
        exit 1
        ;;
esac
