#!/usr/bin/env bash

TARGET_DEF_FILE=$1
GEN_DIR=$2
YAML_PARSER="dasel -n -p yaml --plain -f"
TARGET_NAME=$(basename "$TARGET_DEF_FILE" .yml)

rm -rf "$GEN_DIR"
mkdir -p "$GEN_DIR"

OUT_FILE_HEADER_USB="$GEN_DIR"/USBnames.h
OUT_FILE_HEADER_PINS="$GEN_DIR"/Pins.h
OUT_FILE_SOURCE_PINS="$GEN_DIR"/Pins.cpp
OUT_FILE_MAKEFILE_DEFINES="$GEN_DIR"/Defines.mk

#################################################### CORE ####################################################

arch=$($YAML_PARSER "$TARGET_DEF_FILE" arch)
mcu_family=$($YAML_PARSER "$TARGET_DEF_FILE" mcuFamily)
mcu=$($YAML_PARSER "$TARGET_DEF_FILE" mcu)

{
    printf "%s\n" "MCU_FAMILY := $mcu_family"
    #base mcu without the variant-specific letters at the end
    printf "%s\n" "MCU_BASE := $(echo "$mcu" | rev | cut -c3- | rev)"
    printf "%s\n" "MCU_DIR := board/$arch/variants/$mcu_family/$mcu"
    printf "%s\n" "DEFINES += FW_UID=$(../scripts/fw_uid_gen.sh "$TARGET_NAME")"
} >> "$OUT_FILE_MAKEFILE_DEFINES"

if [[ -f board/$arch/variants/$mcu_family/$mcu/fuses.txt ]]
then
    {
        printf "%s\n" "FUSE_UNLOCK := $(grep ^unlock= < board/"$arch"/variants/"$mcu_family"/"$mcu"/fuses.txt | cut -d= -f2)"
        printf "%s\n" "FUSE_EXT := $(grep ^ext= < board/"$arch"/variants/"$mcu_family"/"$mcu"/fuses.txt | cut -d= -f2)"
        printf "%s\n" "FUSE_HIGH := $(grep ^high= < board/"$arch"/variants/"$mcu_family"/"$mcu"/fuses.txt | cut -d= -f2)"
        printf "%s\n" "FUSE_LOW := $(grep ^low= < board/"$arch"/variants/"$mcu_family"/"$mcu"/fuses.txt | cut -d= -f2)"
        printf "%s\n" "FUSE_LOCK := $(grep ^lock= < board/"$arch"/variants/"$mcu_family"/"$mcu"/fuses.txt | cut -d= -f2)"
    } >> "$OUT_FILE_MAKEFILE_DEFINES"
fi

hse_val=$($YAML_PARSER "$TARGET_DEF_FILE" extClockMhz)

if [[ $hse_val != "null" ]]
then
    printf "%s%s\n" "DEFINES += HSE_VALUE=$hse_val" "000000" >> "$OUT_FILE_MAKEFILE_DEFINES"
fi

printf "%s\n" "DEFINES += BOARD_STRING=\\\"$TARGET_NAME\\\"" >> "$OUT_FILE_MAKEFILE_DEFINES"

########################################################################################################

{
    printf "%s\n\n" "#include \"core/src/general/IO.h\""
} >> "$OUT_FILE_HEADER_PINS"

#################################################### PERIPHERALS ####################################################

if [[ $($YAML_PARSER "$TARGET_DEF_FILE" usb) == "true" ]]
then
    printf "%s\n" "DEFINES += USB_MIDI_SUPPORTED" >> "$OUT_FILE_MAKEFILE_DEFINES"

    {
        printf "%s\n" "#if defined(FW_APP)"
        printf "%s\n" "#define USB_PRODUCT UNICODE_STRING(\"OpenDeck | $TARGET_NAME\")"
        printf "%s\n" "#elif defined(FW_BOOT)"
        printf "%s\n" "#define USB_PRODUCT UNICODE_STRING(\"OpenDeck DFU | $TARGET_NAME\")"
        printf "%s\n" "#elif defined(FW_CDC)"
        printf "%s\n" "#define USB_PRODUCT UNICODE_STRING(\"OpenDeck CDC | $TARGET_NAME\")"
        printf "%s\n" "#endif"
    } >> "$OUT_FILE_HEADER_USB"
fi

