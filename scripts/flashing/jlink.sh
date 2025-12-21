#!/usr/bin/env bash

# JLink on SWD interface

set -e

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
project_root="$(realpath "${script_dir}"/../..)"
yaml_parser="dasel -n -p yaml --plain -f"
target=$1
target_config=$project_root/config/target/$target.yml
mcu=$($yaml_parser "$target_config" mcu)

printf "r\nh\nloadfile %s\nr\ng\nqc\n" "$PWD/merged.hex" | \
JLinkExe -device "$mcu" -if SWD -speed 4000 -autoconnect 1