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
            printf "%s\n" "#define UNUSED_PORT_${i} CORE_IO_PIN_PORT_DEF(${port})"
            printf "%s\n" "#define UNUSED_PIN_${i} CORE_IO_PIN_INDEX_DEF(${index})"
        } >> "$out_header"
    done

    {
        printf "%s\n" "namespace {"
        printf "%s\n" "constexpr inline Board::detail::io::unusedIO_t unusedPins[TOTAL_UNUSED_IO] = {"
    } >> "$out_header"

    for ((i=0; i<unused_pins; i++))
    do
        mode=$($yaml_parser "$yaml_file" unused-io.["$i"].mode)

        case $mode in
            "in-pull")
                {
                    printf "%s\n" "{ .pin = { .port = UNUSED_PORT_${i}, .index = UNUSED_PIN_${i}",
                    printf "\n%s\n" "#ifdef CORE_ARCH_AVR"
                    printf "%s\n" ".mode = core::io::pinMode_t::input, },"
                    printf "%s\n" "#else"
                    printf "%s\n" ".mode = core::io::pinMode_t::input, .pull = core::io::pullMode_t::up, },"
                    printf "%s\n" "#endif"
                    printf "%s\n" ".state = true, },"
                } >> "$out_header"
                ;;

            "out-low")
                {
                    printf "%s\n" "{ .pin = { .port = UNUSED_PORT_${i}, .index = UNUSED_PIN_${i}",
                    printf "\n%s\n" "#ifdef CORE_ARCH_AVR"
                    printf "%s\n" ".mode = core::io::pinMode_t::output, },"
                    printf "%s\n" "#else"
                    printf "%s\n" ".mode = core::io::pinMode_t::outputPP, .pull = core::io::pullMode_t::none, },"
                    printf "%s\n" "#endif"
                    printf "%s\n" ".state = false, },"
                } >> "$out_header"
                ;;

            "out-high")
                {
                    printf "%s\n" "{ .pin = { .port = UNUSED_PORT_${i}, .index = UNUSED_PIN_${i}",
                    printf "\n%s\n" "#ifdef CORE_ARCH_AVR"
                    printf "%s\n" ".mode = core::io::pinMode_t::output, },"
                    printf "%s\n" "#else"
                    printf "%s\n" ".mode = core::io::pinMode_t::outputPP, .pull = core::io::pullMode_t::none, }," >> "$out_header"
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

    printf "%s\n" "DEFINES += TOTAL_UNUSED_IO=$unused_pins" >> "$out_makefile"
fi

{
    printf "%s\n" 'ifneq (,$(findstring UART_CHANNEL,$(DEFINES)))'
    printf "%s\n" 'DEFINES += USE_UART'
    printf "%s\n" 'endif'
} >> "$out_makefile"