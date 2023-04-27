#!/usr/bin/env bash

declare -i app_start_page
declare -i boot_supported
declare -i boot_start_page
declare -i boot_end_page
declare -i app_boot_jump_offset
declare -i boot_size
declare -i factory_flash_page
declare -i factory_flash_page_offset
declare -i eeprom_flash_page_1
declare -i eeprom_flash_page_2
declare -i total_flash_pages

if [[ $($yaml_parser "$project_yaml_file" flash.boot-start-page) != "null" ]]
then
    # Having just "if [[ $boot_start_page != "null" ]]" check later will
    # fail because boot_start_page is defined as integer, so even if
    # dasel returns null for boot-start-page key, the value will be 0
    boot_supported=1
fi

app_start_page=$($yaml_parser "$project_yaml_file" flash.app-start-page)
boot_start_page=$($yaml_parser "$project_yaml_file" flash.boot-start-page)
boot_end_page=0
app_boot_jump_offset=$($yaml_parser "$project_yaml_file" flash.app-boot-jump-offset)
boot_size=0
factory_flash_page=0
factory_flash_page_offset=0
eeprom_flash_page_1=0
eeprom_flash_page_2=0
total_flash_pages=0

if [[ $app_start_page == "null" ]]
then
    app_start_page=0
fi

if [[ $boot_supported -ne 0 ]]
then
    {
        printf "%s\n" "DEFINES += HW_SUPPORT_BOOTLOADER"
    } >> "$out_makefile"
else
    # No bootloader, start at first page
    app_start_page=0
fi

{
    printf "%s\n" "DEFINES += FLASH_PAGE_APP=$app_start_page"
} >> "$out_makefile"

if [[ $($yaml_parser "$core_yaml_file" flash.pages) != "null" ]]
then
    app_start_address=$($yaml_parser "$core_yaml_file" flash.pages.["$app_start_page"].address)

    if [[ $boot_supported -ne 0 ]]
    then
        boot_start_address=$($yaml_parser "$core_yaml_file" flash.pages.["$boot_start_page"].address)
    fi

    total_flash_pages=$($yaml_parser "$core_yaml_file" flash.pages --length)
else
    # All flash pages have common size
    app_start_address=$($yaml_parser "$core_yaml_file" flash.page-size)
    flash_start_address=$($yaml_parser "$core_yaml_file" flash.flash-start)
    ((app_start_address*=app_start_page))
    ((app_start_address+=flash_start_address))

    if [[ $boot_supported -ne 0 ]]
    then
        boot_start_address=$($yaml_parser "$core_yaml_file" flash.page-size)
        ((boot_start_address*=boot_start_page))
        ((boot_start_address+=flash_start_address))
    fi

    total_flash_pages=$($yaml_parser "$core_yaml_file" flash.size)/$($yaml_parser "$core_yaml_file" flash.page-size)
fi

{
    printf "%s%x\n" "FLASH_ADDR_APP_START := 0x" "$app_start_address"
    printf "%s\n" "DEFINES += FLASH_ADDR_APP_START=$app_start_address"
    printf "%s\n" 'FLASH_ADDR_FW_METADATA := $(shell printf "0x%x" $$(( $(FLASH_ADDR_APP_START) + $(CORE_MCU_FW_METADATA_OFFSET) )) )'
    printf "%s\n" 'DEFINES += FLASH_ADDR_FW_METADATA=$(FLASH_ADDR_FW_METADATA)'
} >> "$out_makefile"

if [[ $boot_supported -ne 0 ]]
then
    {
        printf "%s%x\n" "FLASH_ADDR_BOOT_START := 0x" "$boot_start_address"
        printf "%s\n" "DEFINES += FLASH_ADDR_BOOT_START=$boot_start_address"
    } >> "$out_makefile"
fi

