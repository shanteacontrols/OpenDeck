#!/usr/bin/env bash

project=$1
yaml_file=$2
gen_dir=$3
yaml_parser="dasel -n -p yaml --plain -f"
target_name=$(basename "$yaml_file" .yml)
out_header="$gen_dir"/hw_test_defines.h

mkdir -p "$gen_dir"
echo "" > "$out_header"

echo "Generating HW test config..."

{
    printf "%s\n\n" "#pragma once"
    printf "%s\n" "std::string HW_TEST_USB_DEVICE_NAME_APP=\"$project | $target_name\";"
    printf "%s\n" "std::string HW_TEST_USB_DEVICE_NAME_BOOT=\"$project DFU | $target_name\";"
} >> "$out_header"

hw_controller=$($yaml_parser "$yaml_file" hw-controller)

if [[ $hw_controller == "true" ]]
then
    printf "%s\n" "#define HW_TEST_HW_CONTROLLER_SUPPORTED" >> "$out_header"
fi

if [[ $($yaml_parser "$yaml_file" flash) != "null" ]]
then
    flash_args=$($yaml_parser "$yaml_file" flash.args)

    {
        printf "%s\n" "#define HW_TEST_FLASHING_SUPPORTED"
        printf "%s\n" "std::string HW_TEST_FLASH_ARGS=\"$flash_args\";"
    } >> "$out_header"
fi

if [[ $($yaml_parser "$yaml_file" dinMidi) != "null" ]]
then
    in_din_midi_port=$($yaml_parser "$yaml_file" dinMidi.in)
    out_din_midi_port=$($yaml_parser "$yaml_file" dinMidi.out)

    {
        printf "%s\n" "#define HW_TEST_DIN_MIDI_SUPPORTED"
        printf "%s\n" "std::string HW_TEST_DIN_MIDI_IN_PORT=\"$in_din_midi_port\";"
        printf "%s\n" "std::string HW_TEST_DIN_MIDI_OUT_PORT=\"$out_din_midi_port\";"
    } >> "$out_header"
fi

usb_link_target=$($yaml_parser "$yaml_file" usbLinkTarget)

if [[ $usb_link_target != "null" ]]
then
    usb_link_yaml_file=$(dirname "$yaml_file")/$usb_link_target.yml
    usb_link_flash_args=$($yaml_parser "$usb_link_yaml_file" flash.args)

    {
        printf "%s\n" "std::string HW_TEST_FLASH_ARGS_USB_LINK=\"$usb_link_flash_args\";"
        printf "%s\n" "std::string HW_TEST_USB_LINK_TARGET=\"$usb_link_target\";"
    } >> "$out_header"
fi

if [[ $($yaml_parser "$yaml_file" io) != "null" ]]
then
    printf "%s\n" "#define HW_TEST_IO_SUPPORTED" >> "$out_header"

    declare -i nr_of_switches
    declare -i nr_of_analog
    declare -i nr_of_leds

    nr_of_switches=$($yaml_parser "$yaml_file" io.switches-pins --length)
    nr_of_analog=$($yaml_parser "$yaml_file" io.analog-pins --length)
    nr_of_leds=$($yaml_parser "$yaml_file" io.led-pins --length)

    printf "%s\n" "using hwTestDescriptor_t = struct { int pin; int index; };" >> "$out_header"

    if [[ $nr_of_switches -ne 0 ]]
    then
        {
            printf "%s\n" "#define HW_TEST_IO_SWITCHES_SUPPORTED"
            printf "%s\n" "std::vector<hwTestDescriptor_t> hwTestSwDescriptor = {"
        } >> "$out_header"

        for ((i=0; i<nr_of_switches; i++))
        do
            {
                printf "%s" "{ "
                printf "%s" "$($yaml_parser "$yaml_file" io.switches-pins.["$i"]),"
                printf "%s" "$($yaml_parser "$yaml_file" io.switches-id.["$i"])"
                printf "%s\n" "},"
            } >> "$out_header"
        done

        printf "%s\n" "};" >> "$out_header"
    fi

    if [[ $nr_of_analog -ne 0 ]]
    then
        {
            printf "%s\n" "#define HW_TEST_IO_ANALOG_SUPPORTED"
            printf "%s\n" "std::vector<hwTestDescriptor_t> hwTestAnalogDescriptor = {"
        } >> "$out_header"

        for ((i=0; i<nr_of_analog; i++))
        do
            {
                printf "%s" "{ "
                printf "%s" "$($yaml_parser "$yaml_file" io.analog-pins.["$i"]),"
                printf "%s" "$($yaml_parser "$yaml_file" io.analog-id.["$i"])"
                printf "%s\n" "},"
            } >> "$out_header"
        done

        printf "%s\n" "};" >> "$out_header"
    fi

    if [[ $nr_of_leds -ne 0 ]]
    then
        {
            printf "%s\n" "#define HW_TEST_IO_LEDS_SUPPORTED"
            printf "%s\n" "std::vector<hwTestDescriptor_t> hwTestLEDDescriptor = {"
        } >> "$out_header"

        for ((i=0; i<nr_of_leds; i++))
        do
            {
                printf "%s" "{ "
                printf "%s" "$($yaml_parser "$yaml_file" io.led-pins.["$i"]),"
                printf "%s" "$($yaml_parser "$yaml_file" io.led-id.["$i"])"
                printf "%s\n" "},"
            } >> "$out_header"
        done

        printf "%s\n" "};" >> "$out_header"
    fi
fi