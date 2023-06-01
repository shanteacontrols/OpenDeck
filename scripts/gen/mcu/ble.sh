#!/usr/bin/env bash

if [[ $($yaml_parser "$project_yaml_file" ble) != "null" ]]
then
    if [[ $($yaml_parser "$project_yaml_file" ble.buffers.midi) != "null" ]]
    then
        ble_buffer_midi=$($yaml_parser "$project_yaml_file" ble.buffers.midi)

        printf "%s\n" "list(APPEND $cmake_mcu_defines_var PROJECT_MCU_BUFFER_SIZE_BLE_MIDI_PACKET=$ble_buffer_midi)" >> "$out_cmakelists"
    fi
fi