if [[ $($yaml_parser "$project_yaml_file" flash.emueeprom) != "null" ]]
then
    factory_flash_page=$($yaml_parser "$project_yaml_file"  flash.emueeprom.factory-page.page)
    factory_flash_page_offset=$($yaml_parser "$project_yaml_file" flash.emueeprom.factory-page.start-offset)
    eeprom_flash_page_1=$($yaml_parser "$project_yaml_file" flash.emueeprom.page1.page)
    eeprom_flash_page_2=$($yaml_parser "$project_yaml_file" flash.emueeprom.page2.page)

    if [[ $factory_flash_page_offset == "null" ]]
    then
        factory_flash_page_offset=0
    fi

    {
        printf "%s\n" "FLASH_PAGE_FACTORY := $factory_flash_page"
        printf "%s\n" "FLASH_PAGE_EEPROM_1 := $eeprom_flash_page_1"
        printf "%s\n" "FLASH_PAGE_EEPROM_2 := $eeprom_flash_page_2"
        printf "%s\n" 'DEFINES += FLASH_PAGE_FACTORY=$(FLASH_PAGE_FACTORY)'
        printf "%s\n" 'DEFINES += FLASH_PAGE_EEPROM_1=$(FLASH_PAGE_EEPROM_1)'
        printf "%s\n" 'DEFINES += FLASH_PAGE_EEPROM_2=$(FLASH_PAGE_EEPROM_2)'
    } >> "$out_makefile"

    if [[ $($yaml_parser "$core_yaml_file" flash.pages) == "null" ]]
    then
        # Common flash page size
        {
            printf "%s\n" 'EMU_EEPROM_PAGE_SIZE := $(shell echo $$(( ($(FLASH_PAGE_EEPROM_2) - $(FLASH_PAGE_EEPROM_1)) * $(CORE_MCU_FLASH_PAGE_SIZE_COMMON) )) )'
            printf "%s\n" 'DEFINES += EMU_EEPROM_PAGE_SIZE=$(EMU_EEPROM_PAGE_SIZE)'
            # Offset is never used when flash pages have the same size
            printf "%s\n" 'EMU_EEPROM_FLASH_USAGE := $(shell echo $$(( $(EMU_EEPROM_PAGE_SIZE) * 3 )) )'
        } >> "$out_makefile"
    else
        emueeprom_factory_size=$($yaml_parser "$core_yaml_file" flash.pages.["$factory_flash_page"].size)
        emueeprom_page1_size=$($yaml_parser "$core_yaml_file" flash.pages.["$eeprom_flash_page_1"].size)
        emueeprom_page2_size=$($yaml_parser "$core_yaml_file" flash.pages.["$eeprom_flash_page_2"].size)

        emueeprom_page_size=$emueeprom_factory_size
        ((emueeprom_page_size-=factory_flash_page_offset))
        emueeprom_flash_usage=$((emueeprom_page_size + emueeprom_page1_size + emueeprom_page2_size))

        {
            printf "%s\n" "EMU_EEPROM_PAGE_SIZE := $emueeprom_page_size"
            printf "%s\n" 'DEFINES += EMU_EEPROM_PAGE_SIZE=$(EMU_EEPROM_PAGE_SIZE)'
            printf "%s\n" "EMU_EEPROM_FLASH_USAGE := $emueeprom_flash_usage"
        } >> "$out_makefile"
    fi

    # When emulated EEPROM is used, one of the pages is factory page with
    # default settings. Database shouldn't be formatted in this case.
    # The values from factory page should be used as initial ones.
    printf "%s\n" "DEFINES += DATABASE_INIT_DATA=0" >> "$out_makefile"
else
    {
        printf "%s\n" "EMU_EEPROM_FLASH_USAGE := 0"
        printf "%s\n" "DEFINES += DATABASE_INIT_DATA=1"
    } >> "$out_makefile"
fi

if [[ $app_boot_jump_offset != "null" ]]
then
    printf "%s\n" "DEFINES += FLASH_OFFSET_APP_JUMP_FROM_BOOTLOADER=$app_boot_jump_offset" >> "$out_makefile"
fi

# Calculate boot size
# First boot page is known, but last isn't - determine

if [[ $boot_supported -ne 0 ]]
then
    boot_end_page=$total_flash_pages

    for ((i=0;i<total_flash_pages;i++))
    do
        if [[ ($app_start_page == "$i") || ($factory_flash_page == "$i") ]]
        then
            if [[ $i -gt $boot_start_page ]]
            then
                boot_end_page=$i
                break
            fi
        fi
    done

    total_boot_pages=$((boot_end_page-boot_start_page))

    if [[ $($yaml_parser "$core_yaml_file" flash.pages) != "null" ]]
    then
        for ((i=0;i<total_boot_pages;i++))
        do
            boot_size+=$($yaml_parser "$core_yaml_file" flash.pages.["$i"].size)
        done
    else
        boot_size=$(($($yaml_parser "$core_yaml_file" flash.page-size)*total_boot_pages))
    fi

    {
        printf "%s%s\n" "BOOT_SIZE := " "$boot_size"
        printf "%s%s\n" "APP_SIZE := " '$(shell echo $$(( $(CORE_MCU_FLASH_SIZE) - $(BOOT_SIZE) - $(EMU_EEPROM_FLASH_USAGE) )) )'
    } >> "$out_makefile"
else
    printf "%s%s\n" "APP_SIZE := " '$(shell echo $$(( $(CORE_MCU_FLASH_SIZE) - $(EMU_EEPROM_FLASH_USAGE) )) )' >> "$out_makefile"
fi