#!/bin/bash

#first argument should be path to the input json file
JSON_FILE=$1
OUT_FILE=${JSON_FILE/.json/.cpp}

if [[ ! -f "$JSON_FILE" ]]
then
    echo "File $JSON_FILE doesn't exist, nothing to do"
    exit 0
fi

if [[ "$(command -v jq)" == "" ]]
then
    echo "ERROR: jq not installed"
    exit 1
fi

declare -i total_indicators
declare -i total_screenButtons

total_indicators=$(jq '.indicators | length' "$JSON_FILE")
total_screenButtons=$(jq '.screenButtons | length' "$JSON_FILE")

{
    printf "%s\n\n" "#include \"io/touchscreen/Touchscreen.h\""
    printf "%s\n" "#ifdef __AVR__"
    printf "%s\n" "#include <avr/pgmspace.h>"
    printf "%s\n" "#else"
    printf "%s\n" "#define PROGMEM"
    printf "%s\n\n" "#endif"

    printf "%s\n" "#define TOTAL_ICONS $total_indicators"
    printf "%s\n\n" "#define TOTAL_SCREEN_BUTTONS $total_screenButtons"
    printf "%s\n" "namespace"
    printf "%s\n" "{"
    printf "%s\n" "#ifdef __AVR__"
    printf "%s\n" "IO::Touchscreen::icon_t ramIcon;"
    printf "%s\n" "IO::Touchscreen::screenButton_t ramScreenButton;"
    printf "%s\n\n" "#endif"
} > "$OUT_FILE"

printf "    %s\n" "const IO::Touchscreen::icon_t icons[TOTAL_ICONS] PROGMEM = {" >> "$OUT_FILE"

for ((i=0; i<total_indicators; i++))
do
    xPos=$(jq '.indicators | .['${i}'] | .xpos' "$JSON_FILE")
    yPos=$(jq '.indicators | .['${i}'] | .ypos' "$JSON_FILE")
    width=$(jq '.indicators | .['${i}'] | .width' "$JSON_FILE")
    height=$(jq '.indicators | .['${i}'] | .height' "$JSON_FILE")
    onScreen=$(jq '.indicators | .['${i}'] | .screen | .on' "$JSON_FILE")
    offScreen=$(jq '.indicators | .['${i}'] | .screen | .off' "$JSON_FILE")

    {
        printf "        %s\n" "{"
        printf "            %s\n" ".xPos = $xPos,"
        printf "            %s\n" ".yPos = $yPos,"
        printf "            %s\n" ".width = $width,"
        printf "            %s\n" ".height = $height,"
        printf "            %s\n" ".onScreen = $onScreen,"
        printf "            %s\n" ".offScreen = $offScreen,"
        printf "        %s\n" "},"
    } >> "$OUT_FILE"
done

{
    printf "%s\n\n" "    };"
} >> "$OUT_FILE"

printf "    %s\n" "const IO::Touchscreen::screenButton_t screenButtons[TOTAL_SCREEN_BUTTONS] PROGMEM = {" >> "$OUT_FILE"

for ((i=0; i<total_screenButtons; i++))
do
    buttonIndex=$(jq '.screenButtons | .['${i}'] | .indexTS' "$JSON_FILE")
    screenIndex=$(jq '.screenButtons | .['${i}'] | .screen' "$JSON_FILE")

    {
        printf "        %s\n" "{"
        printf "            %s\n" ".indexTS = $buttonIndex,"
        printf "            %s\n" ".screen = $screenIndex,"
        printf "        %s\n" "},"
    } >> "$OUT_FILE"
done

{
    printf "    %s\n" "};"
    printf "%s\n\n" "}"
} >> "$OUT_FILE"

#now generate fetching
{
    printf "%s\n" "bool IO::Touchscreen::getIcon(size_t index, icon_t& icon)"
    printf "%s\n" "{"
    printf "%s\n\n" "    if (index >= TOTAL_ICONS) return false;"
    printf "%s\n" "#ifdef __AVR__"
    printf "%s\n" "    memcpy_P(&ramIcon, &icons[index], sizeof(IO::Touchscreen::icon_t));"
    printf "%s\n" "    icon = ramIcon;"
    printf "%s\n" "#else"
    printf "%s\n" "    icon = icons[index];"
    printf "%s\n\n" "#endif"
    printf "%s\n" "    return true;"
    printf "%s\n\n" "}"
} >> "$OUT_FILE"

{
    printf "%s\n" "bool IO::Touchscreen::isScreenChangeButton(size_t index, size_t& screenID)"
    printf "%s\n" "{"
    printf "%s\n" "    for (size_t i = 0; i < TOTAL_SCREEN_BUTTONS; i++)"
    printf "%s\n" "    {"
    printf "%s\n" "#ifdef __AVR__"
    printf "%s\n" "        memcpy_P(&ramScreenButton, &screenButtons[i], sizeof(IO::Touchscreen::screenButton_t));"
    printf "%s\n" "        if (ramScreenButton.indexTS == index)"
    printf "%s\n" "        {"
    printf "%s\n" "            screenID = ramScreenButton.screen;"
    printf "%s\n" "            return true;"
    printf "%s\n" "        }"
    printf "%s\n" "#else"
    printf "%s\n" "        if (screenButtons[i].indexTS == index)"
    printf "%s\n" "        {"
    printf "%s\n" "            screenID = screenButtons[i].screen;"
    printf "%s\n" "            return true;"
    printf "%s\n" "        }"
    printf "%s\n" "#endif"
    printf "%s\n\n" "    }"
    printf "%s\n" "    return false;"
    printf "%s\n\n" "}"
} >> "$OUT_FILE"