#!/usr/bin/env bash

YAML_FILE=$1
GEN_DIR=$2
YAML_PARSER="dasel -n -p yaml --plain -f"
OUT_MAKEFILE="$GEN_DIR"/Arch.mk

echo "Generating arch definitions..."

mkdir -p "$GEN_DIR"
echo "" > "$OUT_MAKEFILE"

total_symbols=$($YAML_PARSER "$YAML_FILE" symbols --length)

for ((i=0; i<total_symbols; i++))
do
    symbol=$($YAML_PARSER "$YAML_FILE" symbols.["$i"])
    printf "%s\n" "DEFINES += $symbol" >> "$OUT_MAKEFILE"
done