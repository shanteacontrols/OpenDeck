#!/usr/bin/env bash

TARGET_DEF_FILE=$1
GEN_DIR=$2
YAML_PARSER="dasel -n -p yaml --plain -f"
TARGET_NAME=$(basename "$TARGET_DEF_FILE" .yml)

OUT_FILE_HEADER_USB="$GEN_DIR"/USBnames.h
OUT_FILE_HEADER_PINS="$GEN_DIR"/Pins.h
OUT_FILE_SOURCE_PINS="$GEN_DIR"/$TARGET_NAME.cpp
OUT_FILE_MAKEFILE_DEFINES="$GEN_DIR"/Defines.mk
HW_TEST_HEADER_CONSTANTS="$GEN_DIR"/HWTestDefines.h

mkdir -p "$GEN_DIR"

#################################################### CORE ####################################################

mcu=$($YAML_PARSER "$TARGET_DEF_FILE" mcu)

## MCU processing first
MCU_GEN_DIR=$(dirname "$2")/../mcu/$mcu
MCU_DEF_FILE=$(dirname "$TARGET_DEF_FILE")/../mcu/$mcu.yml
HW_TEST_DEF_FILE=$(dirname "$TARGET_DEF_FILE")/../hw-test/$TARGET_NAME.yml

if [[ ! -f $MCU_DEF_FILE ]]
then
    echo "$MCU_DEF_FILE doesn't exist"
    exit 1
fi

printf "%s\n\n" "#pragma once" > "$HW_TEST_HEADER_CONSTANTS"

if [[ -f $HW_TEST_DEF_FILE ]]
then
    echo "Generating HW test config..."


    flash_port=$($YAML_PARSER "$HW_TEST_DEF_FILE" ports.flash)

    if [[ $flash_port != "null" ]]
    then
        {
            printf "%s\n" "#define TEST_FLASHING"
            printf "%s\n" "constexpr inline char FLASH_PORT[]=\"$flash_port\";"
        } >> "$HW_TEST_HEADER_CONSTANTS"
    fi

    din_midi_port=$($YAML_PARSER "$HW_TEST_DEF_FILE" ports.dinMidi)

    if [[ $din_midi_port != "null" ]]
    then
        {
            printf "%s\n" "#define TEST_DIN_MIDI_PORT"
            printf "%s\n" "constexpr inline char DIN_MIDI_PORT[]=\"$din_midi_port\";"
        } >> "$HW_TEST_HEADER_CONSTANTS"
    fi

    usb_link_target=$($YAML_PARSER "$HW_TEST_DEF_FILE" usbLinkTarget)

    if [[ $usb_link_target != "null" ]]
    then
        USB_LINK_HW_TEST_DEF_FILE=$(dirname "$HW_TEST_DEF_FILE")/$usb_link_target.yml
        usb_link_flash_port=$($YAML_PARSER "$USB_LINK_HW_TEST_DEF_FILE" ports.flash)

        {
            printf "%s\n" "constexpr inline char FLASH_PORT_USB_LINK[]=\"$usb_link_flash_port\";"
            printf "%s\n" "constexpr inline char USB_LINK_TARGET[]=\"$usb_link_target\";"
        } >> "$HW_TEST_HEADER_CONSTANTS"
    fi
fi

if [[ ! -d $MCU_GEN_DIR ]]
then
    echo "Generating MCU definitions..."
    ../scripts/gen_mcu.sh "$MCU_DEF_FILE" "$MCU_GEN_DIR"
fi

{
    printf "%s%s\n" '-include $(MAKEFILE_INCLUDE_PREFIX)$(BOARD_MCU_BASE_DIR)/' "$mcu/MCU.mk"
    printf "%s\n" "DEFINES += FW_UID=$(../scripts/fw_uid_gen.sh "$TARGET_NAME")"
} >> "$OUT_FILE_MAKEFILE_DEFINES"

hse_val=$($YAML_PARSER "$TARGET_DEF_FILE" extClockMhz)

if [[ $hse_val != "null" ]]
then
    printf "%s%s\n" "DEFINES += HSE_VALUE=$hse_val" "000000" >> "$OUT_FILE_MAKEFILE_DEFINES"
fi

board_name=$($YAML_PARSER "$TARGET_DEF_FILE" boardNameOverride)

if [[ $board_name == "null" ]]
then
    board_name=$TARGET_NAME
fi

printf "%s\n" "DEFINES += BOARD_STRING=\\\"$board_name\\\"" >> "$OUT_FILE_MAKEFILE_DEFINES"

########################################################################################################

{
    printf "%s\n\n" "#include \"core/src/general/IO.h\""
} >> "$OUT_FILE_HEADER_PINS"

#################################################### PERIPHERALS ####################################################

