#!/usr/bin/env bash

if [[ $($yaml_parser "$project_yaml_file" usb) != "null" ]]
then
    if [[ $($yaml_parser "$project_yaml_file" usb.endpoints.size) != "null" ]]
    then
        usb_endpoint_size_control=$($yaml_parser "$project_yaml_file" usb.endpoints.size.control)
        usb_endpoint_size_midi_in_out=$($yaml_parser "$project_yaml_file" usb.endpoints.size.midi-in-out)
        usb_endpoint_size_cdc_in_out=$($yaml_parser "$project_yaml_file" usb.endpoints.size.cdc-in-out)
        usb_endpoint_size_cdc_notification=$($yaml_parser "$project_yaml_file" usb.endpoints.size.cdc-notification)

        {
            printf "%s\n" "PROJECT_MCU_DEFINES += PROJECT_MCU_USB_ENDPOINT_SIZE_CONTROL=$usb_endpoint_size_control"
            printf "%s\n" "PROJECT_MCU_DEFINES += PROJECT_MCU_USB_ENDPOINT_SIZE_MIDI_IN_OUT=$usb_endpoint_size_midi_in_out"
            printf "%s\n" "PROJECT_MCU_DEFINES += PROJECT_MCU_USB_ENDPOINT_SIZE_CDC_IN_OUT=$usb_endpoint_size_cdc_in_out"
            printf "%s\n" "PROJECT_MCU_DEFINES += PROJECT_MCU_USB_ENDPOINT_SIZE_CDC_NOTIFICATION=$usb_endpoint_size_cdc_notification"
        } >> "$out_makefile"

        # tinyusb only
        {
            printf "%s\n" "PROJECT_MCU_DEFINES += CFG_TUD_ENDPOINT0_SIZE=$usb_endpoint_size_control"
            printf "%s\n" "PROJECT_MCU_DEFINES += CFG_TUD_MIDI_EP_BUFSIZE=$usb_endpoint_size_midi_in_out"
            printf "%s\n" "PROJECT_MCU_DEFINES += CFG_TUD_CDC_EP_BUFSIZE=$usb_endpoint_size_cdc_in_out"
        } >> "$out_makefile"
    fi

    if [[ $($yaml_parser "$project_yaml_file" usb.endpoints.midi) != "null" ]]
    then
        usb_endpoint_midi_addr_in=$($yaml_parser "$project_yaml_file" usb.endpoints.midi.in)
        usb_endpoint_midi_addr_out=$($yaml_parser "$project_yaml_file" usb.endpoints.midi.out)

        {
            printf "%s\n" "PROJECT_MCU_DEFINES += PROJECT_MCU_USB_ENDPOINT_MIDI_ADDR_IN=$usb_endpoint_midi_addr_in"
            printf "%s\n" "PROJECT_MCU_DEFINES += PROJECT_MCU_USB_ENDPOINT_MIDI_ADDR_OUT=$usb_endpoint_midi_addr_out"
        } >> "$out_makefile"
    fi

    if [[ $($yaml_parser "$project_yaml_file" usb.endpoints.midi-cdc-dual) != "null" ]]
    then
        usb_endpoint_midi_cdc_dual_addr_cdc_in=$($yaml_parser "$project_yaml_file" usb.endpoints.midi-cdc-dual.cdc-in)
        usb_endpoint_midi_cdc_dual_addr_cdc_out=$($yaml_parser "$project_yaml_file" usb.endpoints.midi-cdc-dual.cdc-out)
        usb_endpoint_midi_cdc_dual_addr_cdc_notification=$($yaml_parser "$project_yaml_file" usb.endpoints.midi-cdc-dual.cdc-notification)
        usb_endpoint_midi_cdc_dual_addr_midi_in=$($yaml_parser "$project_yaml_file" usb.endpoints.midi-cdc-dual.midi-in)
        usb_endpoint_midi_cdc_dual_addr_midi_out=$($yaml_parser "$project_yaml_file" usb.endpoints.midi-cdc-dual.midi-out)

        {
            printf "%s\n" "PROJECT_MCU_DEFINES += PROJECT_MCU_USB_ENDPOINT_MIDI_CDC_DUAL_ADDR_CDC_IN=$usb_endpoint_midi_cdc_dual_addr_cdc_in"
            printf "%s\n" "PROJECT_MCU_DEFINES += PROJECT_MCU_USB_ENDPOINT_MIDI_CDC_DUAL_ADDR_CDC_OUT=$usb_endpoint_midi_cdc_dual_addr_cdc_out"
            printf "%s\n" "PROJECT_MCU_DEFINES += PROJECT_MCU_USB_ENDPOINT_MIDI_CDC_DUAL_ADDR_CDC_NOTIFICATION=$usb_endpoint_midi_cdc_dual_addr_cdc_notification"
            printf "%s\n" "PROJECT_MCU_DEFINES += PROJECT_MCU_USB_ENDPOINT_MIDI_CDC_DUAL_ADDR_MIDI_IN=$usb_endpoint_midi_cdc_dual_addr_midi_in"
            printf "%s\n" "PROJECT_MCU_DEFINES += PROJECT_MCU_USB_ENDPOINT_MIDI_CDC_DUAL_ADDR_MIDI_OUT=$usb_endpoint_midi_cdc_dual_addr_midi_out"
        } >> "$out_makefile"
    fi

    if [[ $($yaml_parser "$project_yaml_file" usb.buffers) != "null" ]]
    then
        usb_buffer_midi_tx=$($yaml_parser "$project_yaml_file" usb.buffers.midi.tx)
        usb_buffer_midi_rx=$($yaml_parser "$project_yaml_file" usb.buffers.midi.rx)
        usb_buffer_cdc_tx=$($yaml_parser "$project_yaml_file" usb.buffers.cdc.tx)
        usb_buffer_cdc_rx=$($yaml_parser "$project_yaml_file" usb.buffers.cdc.rx)

        {
            printf "%s\n" "PROJECT_MCU_DEFINES += PROJECT_MCU_USB_BUFFER_SIZE_MIDI_TX=$usb_buffer_midi_tx"
            printf "%s\n" "PROJECT_MCU_DEFINES += PROJECT_MCU_USB_BUFFER_SIZE_MIDI_RX=$usb_buffer_midi_rx"
            printf "%s\n" "PROJECT_MCU_DEFINES += PROJECT_MCU_USB_BUFFER_SIZE_CDC_TX=$usb_buffer_cdc_tx"
            printf "%s\n" "PROJECT_MCU_DEFINES += PROJECT_MCU_USB_BUFFER_SIZE_CDC_RX=$usb_buffer_cdc_rx"
        } >> "$out_makefile"

        # tinyusb only
        {
            printf "%s\n" "PROJECT_MCU_DEFINES += CFG_TUD_MIDI_TX_BUFSIZE=$usb_buffer_midi_tx"
            printf "%s\n" "PROJECT_MCU_DEFINES += CFG_TUD_MIDI_RX_BUFSIZE=$usb_buffer_midi_rx"
            printf "%s\n" "PROJECT_MCU_DEFINES += CFG_TUD_CDC_TX_BUFSIZE=$usb_buffer_cdc_tx"
            printf "%s\n" "PROJECT_MCU_DEFINES += CFG_TUD_CDC_RX_BUFSIZE=$usb_buffer_cdc_rx"
        } >> "$out_makefile"
    fi
fi