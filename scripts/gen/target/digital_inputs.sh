#!/usr/bin/env bash

if [[ "$($yaml_parser "$yaml_file" buttons)" != "null" ]]
then
    printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_SUPPORT_DIGITAL_INPUTS" >> "$out_makefile"

    digital_in_type=$($yaml_parser "$yaml_file" buttons.type)

    declare -i nr_of_digital_inputs
    nr_of_digital_inputs=0

    if [[ $digital_in_type == native ]]
    then
        printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_DRIVER_DIGITAL_INPUT_NATIVE" >> "$out_makefile"

        nr_of_digital_inputs=$($yaml_parser "$yaml_file" buttons.pins --length)

        unset port_duplicates
        unset port_array
        unset index_array
        unset port_array_unique
        declare -A port_duplicates
        declare -a port_array
        declare -a index_array
        declare -a port_array_unique

        for ((i=0; i<nr_of_digital_inputs; i++))
        do
            port=$($yaml_parser "$yaml_file" buttons.pins.["$i"].port)
            index=$($yaml_parser "$yaml_file" buttons.pins.["$i"].index)

            port_array+=("$port")
            index_array+=("$index")

            {
                printf "%s\n" "#define PIN_PORT_DIN_${i} CORE_MCU_IO_PIN_PORT_DEF(${port})"
                printf "%s\n" "#define PIN_INDEX_DIN_${i} CORE_MCU_IO_PIN_INDEX_DEF(${index})"
            } >> "$out_header"

            if [[ -z ${port_duplicates[$port]} ]]
            then
                port_array_unique+=("$port")
            fi

            port_duplicates["$port"]=1
        done

        printf "%s\n" "#define PROJECT_TARGET_NR_OF_DIGITAL_INPUT_PORTS ${#port_array_unique[@]}" >> "$out_header"

        {
            printf "%s\n" "namespace gen {"
            printf "%s\n" "constexpr inline core::mcu::io::pinPort_t DIGITAL_IN_PORT[PROJECT_TARGET_NR_OF_DIGITAL_INPUT_PORTS] = {"
        } >> "$out_header"

        for ((i=0; i<${#port_array_unique[@]}; i++))
        do
            {
                printf "%s\n" "CORE_MCU_IO_PIN_PORT_DEF(${port_array_unique[$i]}),"
            } >> "$out_header"
        done

        {
            printf "%s\n" "};"
            printf "%s\n" "constexpr inline core::mcu::io::pin_t BUTTON_PIN[PROJECT_TARGET_MAX_NR_OF_DIGITAL_INPUTS] = {"
        } >> "$out_header"

        for ((i=0; i<nr_of_digital_inputs; i++))
        do
            printf "%s\n" "core::mcu::io::pin_t{PIN_PORT_DIN_${i}, PIN_INDEX_DIN_${i}}," >> "$out_header"
        done

        {
            printf "%s\n" "};" >> "$out_header"
        } >> "$out_header"

        {
            printf "%s\n" "constexpr inline uint8_t BUTTON_PORT_INDEX[PROJECT_TARGET_MAX_NR_OF_DIGITAL_INPUTS] = {"
        } >> "$out_header"

        for ((i=0; i<nr_of_digital_inputs; i++))
        do
            for ((port=0; port<${#port_array_unique[@]}; port++))
            do
                if [[ ${port_array[$i]} == "${port_array_unique[$port]}" ]]
                then
                    printf "%s\n" "$port," >> "$out_header"
                fi
            done
        done

        {
            printf "%s\n" "};" >> "$out_header"
            printf "%s\n" "constexpr inline uint8_t BUTTON_PIN_INDEX[PROJECT_TARGET_MAX_NR_OF_DIGITAL_INPUTS] = {"
        } >> "$out_header"

        for ((i=0; i<nr_of_digital_inputs; i++))
        do
            printf "%s\n" "${index_array[i]}," >> "$out_header"
        done

        {
            printf "%s\n" "};" >> "$out_header"
            printf "%s\n" "}"
        } >> "$out_header"

        if [[ "$($yaml_parser "$yaml_file" buttons.extPullups)" == "true" ]]
        then
            printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_BUTTONS_EXT_PULLUPS" >> "$out_makefile"
        fi
    elif [[ $digital_in_type == shiftRegister ]]
    then
        printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_DRIVER_DIGITAL_INPUT_SHIFT_REGISTER" >> "$out_makefile"

        port=$($yaml_parser "$yaml_file" buttons.pins.data.port)
        index=$($yaml_parser "$yaml_file" buttons.pins.data.index)

        {
            printf "%s\n" "#define PIN_PORT_SR_IN_DATA CORE_MCU_IO_PIN_PORT_DEF(${port})"
            printf "%s\n" "#define PIN_INDEX_SR_IN_DATA CORE_MCU_IO_PIN_INDEX_DEF(${index})"
        } >> "$out_header"

        port=$($yaml_parser "$yaml_file" buttons.pins.clock.port)
        index=$($yaml_parser "$yaml_file" buttons.pins.clock.index)

        {
            printf "%s\n" "#define PIN_PORT_SR_IN_CLK CORE_MCU_IO_PIN_PORT_DEF(${port})"
            printf "%s\n" "#define PIN_INDEX_SR_IN_CLK CORE_MCU_IO_PIN_INDEX_DEF(${index})"
        } >> "$out_header"

        port=$($yaml_parser "$yaml_file" buttons.pins.latch.port)
        index=$($yaml_parser "$yaml_file" buttons.pins.latch.index)

        {
            printf "%s\n" "#define PIN_PORT_SR_IN_LATCH CORE_MCU_IO_PIN_PORT_DEF(${port})"
            printf "%s\n" "#define PIN_INDEX_SR_IN_LATCH CORE_MCU_IO_PIN_INDEX_DEF(${index})"
        } >> "$out_header"

        number_of_in_sr=$($yaml_parser "$yaml_file" buttons.shiftRegisters)
        nr_of_digital_inputs=$(( 8 * "$number_of_in_sr"))

        printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_NR_OF_IN_SR=$number_of_in_sr" >> "$out_makefile"
    elif [[ $digital_in_type == matrix ]]
    then
        number_of_rows=0
        number_of_columns=0

        if [[ $($yaml_parser "$yaml_file" buttons.rows.type) == "shiftRegister" ]]
        then
            printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_DRIVER_DIGITAL_INPUT_MATRIX_SHIFT_REGISTER_ROWS" >> "$out_makefile"

            number_of_in_sr=$($yaml_parser "$yaml_file" buttons.rows.shiftRegisters)
            number_of_rows=$((8 * number_of_in_sr))

            port=$($yaml_parser "$yaml_file" buttons.rows.pins.data.port)
            index=$($yaml_parser "$yaml_file" buttons.rows.pins.data.index)

            {
                printf "%s\n" "#define PIN_PORT_SR_IN_DATA CORE_MCU_IO_PIN_PORT_DEF(${port})"
                printf "%s\n" "#define PIN_INDEX_SR_IN_DATA CORE_MCU_IO_PIN_INDEX_DEF(${index})"
            } >> "$out_header"

            port=$($yaml_parser "$yaml_file" buttons.rows.pins.clock.port)
            index=$($yaml_parser "$yaml_file" buttons.rows.pins.clock.index)

            {
                printf "%s\n" "#define PIN_PORT_SR_IN_CLK CORE_MCU_IO_PIN_PORT_DEF(${port})"
                printf "%s\n" "#define PIN_INDEX_SR_IN_CLK CORE_MCU_IO_PIN_INDEX_DEF(${index})"
            } >> "$out_header"

            port=$($yaml_parser "$yaml_file" buttons.rows.pins.latch.port)
            index=$($yaml_parser "$yaml_file" buttons.rows.pins.latch.index)

            {
                printf "%s\n" "#define PIN_PORT_SR_IN_LATCH CORE_MCU_IO_PIN_PORT_DEF(${port})"
                printf "%s\n" "#define PIN_INDEX_SR_IN_LATCH CORE_MCU_IO_PIN_INDEX_DEF(${index})"
            } >> "$out_header"

            printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_NR_OF_IN_SR=$number_of_in_sr" >> "$out_makefile"
        elif [[ $($yaml_parser "$yaml_file" buttons.rows.type) == "native" ]]
        then
            printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_DRIVER_DIGITAL_INPUT_MATRIX_NATIVE_ROWS" >> "$out_makefile"

            number_of_rows=$($yaml_parser "$yaml_file" buttons.rows.pins --length)

           for ((i=0; i<number_of_rows; i++))
            do
                port=$($yaml_parser "$yaml_file" buttons.rows.pins.["$i"].port)
                index=$($yaml_parser "$yaml_file" buttons.rows.pins.["$i"].index)

                {
                    printf "%s\n" "#define PIN_PORT_DIN_${i} CORE_MCU_IO_PIN_PORT_DEF(${port})"
                    printf "%s\n" "#define PIN_INDEX_DIN_${i} CORE_MCU_IO_PIN_INDEX_DEF(${index})"
                } >> "$out_header"
            done

            {
                printf "%s\n" "namespace gen {"
                printf "%s\n" "constexpr inline core::mcu::io::pin_t BUTTON_PIN[PROJECT_TARGET_NR_OF_BUTTON_ROWS] = {"
            } >> "$out_header"

            for ((i=0; i<number_of_rows; i++))
            do
                printf "%s\n" "core::mcu::io::pin_t{PIN_PORT_DIN_${i}, PIN_INDEX_DIN_${i}}," >> "$out_header"
            done

            {
                printf "%s\n" "};"
                printf "%s\n" "}"
            } >> "$out_header"
        else
            echo "Invalid button row type specified"
            exit 1
        fi

        if [[ $($yaml_parser "$yaml_file" buttons.columns.pins --length) -eq 3 ]]
        then
            number_of_columns=8

            for ((i=0; i<3; i++))
            do
                port=$($yaml_parser "$yaml_file" buttons.columns.pins.decA"$i".port)
                index=$($yaml_parser "$yaml_file" buttons.columns.pins.decA"$i".index)

                {
                    printf "%s\n" "#define PIN_PORT_DEC_BM_A${i} CORE_MCU_IO_PIN_PORT_DEF(${port})"
                    printf "%s\n" "#define PIN_INDEX_DEC_BM_A${i} CORE_MCU_IO_PIN_INDEX_DEF(${index})"
                } >> "$out_header"
            done
        else
            echo "Invalid number of columns specified"
            exit 1
        fi

        nr_of_digital_inputs=$(("$number_of_columns" * "$number_of_rows"))

        {
            printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_NR_OF_BUTTON_COLUMNS=$number_of_columns"
            printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_NR_OF_BUTTON_ROWS=$number_of_rows"
        } >> "$out_makefile"
    fi

    printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_MAX_NR_OF_DIGITAL_INPUTS=$nr_of_digital_inputs" >> "$out_makefile"

    if [[ "$($yaml_parser "$yaml_file" buttons.indexing)" != "null" ]]
    then
        nr_of_digital_inputs=$($yaml_parser "$yaml_file" buttons.indexing --length)

        {
            printf "%s\n" "namespace gen {"
            printf "%s\n" "constexpr inline uint8_t BUTTON_INDEX[PROJECT_TARGET_MAX_NR_OF_DIGITAL_INPUTS] = {" 
        } >> "$out_header"

        for ((i=0; i<nr_of_digital_inputs; i++))
        do
            index=$($yaml_parser "$yaml_file" buttons.indexing.["$i"])
            printf "%s\n" "${index}," >> "$out_header"
        done

        {
            printf "%s\n" "};"
            printf "%s\n" "}"
        } >> "$out_header"

        {
            printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_INDEXING_BUTTONS"
            printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_SUPPORTED_NR_OF_DIGITAL_INPUTS=$nr_of_digital_inputs"
        } >> "$out_makefile"
    else
        printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_SUPPORTED_NR_OF_DIGITAL_INPUTS=$nr_of_digital_inputs" >> "$out_makefile"
    fi
else
    {
        printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_MAX_NR_OF_DIGITAL_INPUTS=0"
        printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_SUPPORTED_NR_OF_DIGITAL_INPUTS=0"
    } >> "$out_makefile"
fi