if [[ $($YAML_PARSER "$TARGET_DEF_FILE" usb) == "true" ]]
then
    printf "%s\n" "DEFINES += USB_SUPPORTED" >> "$OUT_FILE_MAKEFILE_DEFINES"

    {
        printf "%s\n" "#if defined(FW_APP)"
        printf "%s\n" "#define USB_PRODUCT UNICODE_STRING(\"OpenDeck | $board_name\")"
        printf "%s\n" "#elif defined(FW_BOOT)"
        printf "%s\n" "#define USB_PRODUCT UNICODE_STRING(\"OpenDeck DFU | $board_name\")"
        printf "%s\n" "#endif"
    } >> "$OUT_FILE_HEADER_USB"
fi

if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" usbLink)" != "null" ]]
then
    if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" usbLink.type)" != "null" ]]
    then
        uart_channel_usb_link=$($YAML_PARSER "$TARGET_DEF_FILE" usbLink.uartChannel)
        printf "%s\n" "DEFINES += UART_CHANNEL_USB_LINK=$uart_channel_usb_link" >> "$OUT_FILE_MAKEFILE_DEFINES"

        if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" usbLink.type)" == "host" ]]
        then
            {
                printf "%s\n" "DEFINES += USB_LINK_MCU"
                printf "%s\n" "DEFINES += FW_SELECTOR_NO_VERIFY_CRC"
                printf "%s\n" "#append this only if it wasn't appended already"
                printf "%s\n" 'ifeq (,$(findstring USB_SUPPORTED,$(DEFINES)))'
                printf "%s\n" "    DEFINES += USB_SUPPORTED"
                printf "%s\n" "endif"
            } >> "$OUT_FILE_MAKEFILE_DEFINES"
        elif [[ "$($YAML_PARSER "$TARGET_DEF_FILE" usbLink.type)" == "device" ]]
        then
            {
                printf "%s\n" "#make sure slave MCUs don't have USB enabled"
                printf "%s\n" 'DEFINES := $(filter-out USB_SUPPORTED,$(DEFINES))'
            } >> "$OUT_FILE_MAKEFILE_DEFINES"
        fi
    fi
fi

if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" dinMIDI)" != "null" ]]
then
    uart_channel_din_midi=$($YAML_PARSER "$TARGET_DEF_FILE" dinMIDI.uartChannel)

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
    } >> "$OUT_FILE_MAKEFILE_DEFINES"
fi

if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" i2c)" != "null" ]]
then
    {
        printf "%s\n" "DEFINES += I2C_SUPPORTED"
        printf "%s\n" "DEFINES += I2C_CHANNEL=$($YAML_PARSER "$TARGET_DEF_FILE" i2c.channel)"
    } >> "$OUT_FILE_MAKEFILE_DEFINES"
fi

{
    printf "%s\n" "#include \"Pins.h\""
    printf "%s\n" "#include <MCU.h>"
    printf "%s\n\n" "#include \"board/Internal.h\""
    printf "%s\n\n" "namespace {"
} > "$OUT_FILE_SOURCE_PINS"

if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" touchscreen)" != "null" ]]
then
    uart_channel_touchscreen=$($YAML_PARSER "$TARGET_DEF_FILE" touchscreen.uartChannel)

    if [[ -n "$uart_channel_usb_link" ]]
    then
        if [[ $uart_channel_usb_link -eq $uart_channel_touchscreen ]]
        then
            echo "USB link channel and touchscreen channel cannot be the same"
            exit 1
        fi
    fi

    #guard against ommisions of touchscreen component amount by assigning the value to 0 if undefined
    nr_of_touchscreen_components=$($YAML_PARSER "$TARGET_DEF_FILE" touchscreen.components | grep -v null | awk '{print$1}END{if(NR==0)print 0}')

    if [[ "$nr_of_touchscreen_components" -eq 0 ]]
    then
        echo "Amount of touchscreen components cannot be 0 or undefined"
        exit 1
    fi

    {
        printf "%s\n" "DEFINES += NR_OF_TOUCHSCREEN_COMPONENTS=$nr_of_touchscreen_components"
        printf "%s\n" "DEFINES += TOUCHSCREEN_SUPPORTED"
        printf "%s\n" "DEFINES += UART_CHANNEL_TOUCHSCREEN=$uart_channel_touchscreen"
    } >> "$OUT_FILE_MAKEFILE_DEFINES"
else
    printf "%s\n" "DEFINES += NR_OF_TOUCHSCREEN_COMPONENTS=0" >> "$OUT_FILE_MAKEFILE_DEFINES"
fi

if [[ $($YAML_PARSER "$TARGET_DEF_FILE" bootloader.button) != "null" ]]
then
    port=$($YAML_PARSER "$TARGET_DEF_FILE" bootloader.button.port)
    index=$($YAML_PARSER "$TARGET_DEF_FILE" bootloader.button.index)

    {
        printf "%s\n" "#define BTLDR_BUTTON_PORT CORE_IO_PORT(${port})"
        printf "%s\n" "#define BTLDR_BUTTON_PIN CORE_IO_PORT_INDEX(${index})"
    } >> "$OUT_FILE_HEADER_PINS"

    if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" bootloader.button.activeState)" == "high" ]]
    then
        #active high
        printf "%s\n" "DEFINES += BTLDR_BUTTON_AH" >> "$OUT_FILE_MAKEFILE_DEFINES"
    fi
