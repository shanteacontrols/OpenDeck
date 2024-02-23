#!/usr/bin/env bash

set -e

function usage
{
    echo -e "\nUsage: ./$(basename "$0") --type"

    echo -e "
    This script is used to build all available targets present in config/target subdirectory.
    This script can be launched with different --type options:

    --type=build
    This option will build the firmware for all the targets.

    --type=test
    This option will build and run the tests for all the targets. Hardware tests will not be run.

    --type=hw-test
    This option will build and run just hardware tests for all the targets.

    --type=lint
    This option will build the firmware for all the targets and then perform static analysis.
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

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
project_root="$(realpath "${script_dir}"/..)"
targets_dir="$project_root"/config/target
yaml_parser="dasel -n -p yaml --plain -f"
targets=()

for target in "$targets_dir"/*.yml;
do
    targets+=("$(basename "$target" .yml)")
done

len_targets=${#targets[@]}

case $type in
    build)
        for (( i=0; i<len_targets; i++ ))
        do
            make TARGET="${targets[$i]}"
        done
    ;;

    test)
        for (( i=0; i<len_targets; i++ ))
        do
            if [[ $($yaml_parser "$targets_dir"/"${targets[$i]}".yml test) == "true" ]]
            then
                make TARGET="${targets[$i]}" test
            fi
        done
    ;;

    hw-test)
        for (( i=0; i<len_targets; i++ ))
        do
            make TARGET="${targets[$i]}" hw-test
        done
    ;;

    lint)
        for (( i=0; i<len_targets; i++ ))
        do
            make TARGET="${targets[$i]}" lint
        done
    ;;

    *)
      echo "ERROR: Invalid build type specified".
      usage
    ;;
esac