#!/usr/bin/env bash

if [[ $($yaml_parser "$project_yaml_file" ble) != "null" ]]
then
    if [[ $($yaml_parser "$project_yaml_file" ble.buffers.midi) != "null" ]]
    then
        ble_buffer_midi=$($yaml_parser "$project_yaml_file" ble.buffers.midi)

        {
            printf "%s\n" "DEFINES += BUFFER_SIZE_BLE_MIDI_PACKET=$ble_buffer_midi"
        } >> "$out_makefile"
    fi
fi