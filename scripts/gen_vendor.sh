#!/usr/bin/env bash

YAML_FILE=$1
GEN_DIR=$2
YAML_PARSER="dasel -n -p yaml --plain -f"
OUT_MAKEFILE="$GEN_DIR"/Vendor.mk

echo "Generating vendor definitions..."

mkdir -p "$GEN_DIR"
echo "" > "$OUT_MAKEFILE"

if [[ $($YAML_PARSER "$YAML_FILE" symbols.common) != "null" ]]
then
    total_symbols=$($YAML_PARSER "$YAML_FILE" symbols.common --length)

    for ((i=0; i<total_symbols; i++))
    do
        symbol=$($YAML_PARSER "$YAML_FILE" symbols.common.["$i"])
        printf "%s\n" "DEFINES += $symbol" >> "$OUT_MAKEFILE"
    done
fi

if [[ $($YAML_PARSER "$YAML_FILE" symbols.app) != "null" ]]
then
    total_symbols=$($YAML_PARSER "$YAML_FILE" symbols.app --length)

    printf "%s\n" 'ifeq ($(TYPE),app)' >> "$OUT_MAKEFILE"

    for ((i=0; i<total_symbols; i++))
    do
        symbol=$($YAML_PARSER "$YAML_FILE" symbols.app.["$i"])
        printf "%s\n" "DEFINES += $symbol" >> "$OUT_MAKEFILE"
    done

    printf "%s\n" 'endif' >> "$OUT_MAKEFILE"
fi

if [[ $($YAML_PARSER "$YAML_FILE" symbols.boot) != "null" ]]
then
    total_symbols=$($YAML_PARSER "$YAML_FILE" symbols.boot --length)

    printf "%s\n" 'ifeq ($(TYPE),boot)' >> "$OUT_MAKEFILE"

    for ((i=0; i<total_symbols; i++))
    do
        symbol=$($YAML_PARSER "$YAML_FILE" symbols.boot.["$i"])
        printf "%s\n" "DEFINES += $symbol" >> "$OUT_MAKEFILE"
    done

    printf "%s\n" 'endif' >> "$OUT_MAKEFILE"
fi