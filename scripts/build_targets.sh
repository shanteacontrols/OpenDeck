#!/usr/bin/env bash

set -e

function usage
{
    echo -e "\nUsage: ./$(basename "$0") --type [--hw]"

    echo -e "
    This script is used to build specific set of targets from targets.yml file
    located in the root directory of OpenDeck repository.
    If the script is launched from src directory, all available firmwares for all targets will be built.
    If the script is launched from tests directory, all tests for all targets will be built."

    echo -e "\n--hw
    If set, only tests which run on physical boards will be compiled. Otherwise, this flag is ignored."

    echo -e "\n--clean
    If set, all build artifacts will be cleaned before running the build."

    echo -e "\n--help
    Displays script usage"
}

run_dir_src="OpenDeck/src"
run_dir_tests="OpenDeck/tests"

if [[ $(pwd) != *"$run_dir_src" && $(pwd) != *"$run_dir_tests" ]]
then
    echo "ERROR: Script must be run either from src or tests directory!"
    usage
    exit 1
fi

if [[ ("$*" == "--help") ]]
then
    usage
    exit 1
fi

for i in "$@"; do
    case "$i" in
        --hw)
            hw=1
            ;;

        --clean)
            clean=1
            ;;
    esac
done

run_dir=$(basename "$(pwd)")

case $run_dir in
  src)
    type="fw"
    ;;

  tests)
    type="tests"
    ;;

  *)
    exit 1
    ;;
esac

if [[ -n "$clean" ]]
then
    make clean
fi

targets=()

if [[ "$type" == "tests" ]]
then
    make pre-build
fi

if [[ "$type" == "tests" && -n "$hw" ]]
then
    for target in ../config/hw-test/*.yml;
    do
        targets+=("$(basename "$target" .yml)")
    done
else
    for target in ../config/target/*.yml;
    do
        targets+=("$(basename "$target" .yml)")
    done
fi

len_targets=${#targets[@]}

for (( i=0; i<len_targets; i++ ))
do
    if [[ "$type" != "tests" ]]
    then
        make TARGET="${targets[$i]}" DEBUG=0
    else
        # binaries, sysex files and defines are needed for tests, compile that as well
        make --no-print-directory -C ../src TARGET="${targets[$i]}" DEBUG=0

        if [[ -n "$hw" ]]
        then
            make TARGET="${targets[$i]}" DEBUG=0 TESTS=hw
        else
            make TARGET="${targets[$i]}" DEBUG=0
        fi
    fi
done