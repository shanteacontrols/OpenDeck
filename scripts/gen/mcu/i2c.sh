#!/usr/bin/env bash

if [[ $($yaml_parser "$project_yaml_file" i2c) != "null" ]]
then
    if [[ $($yaml_parser "$project_yaml_file" i2c.buffers) != "null" ]]
    then
        i2c_buffer_tx=$($yaml_parser "$project_yaml_file" i2c.buffers.tx)
        i2c_buffer_rx=$($yaml_parser "$project_yaml_file" i2c.buffers.rx)

        {
            printf "%s\n" "list(APPEND $cmake_mcu_defines_var PROJECT_MCU_BUFFER_SIZE_I2C_TX=$i2c_buffer_tx)"
            printf "%s\n" "list(APPEND $cmake_mcu_defines_var PROJECT_MCU_BUFFER_SIZE_I2C_RX=$i2c_buffer_rx)"
        } >> "$out_cmakelists"
    fi
fi