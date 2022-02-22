#!/usr/bin/env bash

run_dir="OpenDeck/src"

if [[ $(pwd) != *"$run_dir" ]]
then
    echo This script must be run from $run_dir directory!
    exit 1
fi

if [[ "$(command -v convert)" == "" ]]
then
    echo "ERROR: imagemagick not installed"
    exit 1
fi

# First argument should be path to the input json file,
# and second directory in which generated file shall be placed.
json_file=$1
input_dir=../bin/touchscreen/img/$(basename "$json_file" .json)
gen_dir=$input_dir/icons
json_parser="dasel -n -p json --plain -f"
out_file=$2/$(basename "$json_file" .json).cpp

echo "Generating touchscreen coordinate file..."

mkdir -p "$gen_dir" "$(dirname "$out_file")"

declare -i total_components

total_components=$($json_parser "$json_file" components --length)

{
    printf "%s\n\n" "#include \"database/Database.h\""
    printf "%s\n" "void Database::customInitTouchscreen()"
    printf "%s\n" "{"
} > "$out_file"

for ((i=0; i<total_components; i++))
do
    if [[ $($json_parser "$json_file" components.[${i}].indicator) != "null" ]]
    then
        xPos=$($json_parser "$json_file" components.[${i}].indicator.xpos)
        yPos=$($json_parser "$json_file" components.[${i}].indicator.ypos)
        width=$($json_parser "$json_file" components.[${i}].indicator.width)
        height=$($json_parser "$json_file" components.[${i}].indicator.height)

        onScreen=$($json_parser "$json_file" components.[${i}].indicator.onScreen)
        offScreen=$($json_parser "$json_file" components.[${i}].indicator.offScreen)

        address=$($json_parser "$json_file" components.[${i}].indicator.address)

        {
            if [[ $address != "null" ]]
            then
                printf "    %s\n" "update(Database::Section::touchscreen_t::xPos, $i, $address);"
            else
                printf "    %s\n" "update(Database::Section::touchscreen_t::xPos, $i, $xPos);"
                printf "    %s\n" "update(Database::Section::touchscreen_t::yPos, $i, $yPos);"
                printf "    %s\n" "update(Database::Section::touchscreen_t::width, $i, $width);"
                printf "    %s\n" "update(Database::Section::touchscreen_t::height, $i, $height);"
            fi

            printf "    %s\n" "update(Database::Section::touchscreen_t::onScreen, $i, $onScreen);"
            printf "    %s\n\n" "update(Database::Section::touchscreen_t::offScreen, $i, $offScreen);"
        } >> "$out_file"

        convert "$input_dir"/"$onScreen".bmp -crop "$width"x"$height"+"$xPos"+"$yPos" "$gen_dir"/"$i".ico
    else
        convert -size 1x1 xc:black "$gen_dir"/"$i".ico
    fi

    if [[ $($json_parser "$json_file" components.[${i}].button) != "null" ]]
    then
        switchesScreen=$($json_parser "$json_file" components.[${i}].button.switchesScreen)
        screenIndex=$($json_parser "$json_file" components.[${i}].button.screenToSwitch)

        {
            printf "    %s\n" "update(Database::Section::touchscreen_t::pageSwitchEnabled, $i, $switchesScreen);"
            printf "    %s\n\n" "update(Database::Section::touchscreen_t::pageSwitchIndex, $i, $screenIndex);"
        } >> "$out_file"
    fi

    if [[ $($json_parser "$json_file" components.[${i}].analog) != "null" ]]
    then
        screen=$($json_parser "$json_file" components.[${i}].analog.screen)
        startXCoordinate=$($json_parser "$json_file" components.[${i}].analog.startXCoordinate)
        endXCoordinate=$($json_parser "$json_file" components.[${i}].analog.endXCoordinate)
        startYCoordinate=$($json_parser "$json_file" components.[${i}].analog.startYCoordinate)
        endYCoordinate=$($json_parser "$json_file" components.[${i}].analog.endYCoordinate)

        if [[ $($json_parser "$json_file" components.[${i}].analog.type) == "horizontal" ]]
        then
            type=0
        else
            type=1
        fi

        resetOnRelease=$($json_parser "$json_file" components.[${i}].analog.resetOnRelease)

        {
            printf "    %s\n" "update(Database::Section::touchscreen_t::analogPage, $i, $screen);"
            printf "    %s\n" "update(Database::Section::touchscreen_t::analogStartXCoordinate, $i, $startXCoordinate);"
            printf "    %s\n" "update(Database::Section::touchscreen_t::analogEndXCoordinate, $i, $endXCoordinate);"
            printf "    %s\n" "update(Database::Section::touchscreen_t::analogStartYCoordinate, $i, $startYCoordinate);"
            printf "    %s\n" "update(Database::Section::touchscreen_t::analogEndYCoordinate, $i, $endYCoordinate);"
            printf "    %s\n" "update(Database::Section::touchscreen_t::analogType, $i, $type);"
            printf "    %s\n\n" "update(Database::Section::touchscreen_t::analogResetOnRelease, $i, $resetOnRelease);"
        } >> "$out_file"
    fi
done

printf "%s\n" "}" >> "$out_file"