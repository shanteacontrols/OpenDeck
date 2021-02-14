#!/bin/bash

run_dir="src"

if [[ $(basename "$(pwd)") != "$run_dir" ]]
then
    echo This script must be run from $run_dir directory!
    exit 1
fi

#first argument should be path to the input json file
JSON_FILE=$1
YAML_PARSER="dasel -n -p json --plain -f"

if [[ ! -f "$JSON_FILE" ]]
then
    echo "File $JSON_FILE doesn't exist, nothing to do"
    exit 0
fi

OUT_FILE=$(dirname "$JSON_FILE")/gen/$(basename "$JSON_FILE" .json).cpp
IN_DIR_IMAGES=../bin/touchscreen/img/$(basename "$JSON_FILE" .json)
OUT_DIR_ICONS=$IN_DIR_IMAGES/icons

mkdir -p "$OUT_DIR_ICONS" "$(dirname "$OUT_FILE")"

if [[ "$(command -v convert)" == "" ]]
then
    echo "ERROR: imagemagick not installed"
    exit 1
fi

declare -i total_components

total_components=$($YAML_PARSER "$JSON_FILE" components --length)

{
    printf "%s\n\n" "#include \"database/Database.h\""
    printf "%s\n" "void Database::customInitTouchscreen()"
    printf "%s\n" "{"
} > "$OUT_FILE"

for ((i=0; i<total_components; i++))
do
    if [[ $($YAML_PARSER "$JSON_FILE" components.[${i}].indicator) != "null" ]]
    then
        xPos=$($YAML_PARSER "$JSON_FILE" components.[${i}].indicator.xpos)
        yPos=$($YAML_PARSER "$JSON_FILE" components.[${i}].indicator.ypos)
        width=$($YAML_PARSER "$JSON_FILE" components.[${i}].indicator.width)
        height=$($YAML_PARSER "$JSON_FILE" components.[${i}].indicator.height)

        onScreen=$($YAML_PARSER "$JSON_FILE" components.[${i}].indicator.onScreen)
        offScreen=$($YAML_PARSER "$JSON_FILE" components.[${i}].indicator.offScreen)

        address=$($YAML_PARSER "$JSON_FILE" components.[${i}].indicator.address)

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
        } >> "$OUT_FILE"

        convert "$IN_DIR_IMAGES"/"$onScreen".bmp -crop "$width"x"$height"+"$xPos"+"$yPos" "$OUT_DIR_ICONS"/"$i".ico
    else
        convert -size 1x1 xc:black "$OUT_DIR_ICONS"/"$i".ico
    fi

    if [[ $($YAML_PARSER "$JSON_FILE" components.[${i}].button) != "null" ]]
    then
        switchesScreen=$($YAML_PARSER "$JSON_FILE" components.[${i}].button.switchesScreen)
        screenIndex=$($YAML_PARSER "$JSON_FILE" components.[${i}].button.screenToSwitch)

        {
            printf "    %s\n" "update(Database::Section::touchscreen_t::pageSwitchEnabled, $i, $switchesScreen);"
            printf "    %s\n\n" "update(Database::Section::touchscreen_t::pageSwitchIndex, $i, $screenIndex);"
        } >> "$OUT_FILE"
    fi
done

printf "%s\n" "}" >> "$OUT_FILE"