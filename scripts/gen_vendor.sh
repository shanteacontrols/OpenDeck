#!/usr/bin/env bash

VENDOR_DEF_FILE=$1
GEN_DIR=$2
YAML_PARSER="dasel -n -p yaml --plain -f"
OUT_FILE_MAKEFILE="$GEN_DIR"/Vendor.mk

mkdir -p "$GEN_DIR"

echo "" > "$OUT_FILE_MAKEFILE"

if [[ $($YAML_PARSER "$VENDOR_DEF_FILE" symbols.common) != "null" ]]
then
    total_symbols=$($YAML_PARSER "$VENDOR_DEF_FILE" symbols.common --length)

    for ((i=0; i<total_symbols; i++))
    do
        symbol=$($YAML_PARSER "$VENDOR_DEF_FILE" symbols.common.["$i"])
        printf "%s\n" "DEFINES += $symbol" >> "$OUT_FILE_MAKEFILE"
    done
fi

if [[ $($YAML_PARSER "$VENDOR_DEF_FILE" symbols.app) != "null" ]]
then
    total_symbols=$($YAML_PARSER "$VENDOR_DEF_FILE" symbols.app --length)

    printf "%s\n" 'ifeq ($(TYPE),app)' >> "$OUT_FILE_MAKEFILE"

    for ((i=0; i<total_symbols; i++))
    do
        symbol=$($YAML_PARSER "$VENDOR_DEF_FILE" symbols.app.["$i"])
        printf "%s\n" "DEFINES += $symbol" >> "$OUT_FILE_MAKEFILE"
    done

    printf "%s\n" 'endif' >> "$OUT_FILE_MAKEFILE"
fi

if [[ $($YAML_PARSER "$VENDOR_DEF_FILE" symbols.boot) != "null" ]]
then
    total_symbols=$($YAML_PARSER "$VENDOR_DEF_FILE" symbols.boot --length)

    printf "%s\n" 'ifeq ($(TYPE),boot)' >> "$OUT_FILE_MAKEFILE"

    for ((i=0; i<total_symbols; i++))
    do
        symbol=$($YAML_PARSER "$VENDOR_DEF_FILE" symbols.boot.["$i"])
        printf "%s\n" "DEFINES += $symbol" >> "$OUT_FILE_MAKEFILE"
    done

    printf "%s\n" 'endif' >> "$OUT_FILE_MAKEFILE"
fi