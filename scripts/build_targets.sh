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
    fw_all      Builds all listed firmware targets
    fw_release  Builds only firmware targets which are part of official OpenDeck release
    tests       Builds all tests"

    echo -e "\n--hw
    If set, only tests which run on physical boards will be compiled"

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
    esac
done

BUILD_RELEASE=false

case $TYPE in
  fw_all)
    run_dir="src"
    ;;

  fw_release)
    run_dir="src"
    BUILD_RELEASE=true
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

if [[ "$BUILD_RELEASE" == "true" ]]
then
    make clean
fi

for (( i=0; i<len_targets; i++ ))
do
    if [[ "$TYPE" != "tests" ]]
    then
        make TARGET="${targets[$i]}" DEBUG=0

        if [[ "$BUILD_RELEASE" == "true" ]]
        then
            if [[ ${targets[$i]} == "mega16u2" ]]
            then
                #no need to create sysex firmware for atmega16u2 - it's only a USB link
                continue;
            fi

            #create sysex fw update file
            make sysexfw TARGET="${targets[$i]}"
        fi
    else
        if [[ "$TYPE" == "tests" && -n "$HW" ]]
        then
            make pre-build TARGET="${targets[$i]}" DEBUG=0
            make TARGET="${targets[$i]}" DEBUG=0 TESTS=hw
            make -C ../src TARGET="${targets[$i]}" sysexfw DEBUG=0
        else
            make pre-build TARGET="${targets[$i]}" DEBUG=0
            make TARGET="${targets[$i]}" DEBUG=0
        fi
    fi
done