#!/usr/bin/env bash

if [[ "$($yaml_parser "$yaml_file" analog)" != "null" ]]
then
    printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_SUPPORT_ADC)" >> "$out_cmakelists"

    if [[ "$($yaml_parser "$yaml_file" analog.extReference)" == "true" ]]
    then
        printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_ADC_EXT_REF)" >> "$out_cmakelists"
    fi

    if [[ "$($yaml_parser "$yaml_file" analog.inputVoltage)" != "null" ]]
    then
        input_voltage=$($yaml_parser "$yaml_file" analog.inputVoltage | sed 's/\.//g')
        printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_ADC_INPUT_VOLTAGE=$input_voltage)" >> "$out_cmakelists"
    fi

    analog_filter_median=$($yaml_parser "$yaml_file" analog.filter.median)
    analog_filter_ema=$($yaml_parser "$yaml_file" analog.filter.ema)

    # use filters by default if not specified
    if [[ $analog_filter_median != "false" ]]
    then
        printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_ANALOG_FILTER_MEDIAN)" >> "$out_cmakelists"
    fi

    if [[ $analog_filter_ema != "false" ]]
    then
        printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_ANALOG_FILTER_EMA)" >> "$out_cmakelists"
    fi

    analog_in_type=$($yaml_parser "$yaml_file" analog.type)

    declare -i nr_of_analog_inputs
    nr_of_analog_inputs=0

    if [[ $analog_in_type == "native" ]]
    then
        printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_DRIVER_ANALOG_INPUT_NATIVE)" >> "$out_cmakelists"

        nr_of_analog_inputs=$($yaml_parser "$yaml_file" analog.pins --length)

        for ((i=0; i<nr_of_analog_inputs; i++))
        do
            port=$($yaml_parser "$yaml_file" analog.pins.["$i"].port)
            index=$($yaml_parser "$yaml_file" analog.pins.["$i"].index)

            {
                printf "%s\n" "#define PIN_PORT_AIN_${i} CORE_MCU_IO_PIN_PORT_DEF(${port})"
                printf "%s\n" "#define PIN_INDEX_AIN_${i} CORE_MCU_IO_PIN_INDEX_DEF(${index})"
            } >> "$out_header"

        done

        {
            printf "%s\n" "namespace gen {"
            printf "%s\n" "constexpr inline core::mcu::io::pin_t ADC_PIN[PROJECT_TARGET_NR_OF_ADC_CHANNELS] = {"
        } >> "$out_header"

        for ((i=0; i<nr_of_analog_inputs; i++))
        do
            printf "%s\n" "core::mcu::io::pin_t{PIN_PORT_AIN_${i}, PIN_INDEX_AIN_${i}}," >> "$out_header"
        done

        {
            printf "%s\n" "};"
            printf "%s\n" "}"
        } >> "$out_header"

        printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_NR_OF_ADC_CHANNELS=$nr_of_analog_inputs)" >> "$out_cmakelists"
    elif [[ $analog_in_type == 4067 ]]
    then
        printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_DRIVER_ANALOG_INPUT_MULTIPLEXER)" >> "$out_cmakelists"

        for ((i=0; i<4; i++))
        do
            port=$($yaml_parser "$yaml_file" analog.pins.s"$i".port)
            index=$($yaml_parser "$yaml_file" analog.pins.s"$i".index)

            if [[ ($port == "null") || ($index == "null") ]]
            then
                echo "ERROR: Pin s${i} for multiplexer undefined"
                exit 1
            fi

            {
                printf "%s\n" "#define PIN_PORT_MUX_S${i} CORE_MCU_IO_PIN_PORT_DEF(${port})"
                printf "%s\n" "#define PIN_INDEX_MUX_S${i} CORE_MCU_IO_PIN_INDEX_DEF(${index})"
            } >> "$out_header"
        done

        number_of_mux=$($yaml_parser "$yaml_file" analog.multiplexers)

        for ((i=0; i<"$number_of_mux"; i++))
        do
            port=$($yaml_parser "$yaml_file" analog.pins.z"$i".port)
            index=$($yaml_parser "$yaml_file" analog.pins.z"$i".index)

            if [[ ($port == "null") || ($index == "null") ]]
            then
                echo "ERROR: Pin z${i} for multiplexer undefined"
                exit 1
            fi

            {
                printf "%s\n" "#define PIN_PORT_MUX_INPUT_${i} CORE_MCU_IO_PIN_PORT_DEF(${port})"
                printf "%s\n" "#define PIN_INDEX_MUX_INPUT_${i} CORE_MCU_IO_PIN_INDEX_DEF(${index})"
            } >> "$out_header"
        done

        {
            printf "%s\n" "namespace gen {"
            printf "%s\n" "constexpr inline core::mcu::io::pin_t ADC_PIN[PROJECT_TARGET_NR_OF_ADC_CHANNELS] = {"
        } >> "$out_header"

        for ((i=0; i<"$number_of_mux"; i++))
        do
            printf "%s\n" "core::mcu::io::pin_t{PIN_PORT_MUX_INPUT_${i}, PIN_INDEX_MUX_INPUT_${i}}," >> "$out_header"
        done

        {
            printf "%s\n" "};"
            printf "%s\n" "}"
        } >> "$out_header"

        nr_of_analog_inputs=$((16 * "$number_of_mux"))

        {
            printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_NR_OF_MUX=$number_of_mux)"
            printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_NR_OF_MUX_INPUTS=16)"
            printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_NR_OF_ADC_CHANNELS=$number_of_mux)"
        } >> "$out_cmakelists"
    elif [[ $analog_in_type == 4051 ]]
    then
        printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_DRIVER_ANALOG_INPUT_MULTIPLEXER)"

        for ((i=0; i<3; i++))
        do
            port=$($yaml_parser "$yaml_file" analog.pins.s"$i".port)
            index=$($yaml_parser "$yaml_file" analog.pins.s"$i".index)

            if [[ ($port == "null") || ($index == "null") ]]
            then
                echo "ERROR: Pin s${i} for multiplexer undefined"
                exit 1
            fi

            {
                printf "%s\n" "#define PIN_PORT_MUX_S${i} CORE_MCU_IO_PIN_PORT_DEF(${port})"
                printf "%s\n" "#define PIN_INDEX_MUX_S${i} CORE_MCU_IO_PIN_INDEX_DEF(${index})"
            } >> "$out_header"
        done

        number_of_mux=$($yaml_parser "$yaml_file" analog.multiplexers)

        for ((i=0; i<"$number_of_mux"; i++))
        do
            port=$($yaml_parser "$yaml_file" analog.pins.z"$i".port)
            index=$($yaml_parser "$yaml_file" analog.pins.z"$i".index)

            if [[ ($port == "null") || ($index == "null") ]]
            then
                echo "ERROR: Pin z${i} for multiplexer undefined"
                exit 1
            fi

            {
                printf "%s\n" "#define PIN_PORT_MUX_INPUT_${i} CORE_MCU_IO_PIN_PORT_DEF(${port})"
                printf "%s\n" "#define PIN_INDEX_MUX_INPUT_${i} CORE_MCU_IO_PIN_INDEX_DEF(${index})"
            } >> "$out_header"
        done

        {
            printf "%s\n" "namespace gen {"
            printf "%s\n" "constexpr inline core::mcu::io::pin_t ADC_PIN[PROJECT_TARGET_NR_OF_ADC_CHANNELS] = {"
        } >> "$out_header"

        for ((i=0; i<"$number_of_mux"; i++))
        do
            printf "%s\n" "core::mcu::io::pin_t{PIN_PORT_MUX_INPUT_${i}, PIN_INDEX_MUX_INPUT_${i}}," >> "$out_header"
        done

        {
            printf "%s\n" "};"
            printf "%s\n" "}"
        } >> "$out_header"

        nr_of_analog_inputs=$((8 * "$number_of_mux"))

        {
            printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_NR_OF_MUX=$number_of_mux)"
            printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_NR_OF_MUX_INPUTS=8)"
            printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_NR_OF_ADC_CHANNELS=$number_of_mux)"
        } >> "$out_cmakelists"
    elif [[ $analog_in_type == "muxonmux" ]]
    then
        printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_DRIVER_ANALOG_INPUT_MUXONMUX)" >> "$out_cmakelists"

        port=$($yaml_parser "$yaml_file" analog.pins.controller.z.port)
        index=$($yaml_parser "$yaml_file" analog.pins.controller.z.index)

        {
            printf "%s\n" "#define PIN_PORT_MUX_CTRL_INPUT CORE_MCU_IO_PIN_PORT_DEF(${port})"
            printf "%s\n" "#define PIN_INDEX_MUX_CTRL_INPUT CORE_MCU_IO_PIN_INDEX_DEF(${index})"
            printf "%s\n" "namespace gen {"
            printf "%s\n" "constexpr inline core::mcu::io::pin_t ADC_PIN[PROJECT_TARGET_NR_OF_ADC_CHANNELS] = {"
            printf "%s\n" "core::mcu::io::pin_t{PIN_PORT_MUX_CTRL_INPUT, PIN_INDEX_MUX_CTRL_INPUT}," >> "$out_header"
            printf "%s\n" "};"
            printf "%s\n" "}"
        } >> "$out_header"

        for ((i=0; i<4; i++))
        do
            port=$($yaml_parser "$yaml_file" analog.pins.nodes.s"$i".port)
            index=$($yaml_parser "$yaml_file" analog.pins.nodes.s"$i".index)

            if [[ ($port == "null") || ($index == "null") ]]
            then
                echo "ERROR: Pin s${i} for multiplexer nodes undefined"
                exit 1
            fi

            {
                printf "%s\n" "#define PIN_PORT_MUX_NODE_S${i} CORE_MCU_IO_PIN_PORT_DEF(${port})"
                printf "%s\n" "#define PIN_INDEX_MUX_NODE_S${i} CORE_MCU_IO_PIN_INDEX_DEF(${index})"
            } >> "$out_header"
        done

        declare -i number_of_mux
        declare -i number_of_ctrl_pins

        for ((i=0; i<4; i++))
        do
            port=$($yaml_parser "$yaml_file" analog.pins.controller.s"$i".port)
            index=$($yaml_parser "$yaml_file" analog.pins.controller.s"$i".index)

            # For controller multiplexer, at least two pins are expected
            if [[ $i -lt 2 ]]
            then
                if [[ ($port == "null") || ($index == "null") ]]
                then
                    echo "ERROR: Pin s${i} for multiplexer nodes undefined"
                    exit 1
                fi
            fi

            if [[ ($port != "null") && ($index != "null") ]]
            then
                {
                    printf "%s\n" "#define PIN_PORT_MUX_CTRL_S${i} CORE_MCU_IO_PIN_PORT_DEF(${port})"
                    printf "%s\n" "#define PIN_INDEX_MUX_CTRL_S${i} CORE_MCU_IO_PIN_INDEX_DEF(${index})"
                } >> "$out_header"

                ((number_of_ctrl_pins++))
            fi
        done

        number_of_mux=$((2**"$number_of_ctrl_pins"))
        nr_of_analog_inputs=$((16 * "$number_of_mux"))

        {
            printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_NR_OF_MUX=$number_of_mux)"
            printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_NR_OF_MUX_INPUTS=16)"
            printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_NR_OF_ADC_CHANNELS=1)"
        } >> "$out_cmakelists"
    fi

    printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_MAX_NR_OF_ANALOG_INPUTS=$nr_of_analog_inputs)" >> "$out_cmakelists"

    if [[ "$($yaml_parser "$yaml_file" analog.indexing)" != "null" ]]
    then
        nr_of_analog_inputs=$($yaml_parser "$yaml_file" analog.indexing --length)

        {
            printf "%s\n" "namespace gen {"
            printf "%s\n" "constexpr inline uint8_t ADC_INDEX[PROJECT_TARGET_MAX_NR_OF_ANALOG_INPUTS] = {"
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

        {
            printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_INDEXING_ANALOG)"
            printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_SUPPORTED_NR_OF_ANALOG_INPUTS=$nr_of_analog_inputs)"
        } >> "$out_cmakelists"
    else
        printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_SUPPORTED_NR_OF_ANALOG_INPUTS=$nr_of_analog_inputs)" >> "$out_cmakelists"
    fi
else
    {
        printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_MAX_NR_OF_ANALOG_INPUTS=0)"
        printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_SUPPORTED_NR_OF_ANALOG_INPUTS=0)"
        printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_NR_OF_ADC_CHANNELS=0)"
    } >> "$out_cmakelists"
fi