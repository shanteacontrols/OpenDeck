#!/usr/bin/env bash

set -euo pipefail

if [[ $# -ne 1 ]]
then
    printf 'usage: gen_crc32.sh <file>\n' >&2
    exit 2
fi

file=$1

# gzip stores the reflected IEEE CRC32 of the uncompressed input in the first
# four bytes of its 8-byte trailer.
gzip -c "$file" | tail -c 8 | od -An -N4 -tu4 | tr -d ' '
