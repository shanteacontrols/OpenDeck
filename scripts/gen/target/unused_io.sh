#!/usr/bin/env bash

declare -i unused_pins
unused_pins=$($yaml_parser "$yaml_file" unused-io --length)

if [[ $unused_pins -ne 0 ]]
then
    for ((i=0; i<unused_pins; i++))
    do
        port=$($yaml_parser "$yaml_file" unused-io.["$i"].port)
        index=$($yaml_parser "$yaml_file" unused-io.["$i"].index)

        {
            printf "%s\n" "#define UNUSED_PORT_${i} CORE_MCU_IO_PIN_PORT_DEF(${port})"
            printf "%s\n" "#define UNUSED_PIN_${i} CORE_MCU_IO_PIN_INDEX_DEF(${index})"
        } >> "$out_header"
    done

    {
        printf "%s\n" "namespace gen {"
        printf "%s\n" "constexpr inline board::detail::io::unused::UnusedIo UNUSED_PIN[PROJECT_TARGET_NR_OF_UNUSED_IO] = {"
    } >> "$out_header"

    for ((i=0; i<unused_pins; i++))
    do
        mode=$($yaml_parser "$yaml_file" unused-io.["$i"].mode)

        case $mode in
            "in-pull")
                {
                    printf "%s\n" "{ .pin = { .port = UNUSED_PORT_${i}, .index = UNUSED_PIN_${i}",
                    printf "\n%s\n" "#ifdef CORE_MCU_ARCH_AVR"
                    printf "%s\n" ".mode = core::mcu::io::pinMode_t::INPUT, },"
                    printf "%s\n" "#else"
                    printf "%s\n" ".mode = core::mcu::io::pinMode_t::INPUT, .pull = core::mcu::io::pullMode_t::UP, },"
                    printf "%s\n" "#endif"
                    printf "%s\n" ".state = true, },"
                } >> "$out_header"
                ;;

            "out-low")
                {
                    printf "%s\n" "{ .pin = { .port = UNUSED_PORT_${i}, .index = UNUSED_PIN_${i}",
                    printf "\n%s\n" "#ifdef CORE_MCU_ARCH_AVR"
                    printf "%s\n" ".mode = core::mcu::io::pinMode_t::OUTPUT, },"
                    printf "%s\n" "#else"
                    printf "%s\n" ".mode = core::mcu::io::pinMode_t::OUTPUT_PP, .pull = core::mcu::io::pullMode_t::NONE, },"
                    printf "%s\n" "#endif"
                    printf "%s\n" ".state = false, },"
                } >> "$out_header"
                ;;

            "out-high")
                {
                    printf "%s\n" "{ .pin = { .port = UNUSED_PORT_${i}, .index = UNUSED_PIN_${i}",
                    printf "\n%s\n" "#ifdef CORE_MCU_ARCH_AVR"
                    printf "%s\n" ".mode = core::mcu::io::pinMode_t::OUTPUT, },"
                    printf "%s\n" "#else"
                    printf "%s\n" ".mode = core::mcu::io::pinMode_t::OUTPUT_PP, .pull = core::mcu::io::pullMode_t::NONE, }," >> "$out_header"
                    printf "%s\n" "#endif"
                    printf "%s\n" ".state = true, }," >> "$out_header"
                } >> "$out_header"
                ;;

            *)
                echo "Incorrect unused pin mode specified"
                exit 1
                ;;
        esac
    done

    {
        printf "%s\n" "};"
        printf "%s\n" "}"
    } >> "$out_header"

    printf "%s\n" "list(APPEND $cmake_defines_var PROJECT_TARGET_NR_OF_UNUSED_IO=$unused_pins)" >> "$out_cmakelists"
fi