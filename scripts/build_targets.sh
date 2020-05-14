#!/bin/bash

function usage
{
    echo -e "\nUsage: ./$(basename "$0") --type"

    echo -e "
    This script is used to build specific set of targets from targets.json file
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

TARGETS_FILE=../targets.json

targets=()
release=()
test=()

while IFS= read -r line
do
    targets+=( "$line" )
done < <( jq ".targets[] | .name" $TARGETS_FILE | tr -d \")

while IFS= read -r line
do
    release+=( "$line" )
done < <( jq ".targets[] | .release" $TARGETS_FILE)

while IFS= read -r line
do
    test+=( "$line" )
done < <( jq ".targets[] | .test" $TARGETS_FILE)

len_targets=${#targets[@]}
len_release=${#release[@]}
len_tests=${#test[@]}

if [[ ($len_targets -ne $len_release) || ($len_targets -ne $len_tests) ]]
then
    echo "Invalid JSON record"
    exit 1
fi

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
                #build merged binary
                make merged TARGETNAME="${targets[$i]}"
                result=$?

                if [[ "$result" -eq 0 ]]
                then
                    #copy merged binary to bin directory
                    dir=../bin/compiled/merged/"$(make TARGETNAME="${targets[$i]}" print-ARCH)"/"$(make TARGETNAME="${targets[$i]}" print-MCU)"
                    mkdir -p "$dir"
                    cp "$(make TARGETNAME="${targets[$i]}" BOOT=1 print-MERGED_TARGET)" "$dir"
                fi

                #always copy application to bin directory
                dir=../bin/compiled/fw/"$(make TARGETNAME="${targets[$i]}" print-ARCH)"/"$(make TARGETNAME="${targets[$i]}" print-MCU)"
                mkdir -p "$dir"
                cp "$(make TARGETNAME="${targets[$i]}" BOOT=0 print-TARGET)".hex "$dir"
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
        make pre-build TARGETNAME="${targets[$i]}"
    fi

    make TARGETNAME="${targets[$i]}"

    result=$?

    if [[ ($result -ne 0) ]]
    then
        exit 1
    fi
done