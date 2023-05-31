#!/usr/bin/env bash

run_dir="OpenDeck"

if [[ $(pwd) != *"$run_dir" ]]
then
    echo This script must be run from $run_dir directory!
    exit 1
fi

# Run linting for several targets so that most of the codebase is covered

set -e

clang_tidy_lint()
{
    echo "Linting firmware code with clang-tidy for target: $1"
    dir=src
    cd $dir
    build_cmd="make TARGET=$1 DEBUG=0"
    eval "$build_cmd" clean
    eval "$build_cmd"
    eval compiledb "$build_cmd" binary TYPE=application
    eval "$build_cmd" lint TYPE=application CL_FAIL_ON_DIFF=1
    rm compile_commands.json
    eval compiledb "$build_cmd" binary TYPE=bootloader
    eval "$build_cmd" lint TYPE=bootloader CL_FAIL_ON_DIFF=1
    rm compile_commands.json
    eval compiledb "$build_cmd" binary TYPE=sysexgen
    eval "$build_cmd" lint TYPE=sysexgen CL_FAIL_ON_DIFF=1
    rm compile_commands.json

    if [[ $2 -ne 0 ]]
    then
        eval compiledb "$build_cmd" binary TYPE=flashgen
        eval "$build_cmd" lint TYPE=flashgen CL_FAIL_ON_DIFF=1
        rm compile_commands.json
    fi

    cd ../
}

clang_tidy_lint discovery
clang_tidy_lint opendeck2
clang_tidy_lint rooibos
clang_tidy_lint nrf52840dk
clang_tidy_lint mega2560 0
clang_tidy_lint mega16u2 0
clang_tidy_lint opendeck_mini 0
clang_tidy_lint teensy2pp 0