fi

if [[ $($YAML_PARSER "$TARGET_DEF_FILE" dmx) != "null" ]]
then
    uart_channel_dmx=$($YAML_PARSER "$TARGET_DEF_FILE" dmx.uartChannel)

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
        printf "%s\n" "DEFINES += DMX_SUPPORTED" >> "$OUT_FILE_MAKEFILE_DEFINES"
        printf "%s\n" "DEFINES += UART_CHANNEL_DMX=$uart_channel_dmx"
    } >> "$OUT_FILE_MAKEFILE_DEFINES"
fi

########################################################################################################

#################################################### DIGITAL INPUTS ####################################################

if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" buttons)" != "null" ]]
then
    printf "%s\n" "DEFINES += DIGITAL_INPUTS_SUPPORTED" >> "$OUT_FILE_MAKEFILE_DEFINES"

    digital_in_type=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.type)

    declare -i nr_of_digital_inputs
    nr_of_digital_inputs=0

    if [[ $digital_in_type == native ]]
    then
        printf "%s\n" "const core::io::mcuPin_t dInPins[NR_OF_DIGITAL_INPUTS] = {" >> "$OUT_FILE_SOURCE_PINS"

        nr_of_digital_inputs=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.pins --length)

        for ((i=0; i<nr_of_digital_inputs; i++))
        do
            port=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.pins.["$i"].port)
            index=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.pins.["$i"].index)

            {
                printf "%s\n" "#define DIN_PORT_${i} CORE_IO_PORT(${port})"
                printf "%s\n" "#define DIN_PIN_${i} CORE_IO_PORT_INDEX(${index})"
            } >> "$OUT_FILE_HEADER_PINS"

            printf "%s\n" "CORE_IO_MCU_PIN_DEF(DIN_PORT_${i}, DIN_PIN_${i})," >> "$OUT_FILE_SOURCE_PINS"
        done

        printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
        printf "%s\n" "DEFINES += NATIVE_BUTTON_INPUTS" >> "$OUT_FILE_MAKEFILE_DEFINES"

        if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" buttons.extPullups)" == "true" ]]
        then
            printf "%s\n" "DEFINES += BUTTONS_EXT_PULLUPS" >> "$OUT_FILE_MAKEFILE_DEFINES"
        fi
    elif [[ $digital_in_type == shiftRegister ]]
    then
        port=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.pins.data.port)
        index=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.pins.data.index)

        {
            printf "%s\n" "#define SR_IN_DATA_PORT CORE_IO_PORT(${port})"
            printf "%s\n" "#define SR_IN_DATA_PIN CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"

        port=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.pins.clock.port)
        index=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.pins.clock.index)

        {
            printf "%s\n" "#define SR_IN_CLK_PORT CORE_IO_PORT(${port})"
            printf "%s\n" "#define SR_IN_CLK_PIN CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"

        port=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.pins.latch.port)
        index=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.pins.latch.index)

        {
            printf "%s\n" "#define SR_IN_LATCH_PORT CORE_IO_PORT(${port})"
            printf "%s\n" "#define SR_IN_LATCH_PIN CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"

        number_of_in_sr=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.shiftRegisters)
        nr_of_digital_inputs=$(( 8 * "$number_of_in_sr"))

        printf "%s\n" "DEFINES += NUMBER_OF_IN_SR=$number_of_in_sr" >> "$OUT_FILE_MAKEFILE_DEFINES"
    elif [[ $digital_in_type == matrix ]]
    then
        number_of_rows=0
        number_of_columns=0

        if [[ $($YAML_PARSER "$TARGET_DEF_FILE" buttons.rows.type) == "shiftRegister" ]]
        then
            number_of_rows=8

            port=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.rows.pins.data.port)
            index=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.rows.pins.data.index)

            {
                printf "%s\n" "#define SR_IN_DATA_PORT CORE_IO_PORT(${port})"
                printf "%s\n" "#define SR_IN_DATA_PIN CORE_IO_PORT_INDEX(${index})"
            } >> "$OUT_FILE_HEADER_PINS"

            port=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.rows.pins.clock.port)
            index=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.rows.pins.clock.index)

            {
                printf "%s\n" "#define SR_IN_CLK_PORT CORE_IO_PORT(${port})"
                printf "%s\n" "#define SR_IN_CLK_PIN CORE_IO_PORT_INDEX(${index})"
            } >> "$OUT_FILE_HEADER_PINS"

            port=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.rows.pins.latch.port)
            index=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.rows.pins.latch.index)

            {
                printf "%s\n" "#define SR_IN_LATCH_PORT CORE_IO_PORT(${port})"
                printf "%s\n" "#define SR_IN_LATCH_PIN CORE_IO_PORT_INDEX(${index})"
            } >> "$OUT_FILE_HEADER_PINS"

        elif [[ $($YAML_PARSER "$TARGET_DEF_FILE" buttons.rows.type) == "native" ]]
        then
            number_of_rows=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.rows.pins --length)

            printf "%s\n" "const core::io::mcuPin_t dInPins[$number_of_rows] = {" >> "$OUT_FILE_SOURCE_PINS"

            for ((i=0; i<number_of_rows; i++))
            do
                port=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.rows.pins.["$i"].port)
                index=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.rows.pins.["$i"].index)

                {
                    printf "%s\n" "#define DIN_PORT_${i} CORE_IO_PORT(${port})"
                    printf "%s\n" "#define DIN_PIN_${i} CORE_IO_PORT_INDEX(${index})"
                } >> "$OUT_FILE_HEADER_PINS"

                printf "%s\n" "CORE_IO_MCU_PIN_DEF(DIN_PORT_${i}, DIN_PIN_${i})," >> "$OUT_FILE_SOURCE_PINS"
            done

            printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
        else
            echo "Invalid button row type specified"
            exit 1
        fi

        if [[ $($YAML_PARSER "$TARGET_DEF_FILE" buttons.columns.pins --length) -eq 3 ]]
        then
            number_of_columns=8

            for ((i=0; i<3; i++))
            do
                port=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.columns.pins.decA"$i".port)
                index=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.columns.pins.decA"$i".index)

                {
                    printf "%s\n" "#define DEC_BM_PORT_A${i} CORE_IO_PORT(${port})"
                    printf "%s\n" "#define DEC_BM_PIN_A${i} CORE_IO_PORT_INDEX(${index})"
                } >> "$OUT_FILE_HEADER_PINS"
            done
        else
            echo "Invalid number of columns specified"
            exit 1
        fi

        nr_of_digital_inputs=$(("$number_of_columns" * "$number_of_rows"))

        {
            printf "%s\n" "DEFINES += NUMBER_OF_BUTTON_COLUMNS=$number_of_columns"
            printf "%s\n" "DEFINES += NUMBER_OF_BUTTON_ROWS=$number_of_rows"
        } >> "$OUT_FILE_MAKEFILE_DEFINES"
    fi

    if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" buttons.indexing)" != "null" ]]
    then
        nr_of_digital_inputs=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.indexing --length)

        printf "%s\n" "const uint8_t buttonIndexes[NR_OF_DIGITAL_INPUTS] = {" >> "$OUT_FILE_SOURCE_PINS"

        for ((i=0; i<nr_of_digital_inputs; i++))
        do
            index=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.indexing.["$i"])
            printf "%s\n" "${index}," >> "$OUT_FILE_SOURCE_PINS"
        done

        printf "%s\n" "DEFINES += BUTTON_INDEXING" >> "$OUT_FILE_MAKEFILE_DEFINES"
        printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
    fi

    printf "%s\n" "DEFINES += NR_OF_DIGITAL_INPUTS=$nr_of_digital_inputs" >> "$OUT_FILE_MAKEFILE_DEFINES"
