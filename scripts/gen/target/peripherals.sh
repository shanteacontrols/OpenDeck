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

    if [[ "$($yaml_parser "$yaml_file" uart.usbLink)" != "null" ]]
    then
        if [[ "$($yaml_parser "$yaml_file" uart.usbLink.type)" != "null" ]]
        then
            uart_channel_usb_link=$($yaml_parser "$yaml_file" uart.usbLink.channel)

            if [[ $uart_channel_usb_link == "null" ]]
            then
                echo "USB link channel left unspecified"
                exit 1
            fi

            printf "%s\n" "DEFINES += UART_CHANNEL_USB_LINK=$uart_channel_usb_link" >> "$out_makefile"

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
        uart_channel_din_midi=$($yaml_parser "$yaml_file" uart.dinMIDI.channel)

        if [[ -n "$uart_channel_usb_link" ]]
        then
            if [[ $uart_channel_usb_link -eq $uart_channel_din_midi ]]
            then
                echo "USB link channel and DIN MIDI channel cannot be the same"
                exit 1
            fi
        fi

        {
            printf "%s\n" "DEFINES += DIN_MIDI_SUPPORTED"
            printf "%s\n" "DEFINES += UART_CHANNEL_DIN=$uart_channel_din_midi"
        } >> "$out_makefile"
    fi

    if [[ $($yaml_parser "$yaml_file" uart.dmx) != "null" ]]
    then
        uart_channel_dmx=$($yaml_parser "$yaml_file" uart.dmx.channel)

        if [[ $uart_channel_dmx == "null" ]]
        then
            echo "DMX channel left unspecified"
            exit 1
        fi

        if [[ -n "$uart_channel_usb_link" ]]
        then
            if [[ $uart_channel_usb_link -eq $uart_channel_dmx ]]
            then
                echo "USB link channel and DMX channel cannot be the same"
                exit 1
            fi
        fi

        {
            printf "%s\n" "DEFINES += DMX_SUPPORTED" >> "$out_makefile"
            printf "%s\n" "DEFINES += UART_CHANNEL_DMX=$uart_channel_dmx"
        } >> "$out_makefile"
    fi

    if [[ "$($yaml_parser "$yaml_file" uart.touchscreen)" != "null" ]]
    then
        uart_channel_touchscreen=$($yaml_parser "$yaml_file" uart.touchscreen.channel)

        if [[ $uart_channel_touchscreen == "null" ]]
        then
            echo "Touchscreen channel left unspecified"
            exit 1
        fi

        if [[ -n "$uart_channel_usb_link" ]]
        then
            if [[ $uart_channel_usb_link -eq $uart_channel_touchscreen ]]
            then
                echo "USB link channel and touchscreen channel cannot be the same"
                exit 1
            fi
        fi

        # guard against ommisions of touchscreen component amount by assigning the value to 0 if undefined
        nr_of_touchscreen_components=$($yaml_parser "$yaml_file" uart.touchscreen.components | grep -v null | awk '{print$1}END{if(NR==0)print 0}')

        if [[ "$nr_of_touchscreen_components" -eq 0 ]]
        then
            echo "Amount of touchscreen components cannot be 0 or undefined"
            exit 1
        fi

        {
            printf "%s\n" "DEFINES += TOUCHSCREEN_SUPPORTED"
            printf "%s\n" "DEFINES += NR_OF_TOUCHSCREEN_COMPONENTS=$nr_of_touchscreen_components"
            printf "%s\n" "DEFINES += UART_CHANNEL_TOUCHSCREEN=$uart_channel_touchscreen"
        } >> "$out_makefile"
    else
        printf "%s\n" "DEFINES += NR_OF_TOUCHSCREEN_COMPONENTS=0" >> "$out_makefile"
    fi
else
    # make sure this is set to 0 if uart/touchscreen isn't used as this symbol is used thorugh application IO modules
    printf "%s\n" "DEFINES += NR_OF_TOUCHSCREEN_COMPONENTS=0" >> "$out_makefile"
fi

if [[ "$($yaml_parser "$yaml_file" i2c)" != "null" ]]
then
    {
        printf "%s\n" "DEFINES += I2C_SUPPORTED"
        printf "%s\n" "DEFINES += I2C_CHANNEL=$($yaml_parser "$yaml_file" i2c.channel)"
    } >> "$out_makefile"
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