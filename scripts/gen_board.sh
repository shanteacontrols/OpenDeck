#!/bin/bash

PIN_FILE=$1
YAML_PARSER="dasel -n -p yaml --plain -f"
TARGET_NAME=$(basename "$PIN_FILE" .yml)
mkdir -p board/gen/"$(basename "$PIN_FILE" .yml)"

OUT_FILE_HEADER_USB=board/gen/"$(basename "$PIN_FILE" .yml)"/USBnames.h
OUT_FILE_HEADER_PINS=board/gen/"$(basename "$PIN_FILE" .yml)"/Pins.h
OUT_FILE_SOURCE_PINS=board/gen/"$(basename "$PIN_FILE" .yml)"/Pins.cpp

declare -i digital_inputs
declare -i digital_inputs_indexed
declare -i digital_outputs
declare -i digital_outputs_indexed
declare -i analog_inputs
declare -i analog_inputs_indexed
declare -i unused_pins

digital_inputs=$($YAML_PARSER "$PIN_FILE" buttons.pins --length)
digital_inputs_indexed=$($YAML_PARSER "$PIN_FILE" buttons.indexing --length)
digital_in_type=$($YAML_PARSER "$PIN_FILE" buttons.type)
digital_outputs=$($YAML_PARSER "$PIN_FILE" leds.external.pins --length)
digital_outputs_indexed=$($YAML_PARSER "$PIN_FILE" leds.external.indexing --length)
digital_out_type=$($YAML_PARSER "$PIN_FILE" leds.external.type)
analog_inputs=$($YAML_PARSER "$PIN_FILE" analog.pins --length)
analog_inputs_indexed=$($YAML_PARSER "$PIN_FILE" analog.indexing --length)
analog_in_type=$($YAML_PARSER "$PIN_FILE" analog.type)
unused_pins=$($YAML_PARSER "$PIN_FILE" unused-io --length)

software_version_major=$(make TARGET="$TARGET_NAME" print-SW_VERSION_MAJOR)
software_version_minor=$(make TARGET="$TARGET_NAME" print-SW_VERSION_MINOR)
software_version_revision=$(make TARGET="$TARGET_NAME" print-SW_VERSION_REVISION)

{
    printf "%s\n" "#if defined(FW_APP)"
    printf "%s\n" "#define USB_PRODUCT UNICODE_STRING(\"OpenDeck | Board: $TARGET_NAME\")"
    printf "%s\n" "#elif defined(FW_BOOT)"
    printf "%s\n" "#define USB_PRODUCT UNICODE_STRING(\"OpenDeck DFU v$software_version_major.$software_version_minor.$software_version_revision | Board: $TARGET_NAME\")"
    printf "%s\n" "#elif defined(FW_CDC)"
    printf "%s\n" "#define USB_PRODUCT UNICODE_STRING(\"OpenDeck CDC | Board: $TARGET_NAME\")"
    printf "%s\n" "#endif"
} > "$OUT_FILE_HEADER_USB"

{
    printf "%s\n\n" "#include \"core/src/general/IO.h\""
} > "$OUT_FILE_HEADER_PINS"

{
    printf "%s\n\n" "#include \"Pins.h\""
    printf "%s\n\n" "namespace {"
} > "$OUT_FILE_SOURCE_PINS"

if [[ $digital_inputs_indexed -ne 0 ]]
then
    digital_inputs=$digital_inputs_indexed

    printf "%s\n" "const uint8_t buttonIndexes[MAX_NUMBER_OF_BUTTONS] = {" >> "$OUT_FILE_SOURCE_PINS"

    for ((i=0; i<digital_inputs; i++))
    do
        index=$($YAML_PARSER "$PIN_FILE" buttons.indexing.["$i"])
        printf "%s\n" "${index}," >> "$OUT_FILE_SOURCE_PINS"
    done

    printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
fi

