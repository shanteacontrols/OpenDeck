#!/usr/bin/env bash

# STM32 Programmer utility on SWD interface

set -e

STM32_Programmer_CLI -c port=SWD sn="${PROBE_ID}" -w "$PWD"/merged.hex -rst