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

    --type=hw-test
    This option will build and run hardware tests for targets whose opendeck-bulk-build node contains hw-test.
    Hardware tests tagged with preset are run for every eligible target; hardware tests without
    preset are run once using the first eligible target as the representative preset.

    --type=lint
    This option will run CodeChecker analysis for targets whose opendeck-bulk-build node contains lint.
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
    esac
done

project_root=$ZEPHYR_PROJECT
build_targets=()
yaml_parser="dasel -n -p yaml"
metadata_query_script="${project_root}/scripts/query_test_metadata.sh"

function load_targets
{
    mapfile -t build_targets < <(find "$project_root/app/boards/opendeck" -mindepth 2 -maxdepth 2 -type f -name opendeck.overlay -printf '%h\n' | xargs -r -n1 basename | sort)
}

function metadata_value
{
    local metadata=$1
    local key=$2

    printf '%s\n' "$metadata" | sed -n "s/^${key}=//p" | head -n1
}

function list_tests_by_tag
{
    local required_tag=$1
    local preset_mode=$2

    while IFS= read -r -d '' testcase_file
    do
        while IFS= read -r test_name
        do
            local metadata
            local mode
            local bulk_mode

            metadata=$(bash "$metadata_query_script" testcase --file "$testcase_file" --name "$test_name")
            mode=$(metadata_value "$metadata" "mode")
            bulk_mode=$(metadata_value "$metadata" "bulk_mode")

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

case $type in
    app)
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
        load_targets
        host_targets=()

        for target in "${build_targets[@]}"
        do
            target_metadata=$(bash "$metadata_query_script" target --target "$target")

            if [[ "$(metadata_value "$target_metadata" "bulk_host_test")" == "true" ]] &&
               [[ "$(metadata_value "$target_metadata" "test_host")" == "true" ]]
            then
                host_targets+=("$target")
            fi
        done

        mapfile -t shared_host_tests < <(list_tests_by_tag "host" "without" | sort)
        mapfile -t preset_host_tests < <(list_tests_by_tag "host" "with" | sort)

        if (( ${#host_targets[@]} == 0 ))
        then
            echo "ERROR: No host-test targets are enabled for bulk execution."
            exit 1
        fi

        run_tests_once_per_suite "${host_targets[0]}" "shared_host" "${shared_host_tests[@]}"

        host_tests=("${preset_host_tests[@]}")
        run_tests_for_each_target "host" "${host_targets[@]}"
    ;;

    hw-test)
        load_targets
        hw_targets=()

        for target in "${build_targets[@]}"
        do
            target_metadata=$(bash "$metadata_query_script" target --target "$target")

            if [[ "$(metadata_value "$target_metadata" "bulk_hw_test")" == "true" ]] &&
               [[ "$(metadata_value "$target_metadata" "test_hw")" == "true" ]]
            then
                hw_targets+=("$target")
            fi
        done

        mapfile -t shared_hw_tests < <(list_tests_by_tag "hw" "without" | sort)
        mapfile -t preset_hw_tests < <(list_tests_by_tag "hw" "with" | sort)

        if (( ${#hw_targets[@]} == 0 ))
        then
            echo "ERROR: No hw-test targets are enabled for bulk execution."
            exit 1
        fi

        run_tests_once_per_suite "${hw_targets[0]}" "shared_hw" "${shared_hw_tests[@]}"

        hw_tests=("${preset_hw_tests[@]}")
        run_tests_for_each_target "hw" "${hw_targets[@]}"
    ;;

    lint)
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
