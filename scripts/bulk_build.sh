#!/usr/bin/env bash

set -e

function usage
{
    echo -e "\nUsage: ./$(basename "$0") --type"

    echo -e "
    This script is used to run bulk build, test, and lint actions for OpenDeck targets selected through the opendeck-bulk-build binding.
    This script can be launched with different --type options:

    --type=app
    This option will build the application for targets whose opendeck-bulk-build node contains app.

    --type=host-test
    This option will build and run host tests for targets whose opendeck-bulk-build node contains host-test.
    Host tests tagged with preset are run for every eligible target; host tests without preset are
    run once using the first eligible target as the representative preset.

    --type=hardware-test
    This option will build and run hardware tests for targets whose opendeck-bulk-build node contains hardware-test.
    Hardware tests tagged with preset are run for every eligible target; hardware tests without
    preset are run once using the first eligible target as the representative preset.

    --type=lint
    This option will run CodeChecker analysis for targets whose opendeck-bulk-build node contains lint.

    --print-target-matrix
    Prints selected targets as a JSON array and exits. Currently supported for --type=app and --type=lint.

    --test-scope=shared|preset|all
    Selects which tests to run for --type=host-test and --type=hardware-test. Defaults to all.

    --test-target=<target>
    Restricts --type=host-test and --type=hardware-test preset execution to a single target.
    "

    echo -e "\n--help
    Displays script usage"
}

if [[ ("$*" == "--help") ]]
then
    usage
    exit 0
fi

for arg in "$@"; do
    case "$arg" in
        --type=*)
            type=${arg#--type=}
            ;;
        --print-target-matrix)
            print_target_matrix=true
            ;;
        --test-scope=*)
            test_scope=${arg#--test-scope=}
            ;;
        --test-target=*)
            selected_test_target=${arg#--test-target=}
            ;;
    esac
done

project_root=${ZENV_PROJECT_ROOT:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}
build_targets=()
print_target_matrix=${print_target_matrix:-false}
test_scope=${test_scope:-all}
selected_test_target=${selected_test_target:-}
yaml_parser="dasel -n -p yaml"
metadata_query_script="${project_root}/scripts/query_metadata.sh"

function load_targets
{
    mapfile -t build_targets < <(find "$project_root/app/boards/opendeck" -mindepth 2 -maxdepth 2 -type f -name firmware.overlay -printf '%h\n' | xargs -r -n1 basename | sort)
}

function metadata_value
{
    local metadata=$1
    local key=$2

    printf '%s\n' "$metadata" | sed -n "s/^${key}=//p" | head -n1
}

function testcase_tags
{
    local testcase_file=$1
    local test_name=$2
    local escaped_test_name=${test_name//./\\.}

    $yaml_parser -m --plain -f "$testcase_file" "tests.${escaped_test_name}.tags.[*]"
}

function list_tests_by_tag
{
    local required_tag=$1
    local preset_mode=$2

    while IFS= read -r -d '' testcase_file
    do
        while IFS= read -r test_name
        do
            local mode
            local bulk_mode
            local tag

            mode=unknown
            bulk_mode=shared

            while IFS= read -r tag
            do
                case "$tag" in
                    host|hardware)
                        mode=$tag
                        ;;
                    preset)
                        bulk_mode=preset
                        ;;
                esac
            done < <(testcase_tags "$testcase_file" "$test_name")

            if [[ "$mode" != "$required_tag" ]]
            then
                continue
            fi

            if [[ "$preset_mode" == "with" && "$bulk_mode" != "preset" ]]
            then
                continue
            fi

            if [[ "$preset_mode" == "without" && "$bulk_mode" == "preset" ]]
            then
                continue
            fi

            printf '%s\n' "$test_name"
        done < <($yaml_parser -m --plain -f "$testcase_file" 'tests.-')
    done < <(find "$project_root/tests/src" -type f -name testcase.yaml -print0 | sort -z)
}

