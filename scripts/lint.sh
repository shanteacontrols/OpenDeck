#!/usr/bin/env bash

# Run linting for several targets so that most of the codebase is covered

set -e

make clean

lint_target()
{
    echo "Linting for target: $1"
    make TARGET="$1"
    compiledb make TARGET="$1" binary TYPE=app
    make lint TARGET="$1" TYPE=app CL_FAIL_ON_DIFF=1
    rm -f compile_commands.json
    compiledb make TARGET="$1" binary TYPE=boot
    make lint TARGET="$1" TYPE=boot CL_FAIL_ON_DIFF=1
    rm -f compile_commands.json
    compiledb make TARGET="$1" binary TYPE=sysexgen
    make lint TARGET="$1" TYPE=sysexgen CL_FAIL_ON_DIFF=1
    rm -f compile_commands.json

    if [[ $2 -ne 0 ]]
    then
        compiledb make TARGET="$1" binary TYPE=flashgen
        make lint TARGET="$1" TYPE=flashgen CL_FAIL_ON_DIFF=1
        rm -f compile_commands.json
    fi
}

lint_target discovery
lint_target opendeck2
lint_target rooibos
lint_target nrf52840dk
lint_target mega2560 0
lint_target mega16u2 0
lint_target opendeck_mini 0
lint_target teensy2pp 0