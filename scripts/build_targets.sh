#!/bin/bash

set -e

function usage
{
    echo -e "\nUsage: ./$(basename "$0") --type [--hw]"

    echo -e "
    This script is used to build specific set of targets from targets.yml file
    located in the root directory of OpenDeck repository."

    echo -e "\n--type=
    Specifies which targets to build. Available options are:
    fw          Builds all available firmware targets
    tests       Builds all tests for all targets"

    echo -e "\n--hw
    If set, only tests which run on physical boards will be compiled. If type is set to fw, this flag is ignored."

    echo -e "\n--clean
    If set, all build artifacts will be cleaned before running the build."

    echo -e "\n--help
    Displays script usage"
}

if [[ ("$*" == "--help") || ($# -eq 0) ]]
then
    usage
    exit 1
fi

for i in "$@"; do
    case "$i" in
        --type=*)
            TYPE=${i#--type=}
            ;;
        --hw)
            HW=1
            ;;

        --clean)
            CLEAN=1
            ;;
    esac
done

case $TYPE in
  fw)
    run_dir="src"
    ;;

  tests)
    run_dir="tests"
    ;;

  *)
    echo "ERROR: Invalid build type specified"
    usage
    exit 1
    ;;
esac

if [[ $(basename "$(pwd)") != "$run_dir" ]]
then
    echo "ERROR: Script must be run from $run_dir directory!"
    exit 1
fi

targets=()

if [[ "$TYPE" == "tests" ]]
then
    make pre-build
fi

if [[ "$TYPE" == "tests" && -n "$HW" ]]
then
    while IFS= read -r target
    do
        targets+=("$(basename "$target" .yml)")
    done < src/hw/boards.txt
    
else
    for target in ../targets/*.yml;
    do
        targets+=("$(basename "$target" .yml)")
    done
fi

len_targets=${#targets[@]}

for (( i=0; i<len_targets; i++ ))
do
    if [[ "$TYPE" != "tests" ]]
    then
        make TARGET="${targets[$i]}" DEBUG=0
    else
        #binaries, sysex files and defines are needed for tests, compile that as well
        make -C ../src TARGET="${targets[$i]}" DEBUG=0

        if [[ -n "$HW" ]]
        then
            make TARGET="${targets[$i]}" DEBUG=0 TESTS=hw
        else
            make TARGET="${targets[$i]}" DEBUG=0
        fi
    fi
done