if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" usbLink)" != "null" ]]
then
    if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" usbLink.type)" != "null" ]]
    then
        uart_channel_usb_link=$($YAML_PARSER "$TARGET_DEF_FILE" usbLink.uartChannel)
        printf "%s\n" "DEFINES += UART_CHANNEL_USB_LINK=$uart_channel_usb_link" >> "$OUT_FILE_MAKEFILE_DEFINES"

        if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" usbLink.type)" == "master" ]]
        then
            {
                printf "%s\n" "DEFINES += USB_LINK_MCU"
                printf "%s\n" "DEFINES += FW_SELECTOR_NO_VERIFY_CRC"
                printf "%s\n" "#append this only if it wasn't appended already"
                printf "%s\n" 'ifeq (,$(findstring USB_MIDI_SUPPORTED,$(DEFINES)))'
                printf "%s\n" "    DEFINES += USB_MIDI_SUPPORTED"
                printf "%s\n" "endif"
            } >> "$OUT_FILE_MAKEFILE_DEFINES"
        elif [[ "$($YAML_PARSER "$TARGET_DEF_FILE" usbLink.type)" == "slave" ]]
        then
            {
                printf "%s\n" "#make sure slave MCUs don't have USB enabled"
                printf "%s\n" 'DEFINES := $(filter-out USB_MIDI_SUPPORTED,$(DEFINES))'
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

if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" display)" != "null" ]]
then
    {
        printf "%s\n" "DEFINES += DISPLAY_SUPPORTED"
        printf "%s\n" "DEFINES += I2C_CHANNEL_DISPLAY=$($YAML_PARSER "$TARGET_DEF_FILE" display.i2cChannel)"
    } >> "$OUT_FILE_MAKEFILE_DEFINES"
fi

{
    printf "%s\n\n" "#include \"Pins.h\""
    printf "%s\n\n" "namespace {"
} >> "$OUT_FILE_SOURCE_PINS"

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
    touchscreen_components=$($YAML_PARSER "$TARGET_DEF_FILE" touchscreen.components | grep -v null | awk '{print$1}END{if(NR==0)print 0}')

    if [[ "$touchscreen_components" -eq 0 ]]
    then
        echo "Amount of touchscreen components cannot be 0 or undefined"
        exit 1
    fi

    {
        printf "%s\n" "DEFINES += MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS=$touchscreen_components"
        printf "%s\n" "DEFINES += TOUCHSCREEN_SUPPORTED"
        printf "%s\n" "DEFINES += LEDS_SUPPORTED"
        printf "%s\n" "DEFINES += UART_CHANNEL_TOUCHSCREEN=$uart_channel_touchscreen"
    } >> "$OUT_FILE_MAKEFILE_DEFINES"
else
    printf "%s\n" "DEFINES += MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS=0" >> "$OUT_FILE_MAKEFILE_DEFINES"
fi

if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" bootloader.button.activeState)" == "high" ]]
then
    #active high
    printf "%s\n" "DEFINES += BTLDR_BUTTON_AH" >> "$OUT_FILE_MAKEFILE_DEFINES"
fi

if [[ $($YAML_PARSER "$TARGET_DEF_FILE" bootloader.button) != "null" ]]
then
    port=$($YAML_PARSER "$TARGET_DEF_FILE" bootloader.button.port)
    index=$($YAML_PARSER "$TARGET_DEF_FILE" bootloader.button.index)

    {
        printf "%s\n" "#define BTLDR_BUTTON_PORT CORE_IO_PORT(${port})"
        printf "%s\n" "#define BTLDR_BUTTON_PIN CORE_IO_PORT_INDEX(${index})"
    } >> "$OUT_FILE_HEADER_PINS"
elif [[ $($YAML_PARSER "$TARGET_DEF_FILE" bootloader.buttonIndex) != "null" ]]
then
    index=$($YAML_PARSER "$TARGET_DEF_FILE" bootloader.buttonIndex)

    {
        printf "%s\n" "#define BTLDR_BUTTON_INDEX CORE_IO_PORT_INDEX(${index})"
    } >> "$OUT_FILE_HEADER_PINS"
fi

########################################################################################################

#################################################### DIGITAL INPUTS ####################################################