if [[ $digital_in_type == native ]]
then
    printf "%s\n" "const core::io::mcuPin_t dInPins[MAX_NUMBER_OF_BUTTONS] = {" >> "$OUT_FILE_SOURCE_PINS"

    for ((i=0; i<digital_inputs; i++))
    do
        port=$($YAML_PARSER "$PIN_FILE" buttons.pins.["$i"].port)
        index=$($YAML_PARSER "$PIN_FILE" buttons.pins.["$i"].index)

        {
            printf "%s\n" "#define DIN_PORT_${i} CORE_IO_PORT(${port})"
            printf "%s\n" "#define DIN_PIN_${i} CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"

        printf "%s\n" "CORE_IO_MCU_PIN_DEF(DIN_PORT_${i}, DIN_PIN_${i})," >> "$OUT_FILE_SOURCE_PINS"
    done

    printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
elif [[ $digital_in_type == shiftRegister ]]
then
    port=$($YAML_PARSER "$PIN_FILE" buttons.pins.data.port)
    index=$($YAML_PARSER "$PIN_FILE" buttons.pins.data.index)

    {
        printf "%s\n" "#define SR_IN_DATA_PORT CORE_IO_PORT(${port})"
        printf "%s\n" "#define SR_IN_DATA_PIN CORE_IO_PORT_INDEX(${index})"
    } >> "$OUT_FILE_HEADER_PINS"

    port=$($YAML_PARSER "$PIN_FILE" buttons.pins.clock.port)
    index=$($YAML_PARSER "$PIN_FILE" buttons.pins.clock.index)

    {
        printf "%s\n" "#define SR_IN_CLK_PORT CORE_IO_PORT(${port})"
        printf "%s\n" "#define SR_IN_CLK_PIN CORE_IO_PORT_INDEX(${index})"
    } >> "$OUT_FILE_HEADER_PINS"

    port=$($YAML_PARSER "$PIN_FILE" buttons.pins.latch.port)
    index=$($YAML_PARSER "$PIN_FILE" buttons.pins.latch.index)

    {
        printf "%s\n" "#define SR_IN_LATCH_PORT CORE_IO_PORT(${port})"
        printf "%s\n" "#define SR_IN_LATCH_PIN CORE_IO_PORT_INDEX(${index})"
    } >> "$OUT_FILE_HEADER_PINS"
elif [[ $digital_in_type == matrix ]]
then
    if [[ $($YAML_PARSER "$PIN_FILE" buttons.rows.type) == "shiftRegister" ]]
    then
        port=$($YAML_PARSER "$PIN_FILE" buttons.rows.pins.data.port)
        index=$($YAML_PARSER "$PIN_FILE" buttons.rows.pins.data.index)

        {
            printf "%s\n" "#define SR_IN_DATA_PORT CORE_IO_PORT(${port})"
            printf "%s\n" "#define SR_IN_DATA_PIN CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"

        port=$($YAML_PARSER "$PIN_FILE" buttons.rows.pins.clock.port)
        index=$($YAML_PARSER "$PIN_FILE" buttons.rows.pins.clock.index)

        {
            printf "%s\n" "#define SR_IN_CLK_PORT CORE_IO_PORT(${port})"
            printf "%s\n" "#define SR_IN_CLK_PIN CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"

        port=$($YAML_PARSER "$PIN_FILE" buttons.rows.pins.latch.port)
        index=$($YAML_PARSER "$PIN_FILE" buttons.rows.pins.latch.index)

        {
            printf "%s\n" "#define SR_IN_LATCH_PORT CORE_IO_PORT(${port})"
            printf "%s\n" "#define SR_IN_LATCH_PIN CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"
    elif [[ $($YAML_PARSER "$PIN_FILE" buttons.rows.type) == "native" ]]
    then
        rows=$($YAML_PARSER "$PIN_FILE" buttons.rows.pins --length)

        printf "%s\n" "const core::io::mcuPin_t dInPins[$rows] = {" >> "$OUT_FILE_SOURCE_PINS"

        for ((i=0; i<rows; i++))
        do
            port=$($YAML_PARSER "$PIN_FILE" buttons.rows.pins.["$i"].port)
            index=$($YAML_PARSER "$PIN_FILE" buttons.rows.pins.["$i"].index)

            {
                printf "%s\n" "#define DIN_PORT_${i} CORE_IO_PORT(${port})"
                printf "%s\n" "#define DIN_PIN_${i} CORE_IO_PORT_INDEX(${index})"
            } >> "$OUT_FILE_HEADER_PINS"

            printf "%s\n" "CORE_IO_MCU_PIN_DEF(DIN_PORT_${i}, DIN_PIN_${i})," >> "$OUT_FILE_SOURCE_PINS"
        done

        printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
    fi

    for ((i=0; i<3; i++))
    do
        port=$($YAML_PARSER "$PIN_FILE" buttons.columns.pins.decA"$i".port)
        index=$($YAML_PARSER "$PIN_FILE" buttons.columns.pins.decA"$i".index)

        {
            printf "%s\n" "#define DEC_BM_PORT_A${i} CORE_IO_PORT(${port})"
            printf "%s\n" "#define DEC_BM_PIN_A${i} CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"
    done
fi

if [[ $digital_outputs_indexed -ne 0 ]]
then
    digital_outputs=$digital_outputs_indexed

    printf "%s\n" "const uint8_t ledIndexes[MAX_NUMBER_OF_LEDS] = {" >> "$OUT_FILE_SOURCE_PINS"

    for ((i=0; i<digital_outputs; i++))
    do
        index=$($YAML_PARSER "$PIN_FILE" leds.external.indexing.["$i"])
        printf "%s\n" "${index}," >> "$OUT_FILE_SOURCE_PINS"
    done

    printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
fi

if [[ $digital_out_type == native ]]
then
    printf "%s\n" "const core::io::mcuPin_t dOutPins[MAX_NUMBER_OF_LEDS] = {" >> "$OUT_FILE_SOURCE_PINS"

    for ((i=0; i<digital_outputs; i++))
    do
        port=$($YAML_PARSER "$PIN_FILE" leds.external.pins.["$i"].port)
        index=$($YAML_PARSER "$PIN_FILE" leds.external.pins.["$i"].index)

        {
            printf "%s\n" "#define DOUT_PORT_${i} CORE_IO_PORT(${port})"
            printf "%s\n" "#define DOUT_PIN_${i} CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"

        printf "%s\n" "CORE_IO_MCU_PIN_DEF(DOUT_PORT_${i}, DOUT_PIN_${i})," >> "$OUT_FILE_SOURCE_PINS"
    done

    printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
elif [[ $digital_out_type == shiftRegister ]]
then
    port=$($YAML_PARSER "$PIN_FILE" leds.external.pins.data.port)
    index=$($YAML_PARSER "$PIN_FILE" leds.external.pins.data.index)

    {
        printf "%s\n" "#define SR_OUT_DATA_PORT CORE_IO_PORT(${port})"
        printf "%s\n" "#define SR_OUT_DATA_PIN CORE_IO_PORT_INDEX(${index})"
    } >> "$OUT_FILE_HEADER_PINS"

    port=$($YAML_PARSER "$PIN_FILE" leds.external.pins.clock.port)
    index=$($YAML_PARSER "$PIN_FILE" leds.external.pins.clock.index)

    {
        printf "%s\n" "#define SR_OUT_CLK_PORT CORE_IO_PORT(${port})"
        printf "%s\n" "#define SR_OUT_CLK_PIN CORE_IO_PORT_INDEX(${index})"
    } >> "$OUT_FILE_HEADER_PINS"

    port=$($YAML_PARSER "$PIN_FILE" leds.external.pins.latch.port)
    index=$($YAML_PARSER "$PIN_FILE" leds.external.pins.latch.index)

    {
        printf "%s\n" "#define SR_OUT_LATCH_PORT CORE_IO_PORT(${port})"
        printf "%s\n" "#define SR_OUT_LATCH_PIN CORE_IO_PORT_INDEX(${index})"
    } >> "$OUT_FILE_HEADER_PINS"

    port=$($YAML_PARSER "$PIN_FILE" leds.external.pins.enable.port)
    index=$($YAML_PARSER "$PIN_FILE" leds.external.pins.enable.index)

    {
        printf "%s\n" "#define SR_OUT_OE_PORT CORE_IO_PORT(${port})"
        printf "%s\n" "#define SR_OUT_OE_PIN CORE_IO_PORT_INDEX(${index})"
    } >> "$OUT_FILE_HEADER_PINS"
elif [[ $digital_out_type == matrix ]]
then
    for ((i=0; i<3; i++))
    do
        port=$($YAML_PARSER "$PIN_FILE" leds.external.columns.pins.decA"$i".port)
        index=$($YAML_PARSER "$PIN_FILE" leds.external.columns.pins.decA"$i".index)

        {
            printf "%s\n" "#define DEC_LM_PORT_A${i} CORE_IO_PORT(${port})"
            printf "%s\n" "#define DEC_LM_PIN_A${i} CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"
    done

    rows=$($YAML_PARSER "$PIN_FILE" leds.external.rows.pins --length)
    printf "%s\n" "const core::io::mcuPin_t dOutPins[$rows] = {" >> "$OUT_FILE_SOURCE_PINS"

    for ((i=0; i<"$rows"; i++))
    do
        port=$($YAML_PARSER "$PIN_FILE" leds.external.rows.pins.["$i"].port)
        index=$($YAML_PARSER "$PIN_FILE" leds.external.rows.pins.["$i"].index)

        {
            printf "%s\n" "#define LED_ROW_PORT_${i} CORE_IO_PORT(${port})"
            printf "%s\n" "#define LED_ROW_PIN_${i} CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"

        printf "%s\n" "CORE_IO_MCU_PIN_DEF(LED_ROW_PORT_${i}, LED_ROW_PIN_${i})," >> "$OUT_FILE_SOURCE_PINS"
    done

    printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
fi

if [[ $analog_inputs_indexed -ne 0 ]]
then
    analog_inputs=$analog_inputs_indexed

    printf "%s\n" "const uint8_t analogIndexes[MAX_NUMBER_OF_ANALOG] = {" >> "$OUT_FILE_SOURCE_PINS"

    for ((i=0; i<analog_inputs; i++))
    do
        index=$($YAML_PARSER "$PIN_FILE" analog.indexing.["$i"])
        printf "%s\n" "${index}," >> "$OUT_FILE_SOURCE_PINS"
    done

    printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
fi

if [[ $analog_in_type == native ]]
then
    printf "%s\n" "const core::io::mcuPin_t aInPins[MAX_ADC_CHANNELS] = {" >> "$OUT_FILE_SOURCE_PINS"

    for ((i=0; i<analog_inputs; i++))
    do
        port=$($YAML_PARSER "$PIN_FILE" analog.pins.["$i"].port)
        index=$($YAML_PARSER "$PIN_FILE" analog.pins.["$i"].index)

        {
            printf "%s\n" "#define AIN_PORT_${i} CORE_IO_PORT(${port})"
            printf "%s\n" "#define AIN_PIN_${i} CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"

        printf "%s\n" "CORE_IO_MCU_PIN_DEF(AIN_PORT_${i}, AIN_PIN_${i})," >> "$OUT_FILE_SOURCE_PINS"
    done

        printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
elif [[ $analog_in_type == 4067 ]]
then
    printf "%s\n" "const core::io::mcuPin_t aInPins[MAX_ADC_CHANNELS] = {" >> "$OUT_FILE_SOURCE_PINS"

    for ((i=0; i<4; i++))
    do
        port=$($YAML_PARSER "$PIN_FILE" analog.pins.s"$i".port)
        index=$($YAML_PARSER "$PIN_FILE" analog.pins.s"$i".index)

        {
            printf "%s\n" "#define MUX_PORT_S${i} CORE_IO_PORT(${port})"
            printf "%s\n" "#define MUX_PIN_S${i} CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"
    done

    number_of_mux=$($YAML_PARSER "$PIN_FILE" analog.multiplexers $PIN_FILE)

    for ((i=0; i<"$number_of_mux"; i++))
    do
        port=$($YAML_PARSER "$PIN_FILE" analog.pins.z"$i".port)
        index=$($YAML_PARSER "$PIN_FILE" analog.pins.z"$i".index)

        {
            printf "%s\n" "#define MUX_PORT_INPUT_${i} CORE_IO_PORT(${port})"
            printf "%s\n" "#define MUX_PIN_INPUT_${i} CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"

        printf "%s\n" "CORE_IO_MCU_PIN_DEF(MUX_PORT_INPUT_${i}, MUX_PIN_INPUT_${i})," >> "$OUT_FILE_SOURCE_PINS"
    done

    printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
elif [[ $analog_in_type == 4051 ]]
then
    printf "%s\n" "const core::io::mcuPin_t aInPins[MAX_ADC_CHANNELS] = {" >> "$OUT_FILE_SOURCE_PINS"

    for ((i=0; i<3; i++))
    do
        port=$($YAML_PARSER "$PIN_FILE" analog.pins.s"$i".port)
        index=$($YAML_PARSER "$PIN_FILE" analog.pins.s"$i".index)

        {
            printf "%s\n" "#define MUX_PORT_S${i} CORE_IO_PORT(${port})"
            printf "%s\n" "#define MUX_PIN_S${i} CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"
    done

    number_of_mux=$($YAML_PARSER "$PIN_FILE" analog.multiplexers $PIN_FILE)

    for ((i=0; i<"$number_of_mux"; i++))
    do
        port=$($YAML_PARSER "$PIN_FILE" analog.pins.z"$i".port)
        index=$($YAML_PARSER "$PIN_FILE" analog.pins.z"$i".index)

        {
            printf "%s\n" "#define MUX_PORT_INPUT_${i} CORE_IO_PORT(${port})"
            printf "%s\n" "#define MUX_PIN_INPUT_${i} CORE_IO_PORT_INDEX(${index})"
        } >> "$OUT_FILE_HEADER_PINS"

        printf "%s\n" "CORE_IO_MCU_PIN_DEF(MUX_PORT_INPUT_${i}, MUX_PIN_INPUT_${i})," >> "$OUT_FILE_SOURCE_PINS"
    done

    printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
fi

printf "\n%s" "}" >> "$OUT_FILE_SOURCE_PINS"

if [[ $($YAML_PARSER "$PIN_FILE" bootloader.button) != "null" ]]
then
    port=$($YAML_PARSER "$PIN_FILE" bootloader.button.port)
    index=$($YAML_PARSER "$PIN_FILE" bootloader.button.index)

    {
        printf "%s\n" "#define BTLDR_BUTTON_PORT CORE_IO_PORT(${port})"
        printf "%s\n" "#define BTLDR_BUTTON_PIN CORE_IO_PORT_INDEX(${index})"
    } >> "$OUT_FILE_HEADER_PINS"
elif [[ $($YAML_PARSER "$PIN_FILE" bootloader.buttonIndex) != "null" ]]
then
    index=$($YAML_PARSER "$PIN_FILE" bootloader.buttonIndex)

    {
        printf "%s\n" "#define BTLDR_BUTTON_INDEX CORE_IO_PORT_INDEX(${index})"
    } >> "$OUT_FILE_HEADER_PINS"
fi

if [[ $($YAML_PARSER "$PIN_FILE" leds.internal.pins.din) != "null" ]]
then
    port=$($YAML_PARSER "$PIN_FILE" leds.internal.pins.din.rx.port)
    index=$($YAML_PARSER "$PIN_FILE" leds.internal.pins.din.rx.index)

    {
        printf "%s\n" "#define LED_MIDI_IN_DIN_PORT CORE_IO_PORT(${port})"
        printf "%s\n" "#define LED_MIDI_IN_DIN_PIN CORE_IO_PORT_INDEX(${index})"
    } >> "$OUT_FILE_HEADER_PINS"

    port=$($YAML_PARSER "$PIN_FILE" leds.internal.pins.din.tx.port)
    index=$($YAML_PARSER "$PIN_FILE" leds.internal.pins.din.tx.index)

    {
        printf "%s\n" "#define LED_MIDI_OUT_DIN_PORT CORE_IO_PORT(${port})"
        printf "%s\n" "#define LED_MIDI_OUT_DIN_PIN CORE_IO_PORT_INDEX(${index})"
    } >> "$OUT_FILE_HEADER_PINS"
fi

if [[ $($YAML_PARSER "$PIN_FILE" leds.internal.pins.usb) != "null" ]]
then
    port=$($YAML_PARSER "$PIN_FILE" leds.internal.pins.usb.rx.port)
    index=$($YAML_PARSER "$PIN_FILE" leds.internal.pins.usb.rx.index)

    {
        printf "%s\n" "#define LED_MIDI_IN_USB_PORT CORE_IO_PORT(${port})"
        printf "%s\n" "#define LED_MIDI_IN_USB_PIN CORE_IO_PORT_INDEX(${index})"
    } >> "$OUT_FILE_HEADER_PINS"

    port=$($YAML_PARSER "$PIN_FILE" leds.internal.pins.usb.tx.port)
    index=$($YAML_PARSER "$PIN_FILE" leds.internal.pins.usb.tx.index)

    {
        printf "%s\n" "#define LED_MIDI_OUT_USB_PORT CORE_IO_PORT(${port})"
        printf "%s\n" "#define LED_MIDI_OUT_USB_PIN CORE_IO_PORT_INDEX(${index})"
    } >> "$OUT_FILE_HEADER_PINS"
fi

if [[ $unused_pins -ne 0 ]]
then
    printf "\n%s\n" "const core::io::mcuPin_t unusedPins[TOTAL_UNUSED_IO] = {" >> "$OUT_FILE_SOURCE_PINS"

    for ((i=0; i<unused_pins; i++))
    do
        port=$($YAML_PARSER "$PIN_FILE" unused-io.["$i"].port)
        index=$($YAML_PARSER "$PIN_FILE" unused-io.["$i"].index)

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
        state=$($YAML_PARSER "$PIN_FILE" unused-io.["$i"].state)

        if [[ $state == "high" ]]
        then
            printf "%s\n" "true," >> "$OUT_FILE_SOURCE_PINS"
        else
            printf "%s\n" "false," >> "$OUT_FILE_SOURCE_PINS"
        fi
    done

    printf "%s\n" "};" >> "$OUT_FILE_SOURCE_PINS"
fi

printf "\n%s" "#include \"board/common/Map.cpp.include\"" >> "$OUT_FILE_SOURCE_PINS"