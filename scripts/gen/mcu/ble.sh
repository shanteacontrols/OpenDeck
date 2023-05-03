#!/usr/bin/env bash

if [[ $($yaml_parser "$project_yaml_file" ble) != "null" ]]
then
    if [[ $($yaml_parser "$project_yaml_file" ble.buffers.midi) != "null" ]]
    then
        ble_buffer_midi=$($yaml_parser "$project_yaml_file" ble.buffers.midi)

        {
            printf "%s\n" "PROJECT_MCU_DEFINES += PROJECT_MCU_BUFFER_SIZE_BLE_MIDI_PACKET=$ble_buffer_midi"
        } >> "$out_makefile"
    fi
fi