if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" buttons)" != "null" ]]
then
    printf "%s\n" "DEFINES += BUTTONS_SUPPORTED" >> "$OUT_FILE_MAKEFILE_DEFINES"

    digital_in_type=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.type)

    declare -i max_number_of_buttons
    max_number_of_buttons=0

    if [[ $digital_in_type == native ]]
    then
        printf "%s\n" "const core::io::mcuPin_t dInPins[MAX_NUMBER_OF_BUTTONS] = {" >> "$OUT_FILE_SOURCE_PINS"

        max_number_of_buttons=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.pins --length)

        for ((i=0; i<max_number_of_buttons; i++))
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
        max_number_of_buttons=$(( 8 * "$number_of_in_sr"))

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

        max_number_of_buttons=$(("$number_of_columns" * "$number_of_rows"))

        {
            printf "%s\n" "DEFINES += NUMBER_OF_BUTTON_COLUMNS=$number_of_columns"
            printf "%s\n" "DEFINES += NUMBER_OF_BUTTON_ROWS=$number_of_rows"
        } >> "$OUT_FILE_MAKEFILE_DEFINES"
    fi

    if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" buttons.indexing)" != "null" ]]
    then
        max_number_of_buttons=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.indexing --length)

        printf "%s\n" "const uint8_t buttonIndexes[MAX_NUMBER_OF_BUTTONS] = {" >> "$OUT_FILE_SOURCE_PINS"

        for ((i=0; i<max_number_of_buttons; i++))
        do
            index=$($YAML_PARSER "$TARGET_DEF_FILE" buttons.indexing.["$i"])
            printf "%s\n" "${index}," >> "$OUT_FILE_SOURCE_PINS"
        done

        printf "%s\n" "DEFINES += BUTTON_INDEXING" >> "$OUT_FILE_MAKEFILE_DEFINES"
        printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
    fi

    printf "%s\n" "DEFINES += MAX_NUMBER_OF_BUTTONS=$max_number_of_buttons" >> "$OUT_FILE_MAKEFILE_DEFINES"

    if [[ "$max_number_of_buttons" -gt 1 && $($YAML_PARSER "$TARGET_DEF_FILE" buttons.encoders) != "false" ]]
    then
        {
            printf "%s\n" "DEFINES += ENCODERS_SUPPORTED"
            printf "%s\n" "DEFINES += MAX_NUMBER_OF_ENCODERS=$(("$max_number_of_buttons" / 2))"
        } >> "$OUT_FILE_MAKEFILE_DEFINES"
    else
        printf "%s\n" "DEFINES += MAX_NUMBER_OF_ENCODERS=0" >> "$OUT_FILE_MAKEFILE_DEFINES"
    fi
else
    {
        printf "%s\n" "DEFINES += MAX_NUMBER_OF_BUTTONS=0"
        printf "%s\n" "DEFINES += MAX_NUMBER_OF_ENCODERS=0"
    } >> "$OUT_FILE_MAKEFILE_DEFINES"
fi

########################################################################################################

#################################################### DIGITAL OUTPUTS ####################################################

if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" leds.external)" != "null" ]]
then
    {
        printf "%s\n" 'ifeq (,$(findstring LEDS_SUPPORTED,$(DEFINES)))'
        printf "%s\n" '    DEFINES += LEDS_SUPPORTED'
        printf "%s\n" 'endif'
    } >> "$OUT_FILE_MAKEFILE_DEFINES"

    digital_out_type=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.type)

    declare -i max_number_of_leds
    max_number_of_leds=0

    if [[ $digital_out_type == "native" ]]
    then
        printf "%s\n" "const core::io::mcuPin_t dOutPins[MAX_NUMBER_OF_LEDS] = {" >> "$OUT_FILE_SOURCE_PINS"

        max_number_of_leds=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.pins --length)

        for ((i=0; i<max_number_of_leds; i++))
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
        max_number_of_leds=$((number_of_out_sr * 8))

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

        max_number_of_leds=$(("$number_of_led_columns" * "$number_of_led_rows"))

        {
            printf "%s\n" "DEFINES += NUMBER_OF_LED_COLUMNS=$number_of_led_columns"
            printf "%s\n" "DEFINES += NUMBER_OF_LED_ROWS=$number_of_led_rows"
        } >> "$OUT_FILE_MAKEFILE_DEFINES"
    fi

    if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.indexing)" != "null" ]]
    then
        max_number_of_leds=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.indexing --length)

        printf "%s\n" "const uint8_t ledIndexes[MAX_NUMBER_OF_LEDS] = {" >> "$OUT_FILE_SOURCE_PINS"

        for ((i=0; i<max_number_of_leds; i++))
        do
            index=$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.indexing.["$i"])
            printf "%s\n" "${index}," >> "$OUT_FILE_SOURCE_PINS"
        done

        printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
        printf "%s\n" "DEFINES += LED_INDEXING" >> "$OUT_FILE_MAKEFILE_DEFINES"
    fi

    {
        printf "%s\n" "DEFINES += MAX_NUMBER_OF_LEDS=$max_number_of_leds"
        printf "%s\n" "DEFINES += MAX_NUMBER_OF_RGB_LEDS=$(("$max_number_of_leds" / 3))" 
    } >> "$OUT_FILE_MAKEFILE_DEFINES"

    if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" leds.external.invert)" == "true" ]]
    then
        printf "%s\n" "DEFINES += LED_EXT_INVERT" >> "$OUT_FILE_MAKEFILE_DEFINES"
    fi
