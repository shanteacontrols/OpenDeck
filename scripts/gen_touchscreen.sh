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

declare -i total_indicators
declare -i total_screenButtons

total_indicators=$($YAML_PARSER $JSON_FILE indicators --length)
total_screenButtons=$($YAML_PARSER $JSON_FILE screenButtons --length)

{
    printf "%s\n\n" "#include \"database/Database.h\""
    printf "%s\n" "void Database::customInitTouchscreen()"
    printf "%s\n" "{"
} > "$OUT_FILE"

for ((i=0; i<total_indicators; i++))
do
    xPos=$($YAML_PARSER $JSON_FILE indicators.[${i}].xpos)
    yPos=$($YAML_PARSER $JSON_FILE indicators.[${i}].ypos)
    width=$($YAML_PARSER $JSON_FILE indicators.[${i}].width)
    height=$($YAML_PARSER $JSON_FILE indicators.[${i}].height)

    onScreen=$($YAML_PARSER $JSON_FILE indicators.[${i}].screen.on)
    offScreen=$($YAML_PARSER $JSON_FILE indicators.[${i}].screen.off)

    address=$($YAML_PARSER $JSON_FILE indicators.[${i}].address)

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

    if [[ $width -ne 0 && $height -ne 0 ]]
    then
        convert "$IN_DIR_IMAGES"/"$onScreen".bmp -crop "$width"x"$height"+"$xPos"+"$yPos" "$OUT_DIR_ICONS"/"$i".ico
    else
        convert "$IN_DIR_IMAGES"/"$onScreen".bmp -crop 1x1+0+0 "$OUT_DIR_ICONS"/"$i".ico
    fi
done

for ((i=0; i<total_screenButtons; i++))
do
    buttonIndex=$($YAML_PARSER $JSON_FILE screenButtons.[${i}].indexTS)
    screenIndex=$($YAML_PARSER $JSON_FILE screenButtons.[${i}].screen)

    {
        printf "    %s\n" "update(Database::Section::touchscreen_t::pageSwitchEnabled, $buttonIndex, 1);"
        printf "    %s\n\n" "update(Database::Section::touchscreen_t::pageSwitchIndex, $buttonIndex, $screenIndex);"
    } >> "$OUT_FILE"
done

printf "%s\n" "}" >> "$OUT_FILE"