else
    {
        printf "%s\n" "DEFINES += NR_OF_DIGITAL_INPUTS=0"
    } >> "$OUT_FILE_MAKEFILE_DEFINES"
fi

########################################################################################################

#################################################### DIGITAL OUTPUTS ####################################################

if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" leds.external)" != "null" ]]
then
    printf "%s\n" "DEFINES += DIGITAL_OUTPUTS_SUPPORTED" >> "$OUT_FILE_MAKEFILE_DEFINES"

    digital_out_type=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.type)

    declare -i nr_of_digital_outputs
    nr_of_digital_outputs=0

    if [[ $digital_out_type == "native" ]]
    then
        printf "%s\n" "const core::io::mcuPin_t dOutPins[NR_OF_DIGITAL_OUTPUTS] = {" >> "$OUT_FILE_SOURCE_PINS"

        nr_of_digital_outputs=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.pins --length)

        for ((i=0; i<nr_of_digital_outputs; i++))
        do
            port=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.pins.["$i"].port)
            index=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.pins.["$i"].index)

            {
                printf "%s\n" "#define DOUT_PORT_${i} CORE_IO_PORT(${port})"
                printf "%s\n" "#define DOUT_PIN_${i} CORE_IO_PORT_INDEX(${index})"
            } >> "$OUT_FILE_HEADER_PINS"

            printf "%s\n" "CORE_IO_MCU_PIN_DEF(DOUT_PORT_${i}, DOUT_PIN_${i})," >> "$OUT_FILE_SOURCE_PINS"
        done

        printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
        printf "%s\n" "DEFINES += NATIVE_LED_OUTPUTS" >> "$OUT_FILE_MAKEFILE_DEFINES"
    elif [[ $digital_out_type == shiftRegister ]]
    then
        port=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.pins.data.port)
        index=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.pins.data.index)

        {
            printf "%s\n" "#define SR_OUT_DATA_PORT CORE_IO_PORT(${port})"
            printf "%s\n" "#define SR_OUT_DATA_PIN CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"

        port=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.pins.clock.port)
        index=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.pins.clock.index)

        {
            printf "%s\n" "#define SR_OUT_CLK_PORT CORE_IO_PORT(${port})"
            printf "%s\n" "#define SR_OUT_CLK_PIN CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"

        port=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.pins.latch.port)
        index=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.pins.latch.index)

        {
            printf "%s\n" "#define SR_OUT_LATCH_PORT CORE_IO_PORT(${port})"
            printf "%s\n" "#define SR_OUT_LATCH_PIN CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"

        port=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.pins.enable.port)
        index=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.pins.enable.index)

        {
            printf "%s\n" "#define SR_OUT_OE_PORT CORE_IO_PORT(${port})"
            printf "%s\n" "#define SR_OUT_OE_PIN CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"

        number_of_out_sr=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.shiftRegisters)
        nr_of_digital_outputs=$((number_of_out_sr * 8))

        printf "%s\n" "DEFINES += NUMBER_OF_OUT_SR=$number_of_out_sr" >> "$OUT_FILE_MAKEFILE_DEFINES"
    elif [[ $digital_out_type == matrix ]]
    then
        number_of_led_columns=8
        number_of_led_rows=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.rows.pins --length)

        for ((i=0; i<3; i++))
        do
            port=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.columns.pins.decA"$i".port)
            index=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.columns.pins.decA"$i".index)

            {
                printf "%s\n" "#define DEC_LM_PORT_A${i} CORE_IO_PORT(${port})"
                printf "%s\n" "#define DEC_LM_PIN_A${i} CORE_IO_PORT_INDEX(${index})"
            } >> "$OUT_FILE_HEADER_PINS"
        done

        printf "%s\n" "const core::io::mcuPin_t dOutPins[$rows] = {" >> "$OUT_FILE_SOURCE_PINS"

        for ((i=0; i<"$number_of_led_rows"; i++))
        do
            port=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.rows.pins.["$i"].port)
            index=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.rows.pins.["$i"].index)

            {
                printf "%s\n" "#define LED_ROW_PORT_${i} CORE_IO_PORT(${port})"
                printf "%s\n" "#define LED_ROW_PIN_${i} CORE_IO_PORT_INDEX(${index})"
            } >> "$OUT_FILE_HEADER_PINS"

            printf "%s\n" "CORE_IO_MCU_PIN_DEF(LED_ROW_PORT_${i}, LED_ROW_PIN_${i})," >> "$OUT_FILE_SOURCE_PINS"
        done

        printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"

        nr_of_digital_outputs=$(("$number_of_led_columns" * "$number_of_led_rows"))

        {
            printf "%s\n" "DEFINES += NUMBER_OF_LED_COLUMNS=$number_of_led_columns"
            printf "%s\n" "DEFINES += NUMBER_OF_LED_ROWS=$number_of_led_rows"
        } >> "$OUT_FILE_MAKEFILE_DEFINES"
    fi

    if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.indexing)" != "null" ]]
    then
        nr_of_digital_outputs=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.indexing --length)

        printf "%s\n" "const uint8_t ledIndexes[NR_OF_DIGITAL_OUTPUTS] = {" >> "$OUT_FILE_SOURCE_PINS"

        for ((i=0; i<nr_of_digital_outputs; i++))
        do
            index=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.indexing.["$i"])
            printf "%s\n" "${index}," >> "$OUT_FILE_SOURCE_PINS"
        done

        printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
        printf "%s\n" "DEFINES += LED_INDEXING" >> "$OUT_FILE_MAKEFILE_DEFINES"
    fi

    {
        printf "%s\n" "DEFINES += NR_OF_DIGITAL_OUTPUTS=$nr_of_digital_outputs"
    } >> "$OUT_FILE_MAKEFILE_DEFINES"

    if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.invert)" == "true" ]]
    then
        printf "%s\n" "DEFINES += LED_EXT_INVERT" >> "$OUT_FILE_MAKEFILE_DEFINES"
    fi
