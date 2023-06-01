#!/usr/bin/env bash

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

# merged.hex, merged.bin, merged.uf2 and firmware.sysex files are needed

readarray -t hex_files < <(find "$build_dir" -type f -path "*release/*" -name "*merged.hex")
readarray -t bin_files < <(find "$build_dir" -type f -path "*release/*" -name "*merged.bin")
readarray -t uf2_files < <(find "$build_dir" -type f -path "*release/*" -name "*merged.uf2")
readarray -t sysex_files < <(find "$build_dir" -type f -path "*release/*" -name "*firmware.sysex")

for hex in "${hex_files[@]}"
do
    target=$(basename "$(dirname "$(dirname "$hex")")")
    cp "$hex" "$copy_dir"/"$target".hex
done

for bin in "${bin_files[@]}"
do
    target=$(basename "$(dirname "$(dirname "$bin")")")
    cp "$bin" "$copy_dir"/"$target".bin
done

for uf2 in "${uf2_files[@]}"
do
    target=$(basename "$(dirname "$(dirname "$uf2")")")
    cp "$uf2" "$copy_dir"/"$target".uf2
done

for sysex in "${sysex_files[@]}"
do
    # SysEx file is in subdir
    target=$(basename "$(dirname "$(dirname "$(dirname "$sysex")")")")
    cp "$sysex" "$copy_dir"/"$target".sysex
done