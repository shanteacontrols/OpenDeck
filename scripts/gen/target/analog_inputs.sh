#!/usr/bin/env bash

if [[ "$($yaml_parser "$yaml_file" analog)" != "null" ]]
then
    printf "%s\n" "DEFINES += ADC_SUPPORTED" >> "$out_makefile"

    if [[ "$($yaml_parser "$yaml_file" analog.extReference)" == "true" ]]
    then
        printf "%s\n" "DEFINES += ADC_EXT_REF" >> "$out_makefile"
    fi

    analog_filter_median=$($yaml_parser "$yaml_file" analog.filter.median)
    analog_filter_ema=$($yaml_parser "$yaml_file" analog.filter.ema)

    # use filters by default if not specified
    if [[ $analog_filter_median != "false" ]]
    then
        printf "%s\n" "DEFINES += ANALOG_USE_MEDIAN_FILTER" >> "$out_makefile"
    fi

    if [[ $analog_filter_ema != "false" ]]
    then
        printf "%s\n" "DEFINES += ANALOG_USE_EMA_FILTER" >> "$out_makefile"
    fi

    analog_in_type=$($yaml_parser "$yaml_file" analog.type)

    declare -i nr_of_analog_inputs
    nr_of_analog_inputs=0

    if [[ $analog_in_type == "native" ]]
    then
        nr_of_analog_inputs=$($yaml_parser "$yaml_file" analog.pins --length)

        for ((i=0; i<nr_of_analog_inputs; i++))
        do
            port=$($yaml_parser "$yaml_file" analog.pins.["$i"].port)
            index=$($yaml_parser "$yaml_file" analog.pins.["$i"].index)

            {
                printf "%s\n" "#define AIN_PORT_${i} CORE_IO_PIN_PORT_DEF(${port})"
                printf "%s\n" "#define AIN_PIN_${i} CORE_IO_PIN_INDEX_DEF(${index})"
            } >> "$out_header"

        done

        {
            printf "%s\n" "namespace {"
            printf "%s\n" "constexpr inline core::io::mcuPin_t aInPins[MAX_ADC_CHANNELS] = {"
        } >> "$out_header"

        for ((i=0; i<nr_of_analog_inputs; i++))
        do
            printf "%s\n" "CORE_IO_MCU_PIN_VAR(AIN_PORT_${i}, AIN_PIN_${i})," >> "$out_header"
        done

        {
            printf "%s\n" "};"
            printf "%s\n" "}"
        } >> "$out_header"

        {
            printf "%s\n" "DEFINES += MAX_ADC_CHANNELS=$nr_of_analog_inputs"
            printf "%s\n" "DEFINES += NATIVE_ANALOG_INPUTS"
        } >> "$out_makefile"

    elif [[ $analog_in_type == 4067 ]]
    then
        for ((i=0; i<4; i++))
        do
            port=$($yaml_parser "$yaml_file" analog.pins.s"$i".port)
            index=$($yaml_parser "$yaml_file" analog.pins.s"$i".index)

            {
                printf "%s\n" "#define MUX_PORT_S${i} CORE_IO_PIN_PORT_DEF(${port})"
                printf "%s\n" "#define MUX_PIN_S${i} CORE_IO_PIN_INDEX_DEF(${index})"
            } >> "$out_header"
        done

        number_of_mux=$($yaml_parser "$yaml_file" analog.multiplexers)

        for ((i=0; i<"$number_of_mux"; i++))
        do
            port=$($yaml_parser "$yaml_file" analog.pins.z"$i".port)
            index=$($yaml_parser "$yaml_file" analog.pins.z"$i".index)

            {
                printf "%s\n" "#define MUX_PORT_INPUT_${i} CORE_IO_PIN_PORT_DEF(${port})"
                printf "%s\n" "#define MUX_PIN_INPUT_${i} CORE_IO_PIN_INDEX_DEF(${index})"
            } >> "$out_header"
        done

        {
            printf "%s\n" "namespace {"
            printf "%s\n" "constexpr inline core::io::mcuPin_t aInPins[MAX_ADC_CHANNELS] = {"
        } >> "$out_header"

        for ((i=0; i<"$number_of_mux"; i++))
        do
            printf "%s\n" "CORE_IO_MCU_PIN_VAR(MUX_PORT_INPUT_${i}, MUX_PIN_INPUT_${i})," >> "$out_header"
        done

        {
            printf "%s\n" "};"
            printf "%s\n" "}"
        } >> "$out_header"

        nr_of_analog_inputs=$((16 * "$number_of_mux"))

        {
            printf "%s\n" "DEFINES += NUMBER_OF_MUX=$number_of_mux"
            printf "%s\n" "DEFINES += NUMBER_OF_MUX_INPUTS=16"
            printf "%s\n" "DEFINES += MAX_ADC_CHANNELS=$number_of_mux"
        } >> "$out_makefile"
    elif [[ $analog_in_type == 4051 ]]
    then
        for ((i=0; i<3; i++))
        do
            port=$($yaml_parser "$yaml_file" analog.pins.s"$i".port)
            index=$($yaml_parser "$yaml_file" analog.pins.s"$i".index)

            {
                printf "%s\n" "#define MUX_PORT_S${i} CORE_IO_PIN_PORT_DEF(${port})"
                printf "%s\n" "#define MUX_PIN_S${i} CORE_IO_PIN_INDEX_DEF(${index})"
            } >> "$out_header"
        done

        number_of_mux=$($yaml_parser "$yaml_file" analog.multiplexers)

        for ((i=0; i<"$number_of_mux"; i++))
        do
            port=$($yaml_parser "$yaml_file" analog.pins.z"$i".port)
            index=$($yaml_parser "$yaml_file" analog.pins.z"$i".index)

            {
                printf "%s\n" "#define MUX_PORT_INPUT_${i} CORE_IO_PIN_PORT_DEF(${port})"
                printf "%s\n" "#define MUX_PIN_INPUT_${i} CORE_IO_PIN_INDEX_DEF(${index})"
            } >> "$out_header"
        done

        {
            printf "%s\n" "namespace {"
            printf "%s\n" "constexpr inline core::io::mcuPin_t aInPins[MAX_ADC_CHANNELS] = {"
        } >> "$out_header"

        for ((i=0; i<"$number_of_mux"; i++))
        do
            printf "%s\n" "CORE_IO_MCU_PIN_VAR(MUX_PORT_INPUT_${i}, MUX_PIN_INPUT_${i})," >> "$out_header"
        done

        {
            printf "%s\n" "};"
            printf "%s\n" "}"
        } >> "$out_header"

        nr_of_analog_inputs=$((8 * "$number_of_mux"))

        {
            printf "%s\n" "DEFINES += NUMBER_OF_MUX=$number_of_mux"
            printf "%s\n" "DEFINES += NUMBER_OF_MUX_INPUTS=8"
            printf "%s\n" "DEFINES += MAX_ADC_CHANNELS=$number_of_mux"
        } >> "$out_makefile"
    fi

    if [[ "$($yaml_parser "$yaml_file" analog.indexing)" != "null" ]]
    then
        nr_of_analog_inputs=$($yaml_parser "$yaml_file" analog.indexing --length)

        {
            printf "%s\n" "namespace {"
            printf "%s\n" "constexpr inline uint8_t analogIndexes[NR_OF_ANALOG_INPUTS] = {"
        } >> "$out_header"

        for ((i=0; i<nr_of_analog_inputs; i++))
        do
            index=$($yaml_parser "$yaml_file" analog.indexing.["$i"])
            printf "%s\n" "${index}," >> "$out_header"
        done

        {
            printf "%s\n" "};"
            printf "%s\n" "}"
        } >> "$out_header"

        printf "%s\n" "DEFINES += ANALOG_INDEXING" >> "$out_makefile"
    fi

    printf "%s\n" "DEFINES += NR_OF_ANALOG_INPUTS=$nr_of_analog_inputs" >> "$out_makefile"
else
    {
        printf "%s\n" "DEFINES += NR_OF_ANALOG_INPUTS=0"
        printf "%s\n" "DEFINES += MAX_ADC_CHANNELS=0" 
    } >> "$out_makefile"
fi