else
    {
        printf "%s\n" "DEFINES += NR_OF_DIGITAL_OUTPUTS=0"
    } >> "$OUT_FILE_MAKEFILE_DEFINES"
fi

if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" leds.internal)" != "null" ]]
then
    printf "%s\n" "DEFINES += LED_INDICATORS" >> "$OUT_FILE_MAKEFILE_DEFINES"
    printf "%s\n" "DEFINES += LED_INDICATORS_CTL" >> "$OUT_FILE_MAKEFILE_DEFINES"

    if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" leds.internal.invert)" == "true" ]]
    then
        printf "%s\n" "DEFINES += LED_INT_INVERT" >> "$OUT_FILE_MAKEFILE_DEFINES"
    fi

    if [[ $($YAML_PARSER "$TARGET_DEF_FILE" leds.internal.pins.din) != "null" ]]
    then
        port=$($YAML_PARSER "$TARGET_DEF_FILE" leds.internal.pins.din.rx.port)
        index=$($YAML_PARSER "$TARGET_DEF_FILE" leds.internal.pins.din.rx.index)

        {
            printf "%s\n" "#define LED_MIDI_IN_DIN_PORT CORE_IO_PORT(${port})"
            printf "%s\n" "#define LED_MIDI_IN_DIN_PIN CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"

        port=$($YAML_PARSER "$TARGET_DEF_FILE" leds.internal.pins.din.tx.port)
        index=$($YAML_PARSER "$TARGET_DEF_FILE" leds.internal.pins.din.tx.index)

        {
            printf "%s\n" "#define LED_MIDI_OUT_DIN_PORT CORE_IO_PORT(${port})"
            printf "%s\n" "#define LED_MIDI_OUT_DIN_PIN CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"
    fi

    if [[ $($YAML_PARSER "$TARGET_DEF_FILE" leds.internal.pins.usb) != "null" ]]
    then
        port=$($YAML_PARSER "$TARGET_DEF_FILE" leds.internal.pins.usb.rx.port)
        index=$($YAML_PARSER "$TARGET_DEF_FILE" leds.internal.pins.usb.rx.index)

        {
            printf "%s\n" "#define LED_MIDI_IN_USB_PORT CORE_IO_PORT(${port})"
            printf "%s\n" "#define LED_MIDI_IN_USB_PIN CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"

        port=$($YAML_PARSER "$TARGET_DEF_FILE" leds.internal.pins.usb.tx.port)
        index=$($YAML_PARSER "$TARGET_DEF_FILE" leds.internal.pins.usb.tx.index)

        {
            printf "%s\n" "#define LED_MIDI_OUT_USB_PORT CORE_IO_PORT(${port})"
            printf "%s\n" "#define LED_MIDI_OUT_USB_PIN CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"
    fi
fi

########################################################################################################

#################################################### ANALOG INPUTS ####################################################

if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" analog)" != "null" ]]
then
    printf "%s\n" "DEFINES += ADC_SUPPORTED" >> "$OUT_FILE_MAKEFILE_DEFINES"

    if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" analog.extReference)" == "true" ]]
    then
        printf "%s\n" "DEFINES += ADC_EXT_REF" >> "$OUT_FILE_MAKEFILE_DEFINES"
    fi

    declare -i offset_lower
    declare -i offset_upper

    offset_lower=0
    offset_upper=0

    offset_lower=$($YAML_PARSER "$TARGET_DEF_FILE" analog.offset.lower)
    offset_upper=$($YAML_PARSER "$TARGET_DEF_FILE" analog.offset.upper)

    {
        printf "%s\n" "DEFINES += ADC_LOWER_OFFSET_PERCENTAGE=$offset_lower"
        printf "%s\n" "DEFINES += ADC_UPPER_OFFSET_PERCENTAGE=$offset_upper"
    } >> "$OUT_FILE_MAKEFILE_DEFINES"

    analog_in_type=$($YAML_PARSER "$TARGET_DEF_FILE" analog.type)

    declare -i nr_of_analog_inputs
    nr_of_analog_inputs=0

    if [[ $analog_in_type == "native" ]]
    then
        printf "%s\n" "const core::io::mcuPin_t aInPins[MAX_ADC_CHANNELS] = {" >> "$OUT_FILE_SOURCE_PINS"

        nr_of_analog_inputs=$($YAML_PARSER "$TARGET_DEF_FILE" analog.pins --length)

        for ((i=0; i<nr_of_analog_inputs; i++))
        do
            port=$($YAML_PARSER "$TARGET_DEF_FILE" analog.pins.["$i"].port)
            index=$($YAML_PARSER "$TARGET_DEF_FILE" analog.pins.["$i"].index)

            {
                printf "%s\n" "#define AIN_PORT_${i} CORE_IO_PORT(${port})"
                printf "%s\n" "#define AIN_PIN_${i} CORE_IO_PORT_INDEX(${index})"
            } >> "$OUT_FILE_HEADER_PINS"

            printf "%s\n" "CORE_IO_MCU_PIN_DEF(AIN_PORT_${i}, AIN_PIN_${i})," >> "$OUT_FILE_SOURCE_PINS"
        done

        printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"

        {
            printf "%s\n" "DEFINES += MAX_ADC_CHANNELS=$nr_of_analog_inputs"
            printf "%s\n" "DEFINES += NATIVE_ANALOG_INPUTS"
        } >> "$OUT_FILE_MAKEFILE_DEFINES"

    elif [[ $analog_in_type == 4067 ]]
    then
        printf "%s\n" "const core::io::mcuPin_t aInPins[MAX_ADC_CHANNELS] = {" >> "$OUT_FILE_SOURCE_PINS"

        for ((i=0; i<4; i++))
        do
            port=$($YAML_PARSER "$TARGET_DEF_FILE" analog.pins.s"$i".port)
            index=$($YAML_PARSER "$TARGET_DEF_FILE" analog.pins.s"$i".index)

            {
                printf "%s\n" "#define MUX_PORT_S${i} CORE_IO_PORT(${port})"
                printf "%s\n" "#define MUX_PIN_S${i} CORE_IO_PORT_INDEX(${index})"
            } >> "$OUT_FILE_HEADER_PINS"
        done

        number_of_mux=$($YAML_PARSER "$TARGET_DEF_FILE" analog.multiplexers)

        for ((i=0; i<"$number_of_mux"; i++))
        do
            port=$($YAML_PARSER "$TARGET_DEF_FILE" analog.pins.z"$i".port)
            index=$($YAML_PARSER "$TARGET_DEF_FILE" analog.pins.z"$i".index)

            {
                printf "%s\n" "#define MUX_PORT_INPUT_${i} CORE_IO_PORT(${port})"
                printf "%s\n" "#define MUX_PIN_INPUT_${i} CORE_IO_PORT_INDEX(${index})"
            } >> "$OUT_FILE_HEADER_PINS"

            printf "%s\n" "CORE_IO_MCU_PIN_DEF(MUX_PORT_INPUT_${i}, MUX_PIN_INPUT_${i})," >> "$OUT_FILE_SOURCE_PINS"
        done

        printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"

        nr_of_analog_inputs=$((16 * "$number_of_mux"))

        {
            printf "%s\n" "DEFINES += NUMBER_OF_MUX=$number_of_mux"
            printf "%s\n" "DEFINES += NUMBER_OF_MUX_INPUTS=16"
            printf "%s\n" "DEFINES += MAX_ADC_CHANNELS=$number_of_mux"
        } >> "$OUT_FILE_MAKEFILE_DEFINES"
    elif [[ $analog_in_type == 4051 ]]
    then
        printf "%s\n" "const core::io::mcuPin_t aInPins[MAX_ADC_CHANNELS] = {" >> "$OUT_FILE_SOURCE_PINS"

        for ((i=0; i<3; i++))
        do
            port=$($YAML_PARSER "$TARGET_DEF_FILE" analog.pins.s"$i".port)
            index=$($YAML_PARSER "$TARGET_DEF_FILE" analog.pins.s"$i".index)

            {
                printf "%s\n" "#define MUX_PORT_S${i} CORE_IO_PORT(${port})"
                printf "%s\n" "#define MUX_PIN_S${i} CORE_IO_PORT_INDEX(${index})"
            } >> "$OUT_FILE_HEADER_PINS"
        done

        number_of_mux=$($YAML_PARSER "$TARGET_DEF_FILE" analog.multiplexers)

        for ((i=0; i<"$number_of_mux"; i++))
        do
            port=$($YAML_PARSER "$TARGET_DEF_FILE" analog.pins.z"$i".port)
            index=$($YAML_PARSER "$TARGET_DEF_FILE" analog.pins.z"$i".index)

            {
                printf "%s\n" "#define MUX_PORT_INPUT_${i} CORE_IO_PORT(${port})"
                printf "%s\n" "#define MUX_PIN_INPUT_${i} CORE_IO_PORT_INDEX(${index})"
            } >> "$OUT_FILE_HEADER_PINS"

            printf "%s\n" "CORE_IO_MCU_PIN_DEF(MUX_PORT_INPUT_${i}, MUX_PIN_INPUT_${i})," >> "$OUT_FILE_SOURCE_PINS"
        done

        printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"

        nr_of_analog_inputs=$((8 * "$number_of_mux"))

        {
            printf "%s\n" "DEFINES += NUMBER_OF_MUX=$number_of_mux"
            printf "%s\n" "DEFINES += NUMBER_OF_MUX_INPUTS=8"
            printf "%s\n" "DEFINES += MAX_ADC_CHANNELS=$number_of_mux"
        } >> "$OUT_FILE_MAKEFILE_DEFINES"
    fi

    if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" analog.indexing)" != "null" ]]
    then
        nr_of_analog_inputs=$($YAML_PARSER "$TARGET_DEF_FILE" analog.indexing --length)

        printf "%s\n" "const uint8_t analogIndexes[NR_OF_ANALOG_INPUTS] = {" >> "$OUT_FILE_SOURCE_PINS"

        for ((i=0; i<nr_of_analog_inputs; i++))
        do
            index=$($YAML_PARSER "$TARGET_DEF_FILE" analog.indexing.["$i"])
            printf "%s\n" "${index}," >> "$OUT_FILE_SOURCE_PINS"
        done

        printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
        printf "%s\n" "DEFINES += ANALOG_INDEXING" >> "$OUT_FILE_MAKEFILE_DEFINES"
    fi

    printf "%s\n" "DEFINES += NR_OF_ANALOG_INPUTS=$nr_of_analog_inputs" >> "$OUT_FILE_MAKEFILE_DEFINES"
