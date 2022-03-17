#!/usr/bin/env bash

if [[ $($yaml_parser "$yaml_file" usb) == "true" ]]
then
    printf "%s\n" "DEFINES += USB_SUPPORTED" >> "$out_makefile"

    {
        printf "%s\n" "#if defined(FW_APP)"
        printf "%s\n" "#define USB_PRODUCT UNICODE_STRING(\"OpenDeck | $board_name\")"
        printf "%s\n" "#elif defined(FW_BOOT)"
        printf "%s\n" "#define USB_PRODUCT UNICODE_STRING(\"OpenDeck DFU | $board_name\")"
        printf "%s\n" "#endif"
    } >> "$out_header_usb"
fi

if [[ "$($yaml_parser "$yaml_file" uart)" != "null" ]]
then
    printf "%s\n" 'DEFINES += UART_SUPPORTED' >> "$out_makefile"

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
                printf "%s\n" "DEFINES += UART_CHANNEL_USB_LINK=$uart_channel_usb_link" >> "$out_makefile"
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
                    # unique (non-existing) channel found
                    uartChannelArray[$key]=$total_uart_channels
                    ((total_uart_channels++))
                fi

                uart_channel_usb_link=${uartChannelArray[$key]}
                printf "%s\n" "DEFINES += UART_CHANNEL_USB_LINK=$uart_channel_usb_link" >> "$out_makefile"

                {
                    printf "%s\n" "#define UART_CHANNEL_${uart_channel_usb_link}_RX_PORT CORE_IO_PIN_PORT_DEF(${uart_usb_link_rx_port})"
                    printf "%s\n" "#define UART_CHANNEL_${uart_channel_usb_link}_RX_INDEX CORE_IO_PIN_INDEX_DEF(${uart_usb_link_rx_index})"
                    printf "%s\n" "#define UART_CHANNEL_${uart_channel_usb_link}_TX_PORT CORE_IO_PIN_PORT_DEF(${uart_usb_link_tx_port})"
                    printf "%s\n" "#define UART_CHANNEL_${uart_channel_usb_link}_TX_INDEX CORE_IO_PIN_INDEX_DEF(${uart_usb_link_tx_index})"
                } >> "$out_header"
            fi

            if [[ "$($yaml_parser "$yaml_file" uart.usbLink.type)" == "host" ]]
            then
                {
                    printf "%s\n" "DEFINES += USB_LINK_MCU"
                    printf "%s\n" "DEFINES += FW_SELECTOR_NO_VERIFY_CRC"
                    printf "%s\n" "#append this only if it wasn't appended already"
                    printf "%s\n" 'ifeq (,$(findstring USB_SUPPORTED,$(DEFINES)))'
                    printf "%s\n" "    DEFINES += USB_SUPPORTED"
                    printf "%s\n" "endif"
                } >> "$out_makefile"
            elif [[ "$($yaml_parser "$yaml_file" uart.usbLink.type)" == "device" ]]
            then
                # make sure slave MCUs don't have USB enabled
                printf "%s\n" 'DEFINES := $(filter-out USB_SUPPORTED,$(DEFINES))' >> "$out_makefile"
            fi
        fi
    fi

    if [[ "$($yaml_parser "$yaml_file" uart.dinMIDI)" != "null" ]]
    then
        printf "%s\n" "DEFINES += DIN_MIDI_SUPPORTED" >> "$out_makefile"

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
            printf "%s\n" "DEFINES += UART_CHANNEL_DIN=$uart_channel_din_midi" >> "$out_makefile"
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
                # unique (non-existing) channel found
                uartChannelArray[$key]=$total_uart_channels
                ((total_uart_channels++))
            fi

            uart_channel_din_midi=${uartChannelArray[$key]}
            printf "%s\n" "DEFINES += UART_CHANNEL_DIN=$uart_channel_din_midi" >> "$out_makefile"

            {
                printf "%s\n" "#define UART_CHANNEL_${uart_channel_din_midi}_RX_PORT CORE_IO_PIN_PORT_DEF(${uart_din_midi_rx_port})"
                printf "%s\n" "#define UART_CHANNEL_${uart_channel_din_midi}_RX_INDEX CORE_IO_PIN_INDEX_DEF(${uart_din_midi_rx_index})"
                printf "%s\n" "#define UART_CHANNEL_${uart_channel_din_midi}_TX_PORT CORE_IO_PIN_PORT_DEF(${uart_din_midi_tx_port})"
                printf "%s\n" "#define UART_CHANNEL_${uart_channel_din_midi}_TX_INDEX CORE_IO_PIN_INDEX_DEF(${uart_din_midi_tx_index})"
            } >> "$out_header"
        fi
    fi

    if [[ $($yaml_parser "$yaml_file" uart.dmx) != "null" ]]
    then
        printf "%s\n" "DEFINES += DMX_SUPPORTED" >> "$out_makefile"

        uart_channel_dmx=$($yaml_parser "$yaml_file" uart.dmx.channel)
        uart_dmx_pins=$($yaml_parser "$yaml_file" uart.dmx.pins)

        if [[ -n "$uart_channel_usb_link" ]]
        then
            if [[ $uart_channel_usb_link -eq $uart_channel_dmx ]]
            then
                echo "USB link UART channel and DMX UART channel cannot be the same"
                exit 1
            fi
        fi

        if [[ $uart_channel_dmx == "null" && $uart_dmx_pins == "null" ]]
        then
            echo "DMX UART channel or pins left unspecified"
            exit 1
        fi

        if [[ $uart_channel_dmx != "null" ]]
        then
            printf "%s\n" "DEFINES += UART_CHANNEL_DMX=$uart_channel_dmx" >> "$out_makefile"
        elif [[ $uart_dmx_pins != "null" ]]
        then
            use_custom_uart_pins=1

            uart_dmx_rx_port=$($yaml_parser "$yaml_file" uart.dmx.pins.rx.port)
            uart_dmx_rx_index=$($yaml_parser "$yaml_file" uart.dmx.pins.rx.index)
            uart_dmx_tx_port=$($yaml_parser "$yaml_file" uart.dmx.pins.tx.port)
            uart_dmx_tx_index=$($yaml_parser "$yaml_file" uart.dmx.pins.tx.index)

            key=${uart_dmx_rx_port}${uart_dmx_rx_index}${uart_dmx_tx_port}${uart_dmx_tx_index}

            if [[ -z "${uartChannelArray[$key]}" ]]
            then
                # unique (non-existing) channel found
                uartChannelArray[$key]=$total_uart_channels
                ((total_uart_channels++))
            fi

            uart_channel_dmx=${uartChannelArray[$key]}
            printf "%s\n" "DEFINES += UART_CHANNEL_DMX=$uart_channel_dmx" >> "$out_makefile"

            {
                printf "%s\n" "#define UART_CHANNEL_${uart_channel_dmx}_RX_PORT CORE_IO_PIN_PORT_DEF(${uart_dmx_rx_port})"
                printf "%s\n" "#define UART_CHANNEL_${uart_channel_dmx}_RX_INDEX CORE_IO_PIN_INDEX_DEF(${uart_dmx_rx_index})"
                printf "%s\n" "#define UART_CHANNEL_${uart_channel_dmx}_TX_PORT CORE_IO_PIN_PORT_DEF(${uart_dmx_tx_port})"
                printf "%s\n" "#define UART_CHANNEL_${uart_channel_dmx}_TX_INDEX CORE_IO_PIN_INDEX_DEF(${uart_dmx_tx_index})"
            } >> "$out_header"
        fi
    fi

    if [[ "$($yaml_parser "$yaml_file" uart.touchscreen)" != "null" ]]
    then
        printf "%s\n" "DEFINES += TOUCHSCREEN_SUPPORTED" >> "$out_makefile"

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
            printf "%s\n" "DEFINES += UART_CHANNEL_TOUCHSCREEN=$uart_channel_touchscreen" >> "$out_makefile"
        elif [[ $uart_dmx_pins != "null" ]]
        then
            use_custom_uart_pins=1

            uart_touchscreen_rx_port=$($yaml_parser "$yaml_file" uart.touchscreen.pins.rx.port)
            uart_touchscreen_rx_index=$($yaml_parser "$yaml_file" uart.touchscreen.pins.rx.index)
            uart_touchscreen_tx_port=$($yaml_parser "$yaml_file" uart.touchscreen.pins.tx.port)
            uart_touchscreen_tx_index=$($yaml_parser "$yaml_file" uart.touchscreen.pins.tx.index)

            key=${uart_touchscreen_rx_port}${uart_touchscreen_rx_index}${uart_touchscreen_tx_port}${uart_touchscreen_tx_index}

            if [[ -z "${uartChannelArray[$key]}" ]]
            then
                # unique (non-existing) channel found
                uartChannelArray[$key]=$total_uart_channels
                ((total_uart_channels++))
            fi

            uart_channel_touchscreen=${uartChannelArray[$key]}
            printf "%s\n" "DEFINES += UART_CHANNEL_TOUCHSCREEN=$uart_channel_touchscreen" >> "$out_makefile"

            {
                printf "%s\n" "#define UART_CHANNEL_${uart_channel_touchscreen}_RX_PORT CORE_IO_PIN_PORT_DEF(${uart_touchscreen_rx_port})"
                printf "%s\n" "#define UART_CHANNEL_${uart_channel_touchscreen}_RX_INDEX CORE_IO_PIN_INDEX_DEF(${uart_touchscreen_rx_index})"
                printf "%s\n" "#define UART_CHANNEL_${uart_channel_touchscreen}_TX_PORT CORE_IO_PIN_PORT_DEF(${uart_touchscreen_tx_port})"
                printf "%s\n" "#define UART_CHANNEL_${uart_channel_touchscreen}_TX_INDEX CORE_IO_PIN_INDEX_DEF(${uart_touchscreen_tx_index})"
            } >> "$out_header"
        fi

        # guard against ommisions of touchscreen component amount by assigning the value to 0 if undefined
        nr_of_touchscreen_components=$($yaml_parser "$yaml_file" uart.touchscreen.components | grep -v null | awk '{print$1}END{if(NR==0)print 0}')

        if [[ "$nr_of_touchscreen_components" -eq 0 ]]
        then
            echo "Amount of touchscreen components cannot be 0 or undefined"
            exit 1
        fi

        printf "%s\n" "DEFINES += NR_OF_TOUCHSCREEN_COMPONENTS=$nr_of_touchscreen_components" >> "$out_makefile"
    else
        printf "%s\n" "DEFINES += NR_OF_TOUCHSCREEN_COMPONENTS=0" >> "$out_makefile"
    fi

    if [[ "$use_custom_uart_pins" -eq 1 ]]
    then
        {
            printf "%s\n" "namespace {"
            printf "%s\n" "constexpr inline Board::detail::UART::uartPins_t UART_PINS[$total_uart_channels] = {"
        } >> "$out_header"

        for ((channel=0; channel<total_uart_channels;channel++))
        do
            {
                printf "%s\n" "{"
                printf "%s\n" "CORE_IO_MCU_PIN_VAR(UART_CHANNEL_${channel}_RX_PORT, UART_CHANNEL_${channel}_RX_INDEX),"
                printf "%s\n" "CORE_IO_MCU_PIN_VAR(UART_CHANNEL_${channel}_TX_PORT, UART_CHANNEL_${channel}_TX_INDEX),"
                printf "%s\n" "},"
            } >> "$out_header"
        done

        {
            printf "%s\n" "};"
            printf "%s\n" "}"
        } >> "$out_header"
    fi