function run_tests_once_per_suite
{
    local representative_target=$1
    local suite_label=$2

    shift 2
    local tests=("$@")
    local total_tests=${#tests[@]}
    local test_counter=0

    if (( total_tests == 0 ))
    then
        return
    fi

    echo "==> Running ${suite_label} tests once using representative target: ${representative_target}"

    for test_id in "${tests[@]}"
    do
        ((test_counter += 1))
        echo "==> Running ${suite_label} test ${test_counter}/${total_tests}: ${test_id}"
        make tests TARGET="$representative_target" RUN=1 TEST="${test_id%.test}"
    done
}

function run_tests_for_each_target
{
    local suite_label=$1
    shift
    local targets=("$@")
    local tests_var_name="${suite_label}_tests"
    local -n tests_ref="$tests_var_name"
    local total_targets=${#targets[@]}
    local total_tests=${#tests_ref[@]}
    local target_counter=0

    if (( total_tests == 0 ))
    then
        return
    fi

    for target in "${targets[@]}"
    do
        ((target_counter += 1))
        local test_counter=0

        for test_id in "${tests_ref[@]}"
        do
            ((test_counter += 1))
            echo "==> Running ${suite_label} preset test ${test_counter}/${total_tests} for target ${target_counter}/${total_targets}: ${target} (${test_id})"
            make tests TARGET="$target" RUN=1 TEST="${test_id%.test}"
        done
    done
}

function select_single_target
{
    local selected=$1
    shift
    local targets=("$@")
    local target

    if [[ -z "$selected" ]]
    then
        printf '%s\n' "${targets[@]}"
        return
    fi

    for target in "${targets[@]}"
    do
        if [[ "$target" == "$selected" ]]
        then
            printf '%s\n' "$target"
            return
        fi
    done

    echo "ERROR: Target '${selected}' is not enabled for ${type}." >&2
    exit 1
}

function json_array
{
    python3 -c 'import json, sys; print(json.dumps(sys.argv[1:], separators=(",", ":")))' "$@"
}

function load_app_targets
{
    load_targets
    app_targets=()

    for target in "${build_targets[@]}"
    do
        target_metadata=$(bash "$metadata_query_script" target --target "$target")

        if [[ "$(metadata_value "$target_metadata" "bulk_app")" == "true" ]]
        then
            app_targets+=("$target")
        fi
    done
}

function load_lint_targets
{
    load_targets
    lint_targets=()

    for target in "${build_targets[@]}"
    do
        target_metadata=$(bash "$metadata_query_script" target --target "$target")

        if [[ "$(metadata_value "$target_metadata" "bulk_lint")" == "true" ]]
        then
            lint_targets+=("$target")
        fi
    done
}

function load_host_targets
{
    load_targets
    host_targets=()

    for target in "${build_targets[@]}"
    do
        target_metadata=$(bash "$metadata_query_script" target --target "$target")

        if [[ "$(metadata_value "$target_metadata" "bulk_host_test")" == "true" ]]
        then
            host_targets+=("$target")
        fi
    done
}

function load_hardware_targets
{
    load_targets
    hardware_targets=()

    for target in "${build_targets[@]}"
    do
        target_metadata=$(bash "$metadata_query_script" target --target "$target")

        if [[ "$(metadata_value "$target_metadata" "bulk_hardware_test")" == "true" ]]
        then
            hardware_targets+=("$target")
        fi
    done
}

case $type in
    app)
        load_app_targets

        if [[ "$print_target_matrix" == "true" ]]
        then
            json_array "${app_targets[@]}"
            exit 0
        fi

        if (( ${#app_targets[@]} == 0 ))
        then
            echo "ERROR: No app targets are enabled for bulk execution."
            exit 1
        fi

        total_targets=${#app_targets[@]}
        target_counter=0

        for target in "${app_targets[@]}"
        do
            ((target_counter += 1))
            echo "==> Building target ${target_counter}/${total_targets}: ${target}"
            make TARGET="$target"
        done
    ;;

    host-test)
        load_host_targets

        if (( ${#host_targets[@]} == 0 ))
        then
            echo "ERROR: No host-test targets are enabled for bulk execution."
            exit 1
        fi

        if [[ "$print_target_matrix" == "true" ]]
        then
            json_array "${host_targets[@]}"
            exit 0
        fi

        mapfile -t shared_host_tests < <(list_tests_by_tag "host" "without" | sort)
        mapfile -t preset_host_tests < <(list_tests_by_tag "host" "with" | sort)
        mapfile -t selected_host_targets < <(select_single_target "$selected_test_target" "${host_targets[@]}")

        case "$test_scope" in
            shared)
                run_tests_once_per_suite "${host_targets[0]}" "shared_host" "${shared_host_tests[@]}"
                ;;

            preset)
                host_tests=("${preset_host_tests[@]}")
                run_tests_for_each_target "host" "${selected_host_targets[@]}"
                ;;

            all)
                run_tests_once_per_suite "${host_targets[0]}" "shared_host" "${shared_host_tests[@]}"

                host_tests=("${preset_host_tests[@]}")
                run_tests_for_each_target "host" "${selected_host_targets[@]}"
                ;;

            *)
                echo "ERROR: Invalid test scope '${test_scope}'. Expected shared, preset, or all."
                exit 1
                ;;
        esac
    ;;

    hardware-test)
        load_hardware_targets

        if (( ${#hardware_targets[@]} == 0 ))
        then
            echo "ERROR: No hardware-test targets are enabled for bulk execution."
            exit 1
        fi

        if [[ "$print_target_matrix" == "true" ]]
        then
            json_array "${hardware_targets[@]}"
            exit 0
        fi

        mapfile -t shared_hardware_tests < <(list_tests_by_tag "hardware" "without" | sort)
        mapfile -t preset_hardware_tests < <(list_tests_by_tag "hardware" "with" | sort)
        mapfile -t selected_hardware_targets < <(select_single_target "$selected_test_target" "${hardware_targets[@]}")

        case "$test_scope" in
            shared)
                run_tests_once_per_suite "${hardware_targets[0]}" "shared_hardware" "${shared_hardware_tests[@]}"
                ;;

            preset)
                hardware_tests=("${preset_hardware_tests[@]}")
                run_tests_for_each_target "hardware" "${selected_hardware_targets[@]}"
                ;;

            all)
                run_tests_once_per_suite "${hardware_targets[0]}" "shared_hardware" "${shared_hardware_tests[@]}"

                hardware_tests=("${preset_hardware_tests[@]}")
                run_tests_for_each_target "hardware" "${selected_hardware_targets[@]}"
                ;;

            *)
                echo "ERROR: Invalid test scope '${test_scope}'. Expected shared, preset, or all."
                exit 1
                ;;
        esac
    ;;

    lint)
        load_lint_targets

        if [[ "$print_target_matrix" == "true" ]]
        then
            json_array "${lint_targets[@]}"
            exit 0
        fi

        if (( ${#lint_targets[@]} == 0 ))
        then
            echo "ERROR: No lint targets are enabled for bulk execution."
            exit 1
        fi

        total_targets=${#lint_targets[@]}
        target_counter=0

        for target in "${lint_targets[@]}"
        do
            ((target_counter += 1))
            echo "==> Running lint for target ${target_counter}/${total_targets}: ${target}"
            make CHECK=1 TARGET="$target"
        done
    ;;

    *)
      echo "ERROR: Invalid build type specified."
      usage
      exit 1
    ;;
esac