else
    {
        printf "%s\n" "DEFINES += NR_OF_ANALOG_INPUTS=0"
        printf "%s\n" "DEFINES += MAX_ADC_CHANNELS=0" 
    } >> "$OUT_FILE_MAKEFILE_DEFINES"
fi

########################################################################################################

#################################################### UNUSED OUTPUTS ####################################################

declare -i unused_pins
unused_pins=$($YAML_PARSER "$TARGET_DEF_FILE" unused-io --length)

if [[ $unused_pins -ne 0 ]]
then
    printf "\n%s\n" "const Board::detail::io::unusedIO_t unusedPins[TOTAL_UNUSED_IO] = {" >> "$OUT_FILE_SOURCE_PINS"

    for ((i=0; i<unused_pins; i++))
    do
        port=$($YAML_PARSER "$TARGET_DEF_FILE" unused-io.["$i"].port)
        index=$($YAML_PARSER "$TARGET_DEF_FILE" unused-io.["$i"].index)
        mode=$($YAML_PARSER "$TARGET_DEF_FILE" unused-io.["$i"].mode)

        {
            printf "%s\n" "#define UNUSED_PORT_${i} CORE_IO_PORT(${port})"
            printf "%s\n" "#define UNUSED_PIN_${i} CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"

        case $mode in
            "in-pull")
                {
                    printf "\n%s\n" "#ifdef __AVR__"
                    printf "%s\n" "{ .pin = { .port= &UNUSED_PORT_${i}, .index = UNUSED_PIN_${i}",
                    printf "%s\n" ".mode = core::io::pinMode_t::input, },"
                    printf "%s\n" "#else"
                    printf "%s\n" "{ .pin = { .port= UNUSED_PORT_${i}, .index = UNUSED_PIN_${i}",
                    printf "%s\n" ".mode = core::io::pinMode_t::input, .pull = core::io::pullMode_t::up, },"
                    printf "%s\n" "#endif"
                    printf "%s\n" ".state = true, },"
                } >> "$OUT_FILE_SOURCE_PINS"
                ;;

            "out-low")
                {
                    printf "\n%s\n" "#ifdef __AVR__"
                    printf "%s\n" "{ .pin = { .port= &UNUSED_PORT_${i}, .index = UNUSED_PIN_${i}",
                    printf "%s\n" ".mode = core::io::pinMode_t::output, },"
                    printf "%s\n" "#else"
                    printf "%s\n" "{ .pin = { .port= UNUSED_PORT_${i}, .index = UNUSED_PIN_${i}",
                    printf "%s\n" ".mode = core::io::pinMode_t::outputPP, .pull = core::io::pullMode_t::none, },"
                    printf "%s\n" "#endif"
                    printf "%s\n" ".state = false, },"
                } >> "$OUT_FILE_SOURCE_PINS"
                ;;

            "out-high")
                {
                    printf "\n%s\n" "#ifdef __AVR__"
                    printf "%s\n" "{ .pin = { .port= &UNUSED_PORT_${i}, .index = UNUSED_PIN_${i}",
                    printf "%s\n" ".mode = core::io::pinMode_t::output, },"
                    printf "%s\n" "#else"
                    printf "%s\n" "{ .pin = { .port= UNUSED_PORT_${i}, .index = UNUSED_PIN_${i}",
                    printf "%s\n" ".mode = core::io::pinMode_t::outputPP, .pull = core::io::pullMode_t::none, }," >> "$OUT_FILE_SOURCE_PINS"
                    printf "%s\n" "#endif"
                    printf "%s\n" ".state = true, }," >> "$OUT_FILE_SOURCE_PINS"
                } >> "$OUT_FILE_SOURCE_PINS"
                ;;

            *)
                echo "Incorrect unused pin mode specified"
                exit 1
                ;;
        esac
    done

    printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
    printf "%s\n" "DEFINES += TOTAL_UNUSED_IO=$unused_pins" >> "$OUT_FILE_MAKEFILE_DEFINES"
fi

########################################################################################################

printf "\n%s" "}" >> "$OUT_FILE_SOURCE_PINS"

printf "\n%s" "#include \"MCU.cpp.include\"" >> "$OUT_FILE_SOURCE_PINS"
printf "\n%s" "#include \"board/common/Map.cpp.include\"" >> "$OUT_FILE_SOURCE_PINS"