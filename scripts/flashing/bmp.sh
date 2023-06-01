#!/usr/bin/env bash

# Black Magic Probe

set -e

arm-none-eabi-gdb -nx --batch \
-ex "target extended-remote /dev/${PORT}" \
-ex "monitor swdp_scan" \
-ex "attach 1" \
-ex "load" \
-ex "compare-sections" \
-ex "kill" \
"$PWD"/merged.hex | tee "$PWD"/gdb_output

if grep -q "MIS-MATCHED" "$PWD"/gdb_output
then
    exit 1
fi