else
    {
        printf "%s\n" "DEFINES += MAX_NUMBER_OF_LEDS=0"
        printf "%s\n" "DEFINES += MAX_NUMBER_OF_RGB_LEDS=0"
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
    printf "%s\n" "DEFINES += ANALOG_SUPPORTED" >> "$OUT_FILE_MAKEFILE_DEFINES"

    if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" analog.extReference)" == "true" ]]
    then
        printf "%s\n" "DEFINES += ADC_EXT_REF" >> "$OUT_FILE_MAKEFILE_DEFINES"
    fi

    analog_in_type=$($YAML_PARSER "$TARGET_DEF_FILE" analog.type)

    declare -i max_number_of_analog
    max_number_of_analog=0

    if [[ $analog_in_type == "native" ]]
    then
        printf "%s\n" "const core::io::mcuPin_t aInPins[MAX_ADC_CHANNELS] = {" >> "$OUT_FILE_SOURCE_PINS"

        max_number_of_analog=$($YAML_PARSER "$TARGET_DEF_FILE" analog.pins --length)

        for ((i=0; i<max_number_of_analog; i++))
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
            printf "%s\n" "DEFINES += MAX_ADC_CHANNELS=$max_number_of_analog"
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

        max_number_of_analog=$((16 * "$number_of_mux"))

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

        max_number_of_analog=$((8 * "$number_of_mux"))

        {
            printf "%s\n" "DEFINES += NUMBER_OF_MUX=$number_of_mux"
            printf "%s\n" "DEFINES += NUMBER_OF_MUX_INPUTS=8"
            printf "%s\n" "DEFINES += MAX_ADC_CHANNELS=$number_of_mux"
        } >> "$OUT_FILE_MAKEFILE_DEFINES"
    fi

    if [[ "$($YAML_PARSER "$TARGET_DEF_FILE" analog.indexing)" != "null" ]]
    then
        max_number_of_analog=$($YAML_PARSER "$TARGET_DEF_FILE" analog.indexing --length)

        printf "%s\n" "const uint8_t analogIndexes[MAX_NUMBER_OF_ANALOG] = {" >> "$OUT_FILE_SOURCE_PINS"

        for ((i=0; i<max_number_of_analog; i++))
        do
            index=$($YAML_PARSER "$TARGET_DEF_FILE" analog.indexing.["$i"])
            printf "%s\n" "${index}," >> "$OUT_FILE_SOURCE_PINS"
        done

        printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
        printf "%s\n" "DEFINES += ANALOG_INDEXING" >> "$OUT_FILE_MAKEFILE_DEFINES"
    fi

    printf "%s\n" "DEFINES += MAX_NUMBER_OF_ANALOG=$max_number_of_analog" >> "$OUT_FILE_MAKEFILE_DEFINES"
else
    {
        printf "%s\n" "DEFINES += MAX_NUMBER_OF_ANALOG=0"
        printf "%s\n" "DEFINES += MAX_ADC_CHANNELS=0" 
    } >> "$OUT_FILE_MAKEFILE_DEFINES"
fi

########################################################################################################

#################################################### UNUSED OUTPUTS ####################################################

declare -i unused_pins
unused_pins=$($YAML_PARSER "$TARGET_DEF_FILE" unused-io --length)

if [[ $unused_pins -ne 0 ]]
then
    printf "\n%s\n" "const core::io::mcuPin_t unusedPins[TOTAL_UNUSED_IO] = {" >> "$OUT_FILE_SOURCE_PINS"

    for ((i=0; i<unused_pins; i++))
    do
        port=$($YAML_PARSER "$TARGET_DEF_FILE" unused-io.["$i"].port)
        index=$($YAML_PARSER "$TARGET_DEF_FILE" unused-io.["$i"].index)

        {
            printf "%s\n" "#define UNUSED_PORT_${i} CORE_IO_PORT(${port})"
            printf "%s\n" "#define UNUSED_PIN_${i} CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"

        printf "%s\n" "CORE_IO_MCU_PIN_DEF(UNUSED_PORT_${i}, UNUSED_PIN_${i})," >> "$OUT_FILE_SOURCE_PINS"
    done

    printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"

    printf "\n%s\n" "const bool unusedPinsStates[TOTAL_UNUSED_IO] = {" >> "$OUT_FILE_SOURCE_PINS"

    for ((i=0; i<unused_pins; i++))
    do
        state=$($YAML_PARSER "$TARGET_DEF_FILE" unused-io.["$i"].state)

        if [[ $state == "high" ]]
        then
            printf "%s\n" "true," >> "$OUT_FILE_SOURCE_PINS"
        else
            printf "%s\n" "false," >> "$OUT_FILE_SOURCE_PINS"
        fi
    done

    printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
    printf "%s\n" "DEFINES += TOTAL_UNUSED_IO=$unused_pins" >> "$OUT_FILE_MAKEFILE_DEFINES"
fi

########################################################################################################

printf "\n%s" "}" >> "$OUT_FILE_SOURCE_PINS"

printf "\n%s" "#include \"board/common/Map.cpp.include\"" >> "$OUT_FILE_SOURCE_PINS"