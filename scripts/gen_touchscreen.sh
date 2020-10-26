#!/bin/bash

run_dir="src"

if [[ $(basename "$(pwd)") != "$run_dir" ]]
then
    echo This script must be run from $run_dir directory!
    exit 1
fi

#first argument should be path to the input json file
JSON_FILE=$1

if [[ ! -f "$JSON_FILE" ]]
then
    echo "File $JSON_FILE doesn't exist, nothing to do"
    exit 0
fi

OUT_FILE=${JSON_FILE/.json/.cpp}
IN_DIR_IMAGES=$(dirname "$OUT_FILE")
IN_DIR_IMAGES=${IN_DIR_IMAGES/design/images}/$(basename "$JSON_FILE" .json)
OUT_DIR_ICONS=$IN_DIR_IMAGES/icons

mkdir -p "$OUT_DIR_ICONS"

if [[ "$(command -v jq)" == "" ]]
then
    echo "ERROR: jq not installed"
    exit 1
fi

if [[ "$(command -v convert)" == "" ]]
then
    echo "ERROR: imagemagick not installed"
    exit 1
fi

declare -i total_indicators
declare -i total_screenButtons

total_indicators=$(jq '.indicators | length' "$JSON_FILE")
total_screenButtons=$(jq '.screenButtons | length' "$JSON_FILE")

{
    printf "%s\n\n" "#include \"database/Database.h\""
    printf "%s\n" "void Database::customInitTouchscreen()"
    printf "%s\n" "{"
} > "$OUT_FILE"

for ((i=0; i<total_indicators; i++))
do
    xPos=$(jq '.indicators | .['${i}'] | .xpos' "$JSON_FILE")
    yPos=$(jq '.indicators | .['${i}'] | .ypos' "$JSON_FILE")
    width=$(jq '.indicators | .['${i}'] | .width' "$JSON_FILE")
    height=$(jq '.indicators | .['${i}'] | .height' "$JSON_FILE")
    onScreen=$(jq '.indicators | .['${i}'] | .screen | .on' "$JSON_FILE")
    offScreen=$(jq '.indicators | .['${i}'] | .screen | .off' "$JSON_FILE")

    {
        printf "    %s\n" "update(Database::Section::touchscreen_t::xPos, $i, $xPos);"
        printf "    %s\n" "update(Database::Section::touchscreen_t::yPos, $i, $yPos);"
        printf "    %s\n" "update(Database::Section::touchscreen_t::width, $i, $width);"
        printf "    %s\n" "update(Database::Section::touchscreen_t::height, $i, $height);"
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
    buttonIndex=$(jq '.screenButtons | .['${i}'] | .indexTS' "$JSON_FILE")
    screenIndex=$(jq '.screenButtons | .['${i}'] | .screen' "$JSON_FILE")

    {
        printf "    %s\n" "update(Database::Section::touchscreen_t::pageSwitchEnabled, $buttonIndex, 1);"
        printf "    %s\n\n" "update(Database::Section::touchscreen_t::pageSwitchIndex, $buttonIndex, $screenIndex);"
    } >> "$OUT_FILE"
done

printf "%s\n" "}" >> "$OUT_FILE"