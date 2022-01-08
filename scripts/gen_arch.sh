#!/usr/bin/env bash

ARCH_DEF_FILE=$1
GEN_DIR=$2
YAML_PARSER="dasel -n -p yaml --plain -f"
OUT_FILE_MAKEFILE="$GEN_DIR"/Arch.mk

mkdir -p "$GEN_DIR"

echo "" > "$OUT_FILE_MAKEFILE"
total_symbols=$($YAML_PARSER "$ARCH_DEF_FILE" symbols --length)

for ((i=0; i<total_symbols; i++))
do
    symbol=$($YAML_PARSER "$ARCH_DEF_FILE" symbols.["$i"])
    printf "%s\n" "DEFINES += $symbol" >> "$OUT_FILE_MAKEFILE"
done