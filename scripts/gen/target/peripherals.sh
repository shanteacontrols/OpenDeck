#!/usr/bin/env bash

if [[ $($yaml_parser "$yaml_file" usb) == "true" ]]
then
    printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_SUPPORT_USB" >> "$out_makefile"

    {
        printf "%s\n" "#if defined(FW_APP)"
        printf "%s\n" "#define PROJECT_TARGET_USB_NAME CORE_MCU_USB_STRING(\"$project | $target_name_string\")"
        printf "%s\n" "#elif defined(FW_BOOT)"
        printf "%s\n" "#define PROJECT_TARGET_USB_NAME CORE_MCU_USB_STRING(\"$project DFU | $target_name_string\")"
        printf "%s\n" "#endif"
    } >> "$out_header"
fi

if [[ $($yaml_parser "$yaml_file" ble) == "true" ]]
then
    printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_SUPPORT_BLE" >> "$out_makefile"
    printf "%s\n" "#define PROJECT_TARGET_BLE_NAME \"$project BLE | $target_name_string\"" >> "$out_header"
fi

if [[ "$($yaml_parser "$yaml_file" uart)" != "null" ]]
then
    printf "%s\n" 'PROJECT_TARGET_DEFINES += PROJECT_TARGET_SUPPORT_UART' >> "$out_makefile"

    declare -A uartChannelArray
    declare -i total_uart_channels
    declare -i use_custom_uart_pins

    use_custom_uart_pins=0
    total_uart_channels=0

    if [[ "$($yaml_parser "$yaml_file" uart.usbLink)" != "null" ]]
    then
        if [[ "$($yaml_parser "$yaml_file" uart.usbLink.type)" != "null" ]]
        then
            uart_channel_usb_link=$($yaml_parser "$yaml_file" uart.usbLink.channel)
            uart_usb_link_pins=$($yaml_parser "$yaml_file" uart.usbLink.pins)

            if [[ $uart_channel_usb_link == "null" && $uart_usb_link_pins == "null" ]]
            then
                echo "USB link UART channel or pins left unspecified"
                exit 1
            fi

            if [[ $uart_channel_usb_link != "null" ]]
            then
                printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_UART_CHANNEL_USB_LINK=$uart_channel_usb_link" >> "$out_makefile"
            elif [[ $uart_usb_link_pins != "null" ]]
            then
                use_custom_uart_pins=1

                uart_usb_link_rx_port=$($yaml_parser "$yaml_file" uart.usbLink.pins.rx.port)
                uart_usb_link_rx_index=$($yaml_parser "$yaml_file" uart.usbLink.pins.rx.index)
                uart_usb_link_tx_port=$($yaml_parser "$yaml_file" uart.usbLink.pins.tx.port)
                uart_usb_link_tx_index=$($yaml_parser "$yaml_file" uart.usbLink.pins.tx.index)

                key=${uart_usb_link_rx_port}${uart_usb_link_rx_index}${uart_usb_link_tx_port}${uart_usb_link_tx_index}

                if [[ -z "${uartChannelArray[$key]}" ]]
                then
                    # Unique (non-existing) channel found
                    uartChannelArray[$key]=$total_uart_channels
                    ((total_uart_channels++))
                fi

                uart_channel_usb_link=${uartChannelArray[$key]}
                printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_UART_CHANNEL_USB_LINK=$uart_channel_usb_link" >> "$out_makefile"

                {
                    printf "%s\n" "#define PIN_PORT_UART_CHANNEL_${uart_channel_usb_link}_RX CORE_MCU_IO_PIN_PORT_DEF(${uart_usb_link_rx_port})"
                    printf "%s\n" "#define PIN_INDEX_UART_CHANNEL_${uart_channel_usb_link}_RX CORE_MCU_IO_PIN_INDEX_DEF(${uart_usb_link_rx_index})"
                    printf "%s\n" "#define PIN_PORT_UART_CHANNEL_${uart_channel_usb_link}_TX CORE_MCU_IO_PIN_PORT_DEF(${uart_usb_link_tx_port})"
                    printf "%s\n" "#define PIN_INDEX_UART_CHANNEL_${uart_channel_usb_link}_TX CORE_MCU_IO_PIN_INDEX_DEF(${uart_usb_link_tx_index})"
                } >> "$out_header"
            fi

            if [[ "$($yaml_parser "$yaml_file" uart.usbLink.type)" == "host" ]]
            then
                {
                    printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_USB_OVER_SERIAL"
                    printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_USB_OVER_SERIAL_HOST"
                    printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_BOOTLOADER_NO_VERIFY_CRC"
                    printf "%s\n" "#append this only if it wasn't appended already"
                    printf "%s\n" 'ifeq (,$(findstring PROJECT_TARGET_SUPPORT_USB,$(DEFINES)))'
                    printf "%s\n" "    PROJECT_TARGET_DEFINES += PROJECT_TARGET_SUPPORT_USB"
                    printf "%s\n" "endif"
                } >> "$out_makefile"
            elif [[ "$($yaml_parser "$yaml_file" uart.usbLink.type)" == "device" ]]
            then
                # Make sure USB over serial devices don't have native USB enabled
                {
                    printf "%s\n" 'DEFINES := $(filter-out PROJECT_TARGET_SUPPORT_USB,$(DEFINES))'
                    printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_USB_OVER_SERIAL"
                    printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_USB_OVER_SERIAL_DEVICE"
                } >> "$out_makefile"
            fi
        fi
    fi

    if [[ "$($yaml_parser "$yaml_file" uart.dinMIDI)" != "null" ]]
    then
        printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_SUPPORT_DIN_MIDI" >> "$out_makefile"

        uart_channel_din_midi=$($yaml_parser "$yaml_file" uart.dinMIDI.channel)
        uart_din_mini_pins=$($yaml_parser "$yaml_file" uart.dinMIDI.pins)

        if [[ -n "$uart_channel_usb_link" ]]
        then
            if [[ $uart_channel_usb_link -eq $uart_channel_din_midi ]]
            then
                echo "USB link UART channel and DIN MIDI UART channel cannot be the same"
                exit 1
            fi
        fi

        if [[ $uart_channel_din_midi == "null" && $uart_din_mini_pins == "null" ]]
        then
            echo "DIN MIDI UART channel or pins left unspecified"
            exit 1
        fi

        if [[ $uart_channel_din_midi != "null" ]]
        then
            printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_UART_CHANNEL_DIN=$uart_channel_din_midi" >> "$out_makefile"
        elif [[ $uart_usb_link_pins != "null" ]]
        then
            use_custom_uart_pins=1

            uart_din_midi_rx_port=$($yaml_parser "$yaml_file" uart.dinMIDI.pins.rx.port)
            uart_din_midi_rx_index=$($yaml_parser "$yaml_file" uart.dinMIDI.pins.rx.index)
            uart_din_midi_tx_port=$($yaml_parser "$yaml_file" uart.dinMIDI.pins.tx.port)
            uart_din_midi_tx_index=$($yaml_parser "$yaml_file" uart.dinMIDI.pins.tx.index)

            key=${uart_din_midi_rx_port}${uart_din_midi_rx_index}${uart_din_midi_tx_port}${uart_din_midi_tx_index}

            if [[ -z "${uartChannelArray[$key]}" ]]
            then
                # Unique (non-existing) channel found
                uartChannelArray[$key]=$total_uart_channels
                ((total_uart_channels++))
            fi

            uart_channel_din_midi=${uartChannelArray[$key]}
            printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_UART_CHANNEL_DIN=$uart_channel_din_midi" >> "$out_makefile"

            {
                printf "%s\n" "#define PIN_PORT_UART_CHANNEL_${uart_channel_din_midi}_RX CORE_MCU_IO_PIN_PORT_DEF(${uart_din_midi_rx_port})"
                printf "%s\n" "#define PIN_INDEX_UART_CHANNEL_${uart_channel_din_midi}_RX CORE_MCU_IO_PIN_INDEX_DEF(${uart_din_midi_rx_index})"
                printf "%s\n" "#define PIN_PORT_UART_CHANNEL_${uart_channel_din_midi}_TX CORE_MCU_IO_PIN_PORT_DEF(${uart_din_midi_tx_port})"
                printf "%s\n" "#define PIN_INDEX_UART_CHANNEL_${uart_channel_din_midi}_TX CORE_MCU_IO_PIN_INDEX_DEF(${uart_din_midi_tx_index})"
            } >> "$out_header"
        fi
    fi

    if [[ "$($yaml_parser "$yaml_file" uart.touchscreen)" != "null" ]]
    then
        printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_SUPPORT_TOUCHSCREEN" >> "$out_makefile"

        uart_channel_touchscreen=$($yaml_parser "$yaml_file" uart.touchscreen.channel)
        uart_touchscreen_pins=$($yaml_parser "$yaml_file" uart.touchscreen.pins)

        if [[ -n "$uart_channel_usb_link" ]]
        then
            if [[ $uart_channel_usb_link -eq $uart_channel_touchscreen ]]
            then
                echo "USB link UART channel and touchscreen UART channel cannot be the same"
                exit 1
            fi
        fi

        if [[ $uart_channel_touchscreen == "null" && $uart_touchscreen_pins == "null" ]]
        then
            echo "Touchscreen UART channel or pins left unspecified"
            exit 1
        fi

        if [[ $uart_channel_touchscreen != "null" ]]
        then
            printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_UART_CHANNEL_TOUCHSCREEN=$uart_channel_touchscreen" >> "$out_makefile"
        elif [[ $uart_touchscreen_pins != "null" ]]
        then
            use_custom_uart_pins=1

            uart_touchscreen_rx_port=$($yaml_parser "$yaml_file" uart.touchscreen.pins.rx.port)
            uart_touchscreen_rx_index=$($yaml_parser "$yaml_file" uart.touchscreen.pins.rx.index)
            uart_touchscreen_tx_port=$($yaml_parser "$yaml_file" uart.touchscreen.pins.tx.port)
            uart_touchscreen_tx_index=$($yaml_parser "$yaml_file" uart.touchscreen.pins.tx.index)

            key=${uart_touchscreen_rx_port}${uart_touchscreen_rx_index}${uart_touchscreen_tx_port}${uart_touchscreen_tx_index}

            if [[ -z "${uartChannelArray[$key]}" ]]
            then
                # Unique (non-existing) channel found
                uartChannelArray[$key]=$total_uart_channels
                ((total_uart_channels++))
            fi

            uart_channel_touchscreen=${uartChannelArray[$key]}
            printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_UART_CHANNEL_TOUCHSCREEN=$uart_channel_touchscreen" >> "$out_makefile"

            {
                printf "%s\n" "#define PIN_PORT_UART_CHANNEL_${uart_channel_touchscreen}_RX CORE_MCU_IO_PIN_PORT_DEF(${uart_touchscreen_rx_port})"
                printf "%s\n" "#define PIN_INDEX_UART_CHANNEL_${uart_channel_touchscreen}_RX CORE_MCU_IO_PIN_INDEX_DEF(${uart_touchscreen_rx_index})"
                printf "%s\n" "#define PIN_PORT_UART_CHANNEL_${uart_channel_touchscreen}_TX CORE_MCU_IO_PIN_PORT_DEF(${uart_touchscreen_tx_port})"
                printf "%s\n" "#define PIN_INDEX_UART_CHANNEL_${uart_channel_touchscreen}_TX CORE_MCU_IO_PIN_INDEX_DEF(${uart_touchscreen_tx_index})"
            } >> "$out_header"
        fi

        # Guard against ommisions of touchscreen component amount by assigning the value to 0 if undefined
        nr_of_touchscreen_components=$($yaml_parser "$yaml_file" uart.touchscreen.components | grep -v null | awk '{print$1}END{if(NR==0)print 0}')

        if [[ "$nr_of_touchscreen_components" -eq 0 ]]
        then
            echo "Amount of touchscreen components cannot be 0 or undefined"
            exit 1
        fi

        printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_SUPPORTED_NR_OF_TOUCHSCREEN_COMPONENTS=$nr_of_touchscreen_components" >> "$out_makefile"
    else
        printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_SUPPORTED_NR_OF_TOUCHSCREEN_COMPONENTS=0" >> "$out_makefile"
    fi

    if [[ "$use_custom_uart_pins" -eq 1 ]]
    then
        {
            printf "%s\n" "namespace gen {"
            printf "%s\n" "constexpr inline core::mcu::uart::Config::pins_t UART_PINS[$total_uart_channels] = {"
        } >> "$out_header"

        for ((channel=0; channel<total_uart_channels;channel++))
        do
            {
                printf "%s\n" "{"
                printf "%s\n" "core::mcu::io::pin_t{PIN_PORT_UART_CHANNEL_${channel}_RX, PIN_INDEX_UART_CHANNEL_${channel}_RX},"
                printf "%s\n" "core::mcu::io::pin_t{PIN_PORT_UART_CHANNEL_${channel}_TX, PIN_INDEX_UART_CHANNEL_${channel}_TX},"
                printf "%s\n" "},"
            } >> "$out_header"
        done

        {
            printf "%s\n" "};"
            printf "%s\n" "}"
        } >> "$out_header"
    fi
