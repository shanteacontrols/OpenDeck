#!/bin/bash

#first argument should be path to the binary file
BIN_FILE=$1
#second argument should be path of the output file
SYSEX_FILE=$2

declare -i BYTES_PER_MESSAGE=32

MANUFACTURER_IDs="00 53 43"
FW_START_BYTES="00 55 00 55"

#variables in which low and high bytes will be stored after splitting
declare -i highByte=0
declare -i lowByte=0

#
# Convert single 14-bit value to high and low bytes (7-bit each).
# $1: 14-bit value to split.
#
function split14bit
{
    declare -i value=$1
    newHigh=$((value >> 8 & 255))
    newLow=$((value & 255))
    newHigh=$((newHigh << 1 & 127))

    if (( (newLow >> 7) & 1 ))
    then
        ((newHigh |= 1))
    else
        ((newHigh &= 254))
    fi

    (( newLow &= 127 ))

    highByte=$newHigh
    lowByte=$newLow
}

if [[ ($# -lt 2) ]]
then
    echo -e "
    ERROR: Please provide all arguments
    First argument should be path to the input binary file
    Second argument should be path of the output SysEx file"
    exit 1
fi

if [[ ! -f "$BIN_FILE" ]]
then
    echo "File $BIN_FILE doesn't exist"
    exit 1
fi

if [[ "$(command -v hexdump)" == "" ]]
then
    echo "ERROR: hexdump not installed"
    exit 1
fi

echo "F0 $MANUFACTURER_IDs $FW_START_BYTES F7" > "$SYSEX_FILE"

fw_size=$(wc -c < "$BIN_FILE")
printf '%s\n' "Firmware size is $fw_size bytes. Generating SysEx file, please wait..."

declare -a fw_size_array

fw_size_array[0]=$((fw_size >> 24 & 0xFF))
fw_size_array[1]=$((fw_size >> 16 & 0xFF))
fw_size_array[2]=$((fw_size >> 8  & 0xFF))
fw_size_array[3]=$((fw_size >> 0  & 0xFF))

printf "%s" "F0 $MANUFACTURER_IDs" >> "$SYSEX_FILE"

for fwSizeByte in "${fw_size_array[@]}"
do
    split14bit $fwSizeByte
    {
        printf " %02X" "$highByte"
        printf " %02X" "$lowByte"
    } >> "$SYSEX_FILE"
done

printf " %s\n" "F7" >> "$SYSEX_FILE"

#read binary one byte at the time
#split each byte into two bytes in order to able to send
#values larger than 127

declare -i byteCounter=0
declare -i lastByteSet=0

while IFS= read -r line
do
    if [[ $byteCounter -eq 0 ]]
    then
        printf "%s" "F0 $MANUFACTURER_IDs " >> "$SYSEX_FILE"
        ((lastByteSet=0))
    fi

    split14bit "$line"
    printf "%02X " "$highByte" >> "$SYSEX_FILE"
    printf "%02X " "$lowByte" >> "$SYSEX_FILE"

    ((byteCounter++))

    if [[ $byteCounter -eq $BYTES_PER_MESSAGE ]]
    then
        ((byteCounter=0))
        printf "%s\n" "F7" >> "$SYSEX_FILE"
        ((lastByteSet=1))
    fi
done < <( < "$BIN_FILE" hexdump -v -e '/1 "%d\n"')

if [[ $lastByteSet -eq 0 ]]
then
    printf "F7\n" >> "$SYSEX_FILE"
fi