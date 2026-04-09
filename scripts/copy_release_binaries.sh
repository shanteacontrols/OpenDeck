#!/usr/bin/env bash

# Collect OpenDeck release artifacts from the current build tree into one flat
# directory. The script discovers target build roots by looking for release
# artifacts under build/app/<preset>/<build-type>/<target>/.
#
# Copied files are named after the OpenDeck target:
#
#   <target>.hex      merged bootloader + validated app image
#   <target>.bin      binary form of the merged image
#   <target>.uf2      merged UF2 image, when generated
#   <target>.dfu.bin  WebUSB bootloader update package
#
# Only merged UF2 files are copied. Raw Zephyr outputs such as
# app/zephyr/zephyr.uf2 are intermediate build products and are intentionally
# ignored because they do not represent the full OpenDeck release image.
#
# Options:
#
#   --build-dir=<dir>  build tree root, defaults to build
#   --copy-dir=<dir>   output directory, defaults to release

set -euo pipefail

build_dir=build
copy_dir=release

for arg in "$@"; do
    case "$arg" in
        --build-dir=*)
            build_dir=${arg#--build-dir=}
            ;;

        --copy-dir=*)
            copy_dir=${arg#--copy-dir=}
            ;;
    esac
done

mkdir -p "$copy_dir"

declare -A release_dirs=()

while IFS= read -r artifact_file
do
    case "$artifact_file" in
        */app/dfu.bin)
            release_dirs["$(dirname "$(dirname "$artifact_file")")"]=1
            ;;

        *)
            release_dirs["$(dirname "$artifact_file")"]=1
            ;;
    esac
done < <(
    find "$build_dir/app" -type f \
        \( \
            -name merged.hex \
            -o -name merged.bin \
            -o -name merged.uf2 \
            -o -path '*/app/dfu.bin' \
        \) 2>/dev/null | sort
)

if (( ${#release_dirs[@]} == 0 ))
then
    exit 0
fi

readarray -t sorted_release_dirs < <(printf '%s\n' "${!release_dirs[@]}" | sort)

for release_dir in "${sorted_release_dirs[@]}"
do
    target=$(basename "$release_dir")
    uf2_file="${release_dir}/merged.uf2"
    hex_file="${release_dir}/merged.hex"
    bin_file="${release_dir}/merged.bin"
    dfu_file="${release_dir}/app/dfu.bin"

    if [[ -f "$uf2_file" ]]
    then
        cp "$uf2_file" "${copy_dir}/${target}.uf2"
    fi

    if [[ -f "$hex_file" ]]
    then
        cp "$hex_file" "${copy_dir}/${target}.hex"
    fi

    if [[ -f "$bin_file" ]]
    then
        cp "$bin_file" "${copy_dir}/${target}.bin"
    fi

    if [[ -f "$dfu_file" ]]
    then
        cp "$dfu_file" "${copy_dir}/${target}.dfu.bin"
    fi
done
