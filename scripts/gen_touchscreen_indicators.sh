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

total_indicators=$(jq '.indicators | length' "$JSON_FILE")

{
    printf "%s\n\n" "#include \"io/touchscreen/Touchscreen.h\""
    printf "%s\n" "#ifdef __AVR__"
    printf "%s\n" "#include <avr/pgmspace.h>"
    printf "%s\n" "#else"
    printf "%s\n" "#define PROGMEM"
    printf "%s\n\n" "#endif"

    printf "%s\n\n" "#define TOTAL_ICONS $total_indicators"
    printf "%s\n" "namespace"
    printf "%s\n" "{"
    printf "%s\n" "#ifdef __AVR__"
    printf "%s\n" "IO::Touchscreen::icon_t ramIcon;"
    printf "%s\n\n" "#endif"
    printf "    %s\n" "const IO::Touchscreen::icon_t icons[TOTAL_ICONS] PROGMEM = {"
} > "$OUT_FILE"

for ((i=0; i<total_indicators; i++))
do
    xPos=$(jq '.indicators | .['${i}'] | .xpos' "$JSON_FILE")
    yPos=$(jq '.indicators | .['${i}'] | .ypos' "$JSON_FILE")
    width=$(jq '.indicators | .['${i}'] | .width' "$JSON_FILE")
    height=$(jq '.indicators | .['${i}'] | .height' "$JSON_FILE")
    onPage=$(jq '.indicators | .['${i}'] | .page | .on' "$JSON_FILE")
    offPage=$(jq '.indicators | .['${i}'] | .page | .off' "$JSON_FILE")

    {
        printf "        %s\n" "{"
        printf "            %s\n" ".xPos = $xPos,"
        printf "            %s\n" ".yPos = $yPos,"
        printf "            %s\n" ".width = $width,"
        printf "            %s\n" ".height = $height,"
        printf "            %s\n" ".onPage = $onPage,"
        printf "            %s\n" ".offPage = $offPage,"
        printf "        %s\n" "},"
    } >> "$OUT_FILE"
done

{
    printf "    %s\n" "};"
    printf "%s\n\n" "}"
} >> "$OUT_FILE"

#now generate icon fetching
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