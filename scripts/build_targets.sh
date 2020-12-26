#!/bin/bash

function usage
{
    echo -e "\nUsage: ./$(basename "$0") --type"

    echo -e "
    This script is used to build specific set of targets from targets.yml file
    located in the root directory of OpenDeck repository."

    echo -e "\n--type=
    Specifies which targets to build. Available options are:
    fw_all      Builds all listed firmware targets
    fw_release  Builds only firmware targets which are part of official OpenDeck release
    tests       Builds all tests"

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
btldr=()
release=()
test=()

for config in ../targets/*.yml;
do
    targets+=("$(basename "$config" .yml)")
    release+=("$(yq r "$config" release)")
    test+=("$(yq r "$config" test)")
    btldr+=("$(yq r "$config" bootloader.use)")
done

len_targets=${#targets[@]}

if [[ "$BUILD_RELEASE" == "true" ]]
then
    make clean
fi

for (( i=0; i<len_targets; i++ ))
do
    if [[ "$TYPE" != "tests" ]]
    then
        if [[ "$BUILD_RELEASE" == "true" ]]
        then
            if [[ "${release[$i]}" == "false" ]]
            then
                continue;
            else
                make merged TARGETNAME="${targets[$i]}" DEBUG=0
                result=$?

                if [[ "$result" -eq 0 ]]
                then
                    #copy merged binary to bin directory
                    dir=../bin/compiled/merged/"$(make TARGETNAME="${targets[$i]}" print-ARCH)"/"$(make TARGETNAME="${targets[$i]}" print-MCU)"
                    mkdir -p "$dir"
                    cp "$(make TARGETNAME="${targets[$i]}" TYPE=boot print-MERGED_TARGET).hex" "$dir"
                fi

                if [[ ${targets[$i]} == "mega16u2" ]]
                then
                    #no need to create sysex firmware for atmega16u2 - it's only a USB link
                    continue;
                fi

                #create sysex fw update file
                make sysexfw TARGETNAME="${targets[$i]}" TYPE=app
            fi
        fi
    else
        if [[ "${test[$i]}" == "false" ]]
        then
            continue;
        fi
    fi

    if [[ "$TYPE" == "tests" ]]
    then
        make pre-build TARGETNAME="${targets[$i]}" DEBUG=0
    else
    if [[ "${btldr[$i]}" == "true" ]]
    then
        make TARGETNAME="${targets[$i]}" TYPE=boot DEBUG=0

        result=$?

        if [[ ($result -ne 0) ]]
        then
            exit 1
        fi
    fi
    fi

    make TARGETNAME="${targets[$i]}" TYPE=app DEBUG=0

    result=$?

    if [[ ($result -ne 0) ]]
    then
        exit 1
    fi
done