#!/usr/bin/env bash

for arg in "$@"; do
    case "$arg" in
        --config=*)
            config=${arg#--config=}
            ;;

        --gen-dir=*)
            gen_dir=${arg#--gen-dir=}
            ;;
    esac
done

json_parser="dasel -n -p json --plain -f"
out_src=$gen_dir/touchscreen.cpp

echo "Generating touchscreen coordinate file..."

if [[ ! -d $gen_dir ]]
then
    mkdir -p "$gen_dir"

    declare -i total_components

    total_components=$($json_parser "$config" components --length)

    {
        printf "%s\n\n" "#include \"application/database/database.h\""
        printf "%s\n" "void database::Admin::customInitTouchscreen()"
        printf "%s\n" "{"
    } > "$out_src"

    for ((i=0; i<total_components; i++))
    do
        if [[ $($json_parser "$config" components.[${i}].indicator) != "null" ]]
        then
            xPos=$($json_parser "$config" components.[${i}].indicator.xpos)
            yPos=$($json_parser "$config" components.[${i}].indicator.ypos)
            width=$($json_parser "$config" components.[${i}].indicator.width)
            height=$($json_parser "$config" components.[${i}].indicator.height)

            onScreen=$($json_parser "$config" components.[${i}].indicator.onScreen)
            offScreen=$($json_parser "$config" components.[${i}].indicator.offScreen)

            address=$($json_parser "$config" components.[${i}].indicator.address)

            {
                if [[ $address != "null" ]]
                then
                    printf "    %s\n" "update(database::Config::Section::touchscreen_t::X_POS, $i, $address);"
                else
                    printf "    %s\n" "update(database::Config::Section::touchscreen_t::X_POS, $i, $xPos);"
                    printf "    %s\n" "update(database::Config::Section::touchscreen_t::Y_POS, $i, $yPos);"
                    printf "    %s\n" "update(database::Config::Section::touchscreen_t::WIDTH, $i, $width);"
                    printf "    %s\n" "update(database::Config::Section::touchscreen_t::HEIGHT, $i, $height);"
                fi

                printf "    %s\n" "update(database::Config::Section::touchscreen_t::ON_SCREEN, $i, $onScreen);"
                printf "    %s\n\n" "update(database::Config::Section::touchscreen_t::OFF_SCREEN, $i, $offScreen);"
            } >> "$out_src"
        fi

        if [[ $($json_parser "$config" components.[${i}].button) != "null" ]]
        then
            switchesScreen=$($json_parser "$config" components.[${i}].button.switchesScreen)
            screenIndex=$($json_parser "$config" components.[${i}].button.screenToSwitch)

            {
                printf "    %s\n" "update(database::Config::Section::touchscreen_t::PAGE_SWITCH_ENABLED, $i, $switchesScreen);"
                printf "    %s\n\n" "update(database::Config::Section::touchscreen_t::PAGE_SWITCH_INDEX, $i, $screenIndex);"
            } >> "$out_src"
        fi
    done

    printf "%s\n" "}" >> "$out_src"
fi