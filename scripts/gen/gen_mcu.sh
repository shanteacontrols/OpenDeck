#!/usr/bin/env bash

# core module already provides defines and Makefile for MCU via
# its own gen_mcu.sh script.
# This script is used to provide additional project-specific
# configuration on top of base configuration.

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
base_yaml_file=$1
yaml_file=$2
gen_dir=$3
extFreq=$4
yaml_parser="dasel -n -p yaml --plain -f"
out_header="$gen_dir"/MCU.h
out_makefile="$gen_dir"/MCU.mk

if ! "$script_dir"/../../modules/core/scripts/gen_mcu.sh "$base_yaml_file" "$gen_dir" "$extFreq"
then
    exit 1
fi

echo "Generating MCU configuration..."

mkdir -p "$gen_dir"

app_start_page=$($yaml_parser "$yaml_file" flash.app-start-page)
boot_start_page=$($yaml_parser "$yaml_file" flash.boot-start-page)

if [[ $app_start_page == "null" ]]
then
    app_start_page=0
fi

if [[ $boot_start_page != "null" ]]
then
    {
        printf "%s\n" "DEFINES += BOOTLOADER_SUPPORTED"
    } >> "$out_makefile"
else
    # no bootloader, start at first page
    app_start_page=0
fi

{
    printf "%s\n" "DEFINES += FLASH_PAGE_APP_START=$app_start_page"
} >> "$out_makefile"

if [[ $($yaml_parser "$base_yaml_file" flash.pages) != "null" ]]
then
    app_start_address=$($yaml_parser "$base_yaml_file" flash.pages.["$app_start_page"].address)

    if [[ $boot_start_page != "null" ]]
    then
        boot_start_address=$($yaml_parser "$base_yaml_file" flash.pages.["$boot_start_page"].address)
    fi
else
    # all flash pages have common size
    app_start_address=$($yaml_parser "$base_yaml_file" flash.page-size)
    ((app_start_address*=app_start_page))

    if [[ $boot_start_page != "null" ]]
    then
        boot_start_address=$($yaml_parser "$base_yaml_file" flash.page-size)
        ((boot_start_address*=boot_start_page))
    fi
fi

{
    printf "%s%x\n" "APP_START_ADDR := 0x" "$app_start_address"
    printf "%s\n" 'FW_METADATA_LOCATION := $(shell printf "0x%x" $$(( $(APP_START_ADDR) + $(FW_METADATA_OFFSET) )) )'
} >> "$out_makefile"

if [[ $boot_start_page != "null" ]]
then
    {
        printf "%s%x\n" "BOOT_START_ADDR := 0x" "$boot_start_address"
    } >> "$out_makefile"
fi

if [[ $($yaml_parser "$yaml_file" flash.emueeprom) != "null" ]]
then
    factory_flash_page=$($yaml_parser "$yaml_file"  flash.emueeprom.factory-page.page)
    factory_flash_page_offset=$($yaml_parser "$yaml_file" flash.emueeprom.factory-page.start-offset)
    eeprom_flash_page_1=$($yaml_parser "$yaml_file" flash.emueeprom.page1.page)
    eeprom_flash_page_2=$($yaml_parser "$yaml_file" flash.emueeprom.page2.page)

    if [[ $factory_flash_page_offset == "null" ]]
    then
        factory_flash_page_offset=0
    fi

    {
        printf "%s\n" "#define FLASH_PAGE_FACTORY   $factory_flash_page"
        printf "%s\n" "#define FLASH_PAGE_EEPROM_1  $eeprom_flash_page_1"
        printf "%s\n" "#define FLASH_PAGE_EEPROM_2  $eeprom_flash_page_2"

        printf "%s\n" "constexpr uint32_t FLASH_PAGE_SIZE_EEPROM() {"
        printf "%s\n" "#ifdef FLASH_PAGE_SIZE_COMMON"
        printf "%s\n" "return ((FLASH_PAGE_EEPROM_2 - FLASH_PAGE_EEPROM_1) * FLASH_PAGE_SIZE_COMMON) - $factory_flash_page_offset;"
        printf "%s\n" "#else"
        printf "%s\n" "return FLASH_PAGE_SIZE(FLASH_PAGE_EEPROM_1) - $factory_flash_page_offset;"
        printf "%s\n" "#endif"
        printf "%s\n" "}"
    } >> "$out_header"

    # When emulated EEPROM is used, one of the pages is factory page with
    # default settings. Database shouldn't be formatted in this case.
    # The values from factory page should be used as initial ones.
    printf "%s\n" "DEFINES += INIT_DB_DATA=0" >> "$out_makefile"
else
    printf "%s\n" "DEFINES += INIT_DB_DATA=1" >> "$out_makefile"
fi

if [[ $($yaml_parser "$yaml_file" fuses) != "null" ]]
then
    fuse_unlock=$($yaml_parser "$yaml_file" fuses.unlock)
    fuse_lock=$($yaml_parser "$yaml_file" fuses.lock)
    fuse_ext=$($yaml_parser "$yaml_file" fuses.ext)
    fuse_high=$($yaml_parser "$yaml_file" fuses.high)
    fuse_low=$($yaml_parser "$yaml_file" fuses.low)

    {
        printf "%s\n" "FUSE_UNLOCK := $fuse_unlock"
        printf "%s\n" "FUSE_LOCK := $fuse_lock"
        printf "%s\n" "FUSE_EXT := $fuse_ext"
        printf "%s\n" "FUSE_HIGH := $fuse_high"
        printf "%s\n" "FUSE_LOW := $fuse_low"
    } >> "$out_makefile"
fi