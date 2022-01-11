#!/usr/bin/env bash

YAML_FILE=$1
GEN_DIR=$2
YAML_PARSER="dasel -n -p yaml --plain -f"
TARGET_NAME=$(basename "$YAML_FILE" .yml)
OUT_HEADER="$GEN_DIR"/HWTestDefines.h

mkdir -p "$GEN_DIR"
echo "" > "$OUT_HEADER"

echo "Generating HW test config..."

{
    printf "%s\n\n" "#pragma once"
    printf "%s\n" "std::string OPENDECK_MIDI_DEVICE_NAME=\"OpenDeck | $TARGET_NAME\";"
    printf "%s\n" "std::string OPENDECK_DFU_MIDI_DEVICE_NAME=\"OpenDeck DFU | $TARGET_NAME\";"
} >> "$OUT_HEADER"

if [[ $($YAML_PARSER "$YAML_FILE" flash) != "null" ]]
then
    flash_args=$($YAML_PARSER "$YAML_FILE" flash.args)

    {
        printf "%s\n" "#define TEST_FLASHING"
        printf "%s\n" "std::string FLASH_ARGS=\"$flash_args\";"
    } >> "$OUT_HEADER"
fi

if [[ $($YAML_PARSER "$YAML_FILE" dinMidi) != "null" ]]
then
    in_din_midi_port=$($YAML_PARSER "$YAML_FILE" dinMidi.in)
    out_din_midi_port=$($YAML_PARSER "$YAML_FILE" dinMidi.out)

    {
        printf "%s\n" "#define TEST_DIN_MIDI"
        printf "%s\n" "std::string IN_DIN_MIDI_PORT=\"$in_din_midi_port\";"
        printf "%s\n" "std::string OUT_DIN_MIDI_PORT=\"$out_din_midi_port\";"
    } >> "$OUT_HEADER"
fi

usb_link_target=$($YAML_PARSER "$YAML_FILE" usbLinkTarget)

if [[ $usb_link_target != "null" ]]
then
    USB_LINK_YAML_FILE=$(dirname "$YAML_FILE")/$usb_link_target.yml
    usb_link_flash_args=$($YAML_PARSER "$USB_LINK_YAML_FILE" flash.args)

    {
        printf "%s\n" "std::string FLASH_ARGS_USB_LINK=\"$usb_link_flash_args\";"
        printf "%s\n" "std::string USB_LINK_TARGET=\"$usb_link_target\";"
    } >> "$OUT_HEADER"
fi

if [[ $($YAML_PARSER "$YAML_FILE" io) != "null" ]]
then
    printf "%s\n" "#define TEST_IO" >> "$OUT_HEADER"

    declare -i nr_of_switches
    declare -i nr_of_analog
    declare -i nr_of_leds

    nr_of_switches=$($YAML_PARSER "$YAML_FILE" io.switches-pins --length)
    nr_of_analog=$($YAML_PARSER "$YAML_FILE" io.analog-pins --length)
    nr_of_leds=$($YAML_PARSER "$YAML_FILE" io.led-pins --length)

    printf "%s\n" "using hwTestDescriptor_t = struct { int pin; int index; };" >> "$OUT_HEADER"

    if [[ $nr_of_switches -ne 0 ]]
    then
        {
            printf "%s\n" "#define TEST_IO_SWITCHES"
            printf "%s\n" "std::vector<hwTestDescriptor_t> hwTestSwDescriptor = {"
        } >> "$OUT_HEADER"

        for ((i=0; i<nr_of_switches; i++))
        do
            {
                printf "%s" "{ "
                printf "%s" "$($YAML_PARSER "$YAML_FILE" io.switches-pins.["$i"]),"
                printf "%s" "$($YAML_PARSER "$YAML_FILE" io.switches-id.["$i"])"
                printf "%s\n" "},"
            } >> "$OUT_HEADER"
        done

        printf "%s\n" "};" >> "$OUT_HEADER"
    fi

    if [[ $nr_of_analog -ne 0 ]]
    then
        {
            printf "%s\n" "#define TEST_IO_ANALOG"
            printf "%s\n" "std::vector<hwTestDescriptor_t> hwTestAnalogDescriptor = {"
        } >> "$OUT_HEADER"

        for ((i=0; i<nr_of_analog; i++))
        do
            {
                printf "%s" "{ "
                printf "%s" "$($YAML_PARSER "$YAML_FILE" io.analog-pins.["$i"]),"
                printf "%s" "$($YAML_PARSER "$YAML_FILE" io.analog-id.["$i"])"
                printf "%s\n" "},"
            } >> "$OUT_HEADER"
        done

        printf "%s\n" "};" >> "$OUT_HEADER"
    fi

    if [[ $nr_of_leds -ne 0 ]]
    then
        {
            printf "%s\n" "#define TEST_IO_LEDS"
            printf "%s\n" "std::vector<hwTestDescriptor_t> hwTestLEDDescriptor = {"
        } >> "$OUT_HEADER"

        for ((i=0; i<nr_of_leds; i++))
        do
            {
                printf "%s" "{ "
                printf "%s" "$($YAML_PARSER "$YAML_FILE" io.led-pins.["$i"]),"
                printf "%s" "$($YAML_PARSER "$YAML_FILE" io.led-id.["$i"])"
                printf "%s\n" "},"
            } >> "$OUT_HEADER"
        done

        printf "%s\n" "};" >> "$OUT_HEADER"
    fi
fi