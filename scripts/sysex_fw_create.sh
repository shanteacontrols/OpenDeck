#!/bin/bash

#first argument should be path to the binary file
BIN_FILE=$1
#second argument should be path of the output file
SYSEX_FILE=$2
#third argument is sysex manufacturer id 0 (uint8)
M_ID_0=$3
#fourth argument is sysex manufacturer id 1 (uint8)
M_ID_1=$4
#fifth argument is sysex manufacturer id 2 (uint8)
M_ID_2=$5
#sixth argument is start command (uint32)
START_COMMAND=$6
#seventh argument is end command (uint32)
END_COMMAND=$7
#eight argument is board UID
BOARD_UID=$8

declare -i BYTES_PER_FW_MESSAGE=32

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

function append_command
{
    command=$1
    bytes_per_command_message=$2

    for ((i=0; i<2; i++))
    do
        {
            printf "F0"
            printf " %02X" "$M_ID_0"
            printf " %02X" "$M_ID_1"
            printf " %02X" "$M_ID_2"
        } >> "$SYSEX_FILE"

        unset start_command_array

        if [[ $bytes_per_command_message -eq 2 ]]
        then
            shift_amount=$((16*i))

            start_command_array[0]=$((command >> (shift_amount + 0) & 0xFF))
            start_command_array[1]=$((command >> (shift_amount + 8) & 0xFF))
        elif [[ $bytes_per_command_message -eq 4 ]]
        then
            shift_amount=$((32*i))

            start_command_array[0]=$((command >> (shift_amount + 0) & 0xFF))
            start_command_array[1]=$((command >> (shift_amount + 8) & 0xFF))
            start_command_array[2]=$((command >> (shift_amount + 16) & 0xFF))
            start_command_array[3]=$((command >> (shift_amount + 24) & 0xFF))
        else
            echo "Incorrect number of bytes per command message specified"
            exit 1
        fi

        for byte in "${start_command_array[@]}"
        do
            split14bit $byte
            {
                printf " %02X" "$highByte"
                printf " %02X" "$lowByte"
            } >> "$SYSEX_FILE"
        done

        printf "%s\n" " F7" >> "$SYSEX_FILE"
    done
}

printf "%s" "" > "$SYSEX_FILE"

fw_size=$(wc -c < "$BIN_FILE")
printf '%s\n' "Firmware size is $fw_size bytes. Generating SysEx file, please wait..."

declare -a fw_size_array

append_command "$START_COMMAND" 4

{
    printf "F0"
    printf " %02X" "$M_ID_0"
    printf " %02X" "$M_ID_1"
    printf " %02X" "$M_ID_2"
} >> "$SYSEX_FILE"

fw_size_array[0]=$((fw_size >> 0  & 0xFF))
fw_size_array[1]=$((fw_size >> 8  & 0xFF))
fw_size_array[2]=$((fw_size >> 16 & 0xFF))
fw_size_array[3]=$((fw_size >> 24 & 0xFF))

uid_array[0]=$((BOARD_UID >> 0  & 0xFF))
uid_array[1]=$((BOARD_UID >> 8  & 0xFF))
uid_array[2]=$((BOARD_UID >> 16  & 0xFF))
uid_array[3]=$((BOARD_UID >> 24  & 0xFF))

for fwSizeByte in "${fw_size_array[@]}"
do
    split14bit $fwSizeByte
    {
        printf " %02X" "$highByte"
        printf " %02X" "$lowByte"
    } >> "$SYSEX_FILE"
done

for uidByte in "${uid_array[@]}"
do
    split14bit $uidByte
    {
        printf " %02X" "$highByte"
        printf " %02X" "$lowByte"
    } >> "$SYSEX_FILE"
done

printf "%s\n" " F7" >> "$SYSEX_FILE"

#read binary one byte at the time
#split each byte into two bytes in order to able to send
#values larger than 127

declare -i byteCounter=0
declare -i lastByteSet=0

while IFS= read -r line
do
    if [[ $byteCounter -eq 0 ]]
    then
        {
            printf "F0"
            printf " %02X" "$M_ID_0"
            printf " %02X" "$M_ID_1"
            printf " %02X" "$M_ID_2"
        } >> "$SYSEX_FILE"
        ((lastByteSet=0))
    fi

    split14bit "$line"
    printf " %02X" "$highByte" >> "$SYSEX_FILE"
    printf " %02X" "$lowByte" >> "$SYSEX_FILE"

    ((byteCounter++))

    if [[ $byteCounter -eq $BYTES_PER_FW_MESSAGE ]]
    then
        ((byteCounter=0))
        printf "%s\n" " F7" >> "$SYSEX_FILE"
        ((lastByteSet=1))
    fi
done < <( < "$BIN_FILE" hexdump -v -e '/1 "%d\n"')

if [[ $lastByteSet -eq 0 ]]
then
    printf " F7\n" >> "$SYSEX_FILE"
fi

append_command "$END_COMMAND" 2

#created .sysex file is used for firmware update through web UI
#create another .syx file used for testing purposes with amidi tool

#remove all newlines first
xargs echo < $SYSEX_FILE > $SYSEX_FILE.temp
#reverse hexdump - write all ascii bytes as raw bytes into the new file
cat $SYSEX_FILE.temp | xxd -r -p > $SYSEX_FILE.syx
rm $SYSEX_FILE.temp