#!/usr/bin/env bash

if [[ $($yaml_parser "$project_yaml_file" usb) != "null" ]]
then
    if [[ $($yaml_parser "$project_yaml_file" usb.endpoints.size) != "null" ]]
    then
        usb_endpoint_size_control=$($yaml_parser "$project_yaml_file" usb.endpoints.size.control)
        usb_endpoint_size_midi_in_out=$($yaml_parser "$project_yaml_file" usb.endpoints.size.midi-in-out)

        {
            printf "%s\n" "list(APPEND $cmake_usb_defines_var PROJECT_MCU_USB_ENDPOINT_SIZE_CONTROL=$usb_endpoint_size_control)"
            printf "%s\n" "list(APPEND $cmake_usb_defines_var PROJECT_MCU_USB_ENDPOINT_SIZE_MIDI_IN_OUT=$usb_endpoint_size_midi_in_out)"
        } >> "$out_cmakelists"

        # tinyusb only
        {
            printf "%s\n" "list(APPEND $cmake_usb_defines_var CFG_TUD_ENDPOINT0_SIZE=$usb_endpoint_size_control)"
            printf "%s\n" "list(APPEND $cmake_usb_defines_var CFG_TUD_MIDI_EP_BUFSIZE=$usb_endpoint_size_midi_in_out)"
        } >> "$out_cmakelists"
    fi

    if [[ $($yaml_parser "$project_yaml_file" usb.endpoints.midi) != "null" ]]
    then
        usb_endpoint_midi_addr_in=$($yaml_parser "$project_yaml_file" usb.endpoints.midi.in)
        usb_endpoint_midi_addr_out=$($yaml_parser "$project_yaml_file" usb.endpoints.midi.out)

        {
            printf "%s\n" "list(APPEND $cmake_usb_defines_var PROJECT_MCU_USB_ENDPOINT_MIDI_ADDR_IN=$usb_endpoint_midi_addr_in)"
            printf "%s\n" "list(APPEND $cmake_usb_defines_var PROJECT_MCU_USB_ENDPOINT_MIDI_ADDR_OUT=$usb_endpoint_midi_addr_out)"
        } >> "$out_cmakelists"
    fi

    if [[ $($yaml_parser "$project_yaml_file" usb.buffers) != "null" ]]
    then
        usb_buffer_midi_tx=$($yaml_parser "$project_yaml_file" usb.buffers.midi.tx)
        usb_buffer_midi_rx=$($yaml_parser "$project_yaml_file" usb.buffers.midi.rx)

        # tinyusb only
        {
            printf "%s\n" "list(APPEND $cmake_usb_defines_var CFG_TUD_MIDI_TX_BUFSIZE=$usb_buffer_midi_tx)"
            printf "%s\n" "list(APPEND $cmake_usb_defines_var CFG_TUD_MIDI_RX_BUFSIZE=$usb_buffer_midi_rx)"
        } >> "$out_cmakelists"
    fi
fi