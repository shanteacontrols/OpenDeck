#!/usr/bin/env bash

if [[ $($yaml_parser "$project_yaml_file" uart) != "null" ]]
then
    if [[ $($yaml_parser "$project_yaml_file" uart.buffers) != "null" ]]
    then
        uart_buffer_tx=$($yaml_parser "$project_yaml_file" uart.buffers.tx)
        uart_buffer_rx=$($yaml_parser "$project_yaml_file" uart.buffers.rx)

        {
            printf "%s\n" "list(APPEND $cmake_mcu_defines_var PROJECT_MCU_BUFFER_SIZE_UART_TX=$uart_buffer_tx)"
            printf "%s\n" "list(APPEND $cmake_mcu_defines_var PROJECT_MCU_BUFFER_SIZE_UART_RX=$uart_buffer_rx)"
        } >> "$out_cmakelists"
    fi
fi