else
    # Make sure this is set to 0 if uart/touchscreen isn't used as this symbol is used thorugh application IO modules
    printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_SUPPORTED_NR_OF_TOUCHSCREEN_COMPONENTS=0" >> "$out_makefile"
fi

if [[ "$($yaml_parser "$yaml_file" i2c)" != "null" ]]
then
    printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_SUPPORT_I2C" >> "$out_makefile"

    declare -A i2cChannelArray
    declare -i total_i2c_channels
    declare -i use_custom_i2c_pins

    use_custom_i2c_pins=0
    total_i2c_channels=0

    if [[ "$($yaml_parser "$yaml_file" i2c.display)" != "null" ]]
    then
        printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_SUPPORT_DISPLAY" >> "$out_makefile"

        i2c_channel=$($yaml_parser "$yaml_file" i2c.display.channel)
        i2c_pins=$($yaml_parser "$yaml_file" i2c.display.pins)

        if [[ $i2c_channel == "null" && $i2c_pins == "null" ]]
        then
            echo "I2C channel or pins left unspecified"
            exit 1
        fi

        if [[ $i2c_channel != "null" ]]
        then
            printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_I2C_CHANNEL_DISPLAY=$i2c_channel" >> "$out_makefile"
        elif [[ $i2c_pins != "null" ]]
        then
            use_custom_i2c_pins=1

            i2c_sda_port=$($yaml_parser "$yaml_file" i2c.display.pins.sda.port)
            i2c_sda_index=$($yaml_parser "$yaml_file" i2c.display.pins.sda.index)
            i2c_scl_port=$($yaml_parser "$yaml_file" i2c.display.pins.scl.port)
            i2c_scl_index=$($yaml_parser "$yaml_file" i2c.display.pins.scl.index)

            key=${i2c_sda_port}${i2c_sda_index}${i2c_scl_port}${i2c_scl_index}

            if [[ -z "${i2cChannelArray[$key]}" ]]
            then
                # Unique (non-existing) channel found
                i2cChannelArray[$key]=$total_i2c_channels
                ((total_i2c_channels++))
            fi

            printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_I2C_CHANNEL_DISPLAY=0" >> "$out_makefile"

            {
                printf "%s\n" "#define PIN_PORT_I2C_CHANNEL_${i2cChannelArray[$key]}_SDA CORE_MCU_IO_PIN_PORT_DEF(${i2c_sda_port})"
                printf "%s\n" "#define PIN_INDEX_I2C_CHANNEL_${i2cChannelArray[$key]}_SDA CORE_MCU_IO_PIN_INDEX_DEF(${i2c_sda_index})"
                printf "%s\n" "#define PIN_PORT_I2C_CHANNEL_${i2cChannelArray[$key]}_SCL CORE_MCU_IO_PIN_PORT_DEF(${i2c_scl_port})"
                printf "%s\n" "#define PIN_INDEX_I2C_CHANNEL_${i2cChannelArray[$key]}_SCL CORE_MCU_IO_PIN_INDEX_DEF(${i2c_scl_index})"
            } >> "$out_header"
        fi
    fi

    if [[ "$use_custom_i2c_pins" -eq 1 ]]
    then
        {
            printf "%s\n" "namespace gen {"
            printf "%s\n" "constexpr inline core::mcu::i2c::Config::pins_t I2C_PINS[$total_i2c_channels] = {"
        } >> "$out_header"

        for ((channel=0; channel<total_i2c_channels;channel++))
        do
            {
                printf "%s\n" "{"
                printf "%s\n" "core::mcu::io::pin_t{PIN_PORT_I2C_CHANNEL_${channel}_SDA, PIN_INDEX_I2C_CHANNEL_${channel}_SDA},"
                printf "%s\n" "core::mcu::io::pin_t{PIN_PORT_I2C_CHANNEL_${channel}_SCL, PIN_INDEX_I2C_CHANNEL_${channel}_SCL},"
                printf "%s\n" "},"
            } >> "$out_header"
        done

        {
            printf "%s\n" "};"
            printf "%s\n" "}"
        } >> "$out_header"
    fi
fi

if [[ $($yaml_parser "$yaml_file" bootloader.button) != "null" ]]
then
    printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_SUPPORT_BOOTLOADER_BUTTON" >> "$out_makefile"

    port=$($yaml_parser "$yaml_file" bootloader.button.port)
    index=$($yaml_parser "$yaml_file" bootloader.button.index)

    {
        printf "%s\n" "#define PIN_PORT_BTLDR_BUTTON CORE_MCU_IO_PIN_PORT_DEF(${port})"
        printf "%s\n" "#define PIN_INDEX_BTLDR_BUTTON CORE_MCU_IO_PIN_INDEX_DEF(${index})"
    } >> "$out_header"

    if [[ "$($yaml_parser "$yaml_file" bootloader.button.activeState)" == "high" ]]
    then
        # Active high
        printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_BOOTLOADER_BUTTON_ACTIVE_HIGH" >> "$out_makefile"
    fi
fi