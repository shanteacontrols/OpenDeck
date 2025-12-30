#!/usr/bin/env bash

set -e

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
project_root="$(realpath "${script_dir}"/../..)"
yaml_parser="dasel -n -p yaml --plain -f"
target=$1
target_config=$project_root/config/target/$target.yml
mcu=$($yaml_parser "$target_config" mcu)

if [[ -n $PROBE_ID ]]
then
    pyocd load -u "${PROBE_ID}" -t "$mcu" "$PWD"/merged.hex
else
    pyocd load -t "$mcu" "$PWD"/merged.hex
fi
