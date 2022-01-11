#!/usr/bin/env bash

run_dir="src"

if [[ $(basename "$(pwd)") != "$run_dir" ]]
then
    echo This script must be run from $run_dir directory!
    exit 1
fi

if [[ "$(command -v convert)" == "" ]]
then
    echo "ERROR: imagemagick not installed"
    exit 1
fi

#first argument should be path to the input json file and second directory in which generated file shall be placed
JSON_FILE=$1
INPUT_DIR=../bin/touchscreen/img/$(basename "$JSON_FILE" .json)
GEN_DIR=$INPUT_DIR/icons
JSON_PARSER="dasel -n -p json --plain -f"
OUT_FILE=$2/$(basename "$JSON_FILE" .json).cpp

echo "Generating touchscreen coordinate file..."

mkdir -p "$GEN_DIR" "$(dirname "$OUT_FILE")"

declare -i total_components

total_components=$($JSON_PARSER "$JSON_FILE" components --length)

{
    printf "%s\n\n" "#include \"database/Database.h\""
    printf "%s\n" "void Database::customInitTouchscreen()"
    printf "%s\n" "{"
} > "$OUT_FILE"

for ((i=0; i<total_components; i++))
do
    if [[ $($JSON_PARSER "$JSON_FILE" components.[${i}].indicator) != "null" ]]
    then
        xPos=$($JSON_PARSER "$JSON_FILE" components.[${i}].indicator.xpos)
        yPos=$($JSON_PARSER "$JSON_FILE" components.[${i}].indicator.ypos)
        width=$($JSON_PARSER "$JSON_FILE" components.[${i}].indicator.width)
        height=$($JSON_PARSER "$JSON_FILE" components.[${i}].indicator.height)

        onScreen=$($JSON_PARSER "$JSON_FILE" components.[${i}].indicator.onScreen)
        offScreen=$($JSON_PARSER "$JSON_FILE" components.[${i}].indicator.offScreen)

        address=$($JSON_PARSER "$JSON_FILE" components.[${i}].indicator.address)

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

        convert "$INPUT_DIR"/"$onScreen".bmp -crop "$width"x"$height"+"$xPos"+"$yPos" "$GEN_DIR"/"$i".ico
    else
        convert -size 1x1 xc:black "$GEN_DIR"/"$i".ico
    fi

    if [[ $($JSON_PARSER "$JSON_FILE" components.[${i}].button) != "null" ]]
    then
        switchesScreen=$($JSON_PARSER "$JSON_FILE" components.[${i}].button.switchesScreen)
        screenIndex=$($JSON_PARSER "$JSON_FILE" components.[${i}].button.screenToSwitch)

        {
            printf "    %s\n" "update(Database::Section::touchscreen_t::pageSwitchEnabled, $i, $switchesScreen);"
            printf "    %s\n\n" "update(Database::Section::touchscreen_t::pageSwitchIndex, $i, $screenIndex);"
        } >> "$OUT_FILE"
    fi

    if [[ $($JSON_PARSER "$JSON_FILE" components.[${i}].analog) != "null" ]]
    then
        screen=$($JSON_PARSER "$JSON_FILE" components.[${i}].analog.screen)
        startXCoordinate=$($JSON_PARSER "$JSON_FILE" components.[${i}].analog.startXCoordinate)
        endXCoordinate=$($JSON_PARSER "$JSON_FILE" components.[${i}].analog.endXCoordinate)
        startYCoordinate=$($JSON_PARSER "$JSON_FILE" components.[${i}].analog.startYCoordinate)
        endYCoordinate=$($JSON_PARSER "$JSON_FILE" components.[${i}].analog.endYCoordinate)

        if [[ $($JSON_PARSER "$JSON_FILE" components.[${i}].analog.type) == "horizontal" ]]
        then
            type=0
        else
            type=1
        fi

        resetOnRelease=$($JSON_PARSER "$JSON_FILE" components.[${i}].analog.resetOnRelease)

        {
            printf "    %s\n" "update(Database::Section::touchscreen_t::analogPage, $i, $screen);"
            printf "    %s\n" "update(Database::Section::touchscreen_t::analogStartXCoordinate, $i, $startXCoordinate);"
            printf "    %s\n" "update(Database::Section::touchscreen_t::analogEndXCoordinate, $i, $endXCoordinate);"
            printf "    %s\n" "update(Database::Section::touchscreen_t::analogStartYCoordinate, $i, $startYCoordinate);"
            printf "    %s\n" "update(Database::Section::touchscreen_t::analogEndYCoordinate, $i, $endYCoordinate);"
            printf "    %s\n" "update(Database::Section::touchscreen_t::analogType, $i, $type);"
            printf "    %s\n\n" "update(Database::Section::touchscreen_t::analogResetOnRelease, $i, $resetOnRelease);"
        } >> "$OUT_FILE"
    fi
done

printf "%s\n" "}" >> "$OUT_FILE"