else
    # make sure this is set to 0 if uart/touchscreen isn't used as this symbol is used thorugh application IO modules
    printf "%s\n" "DEFINES += NR_OF_TOUCHSCREEN_COMPONENTS=0" >> "$out_makefile"
fi

if [[ "$($yaml_parser "$yaml_file" i2c)" != "null" ]]
then
    printf "%s\n" "DEFINES += I2C_SUPPORTED" >> "$out_makefile"

    i2c_channel=$($yaml_parser "$yaml_file" i2c.channel)
    i2c_pins=$($yaml_parser "$yaml_file" i2c.pins)

    if [[ $i2c_channel == "null" && $i2c_pins == "null" ]]
    then
        echo "I2C channel or pins left unspecified"
        exit 1
    fi

    if [[ $i2c_channel != "null" ]]
    then
        printf "%s\n" "DEFINES += I2C_CHANNEL=$i2c_channel" >> "$out_makefile"
    elif [[ $i2c_pins != "null" ]]
    then
        i2c_sda_port=$($yaml_parser "$yaml_file" i2c.pins.sda.port)
        i2c_sda_index=$($yaml_parser "$yaml_file" i2c.pins.sda.index)
        i2c_scl_port=$($yaml_parser "$yaml_file" i2c.pins.scl.port)
        i2c_scl_index=$($yaml_parser "$yaml_file" i2c.pins.scl.index)

        printf "%s\n" "DEFINES += I2C_CHANNEL=0" >> "$out_makefile"

        {
            printf "%s\n" "#define I2C_SDA_PORT CORE_IO_PIN_PORT_DEF(${i2c_sda_port})"
            printf "%s\n" "#define I2C_SDA_INDEX CORE_IO_PIN_INDEX_DEF(${i2c_sda_index})"
            printf "%s\n" "#define I2C_SCL_PORT CORE_IO_PIN_PORT_DEF(${i2c_scl_port})"
            printf "%s\n" "#define I2C_SCL_INDEX CORE_IO_PIN_INDEX_DEF(${i2c_scl_index})"
            printf "%s\n" "namespace {"
            printf "%s\n" "constexpr inline Board::detail::I2C::i2cPins_t I2C_PINS[1] = {"
            printf "%s\n" "{"
            printf "%s\n" "CORE_IO_MCU_PIN_VAR(I2C_SDA_PORT, I2C_SDA_INDEX),"
            printf "%s\n" "CORE_IO_MCU_PIN_VAR(I2C_SCL_PORT, I2C_SCL_INDEX),"
            printf "%s\n" "},"
            printf "%s\n" "};"
            printf "%s\n" "}"
        } >> "$out_header"
    fi
fi

if [[ $($yaml_parser "$yaml_file" bootloader.button) != "null" ]]
then
    port=$($yaml_parser "$yaml_file" bootloader.button.port)
    index=$($yaml_parser "$yaml_file" bootloader.button.index)

    {
        printf "%s\n" "#define BTLDR_BUTTON_PORT CORE_IO_PIN_PORT_DEF(${port})"
        printf "%s\n" "#define BTLDR_BUTTON_PIN CORE_IO_PIN_INDEX_DEF(${index})"
    } >> "$out_header"

    if [[ "$($yaml_parser "$yaml_file" bootloader.button.activeState)" == "high" ]]
    then
        # active high
        printf "%s\n" "DEFINES += BTLDR_BUTTON_AH" >> "$out_makefile"
    fi
fi