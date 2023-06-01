#!/usr/bin/env bash

set -e

mcu=$1

pyocd load -u "${PROBE_ID}" -t "$mcu" "$PWD"/merged.hex