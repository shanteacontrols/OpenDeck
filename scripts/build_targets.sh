#!/bin/bash

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
    If set, only tests which run on physical boards will be compiled. If type is set to fw, this flag is ignored."

    echo -e "\n--clean
    If set, all build artifacts will be cleaned before running the build."

    echo -e "\n--help
    Displays script usage"
}

if [[ ("$*" == "--help") ]]
then
    usage
    exit 1
fi

for i in "$@"; do
    case "$i" in
        --hw)
            HW=1
            ;;

        --clean)
            CLEAN=1
            ;;
    esac
done

run_dir=$(basename "$(pwd)")

case $run_dir in
  src)
    TYPE="fw"
    ;;

  tests)
    TYPE="tests"
    ;;

  *)
    echo "ERROR: Script must be run either from src or tests directory!"
    usage
    exit 1
    ;;
esac

if [[ -n "$CLEAN" ]]
then
    make clean
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

        if [[ -n "$HW" ]]
        then
            #binaries, sysex files and defines are needed for tests, compile that as well
            make -C ../src TARGET="${targets[$i]}" DEBUG=0
            make TARGET="${targets[$i]}" DEBUG=0 HW_TESTING=1 TESTS=hw
        else
            #only defines are needed here
            make -C ../src TARGET="${targets[$i]}" DEBUG=0 pre-build
            make TARGET="${targets[$i]}" DEBUG=0 HW_TESTING=0
        fi
    fi
done