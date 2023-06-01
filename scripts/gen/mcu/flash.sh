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
declare -i emueeprom_flash_usage

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
    printf "%s\n" "list(APPEND $cmake_mcu_defines_var PROJECT_MCU_SUPPORT_BOOTLOADER)" >> "$out_cmakelists"
else
    # No bootloader, start at first page
    app_start_page=0
fi

printf "%s\n" "list(APPEND $cmake_mcu_defines_var PROJECT_MCU_FLASH_PAGE_APP=$app_start_page)" >> "$out_cmakelists"

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

core_mcu_fw_metadata_offset=$($yaml_parser "$core_yaml_file" flash.app-metadata-offset)
project_mcu_flash_addr_fw_metadata_start=$((app_start_address + core_mcu_fw_metadata_offset))

{
    printf "%s%x%s\n" "set(PROJECT_MCU_FLASH_ADDR_APP_START 0x" "$app_start_address" ")"
    printf "%s\n" "list(APPEND $cmake_mcu_defines_var PROJECT_MCU_FLASH_ADDR_APP_START=$app_start_address)"
    printf "%s\n" "set(PROJECT_MCU_FLASH_ADDR_FW_METADATA_START $project_mcu_flash_addr_fw_metadata_start)"
    printf "%s\n" "list(APPEND $cmake_mcu_defines_var PROJECT_MCU_FLASH_ADDR_FW_METADATA_START=$project_mcu_flash_addr_fw_metadata_start)"
    printf "%s\n" "set(PROJECT_MCU_FLASH_ADDR_FW_METADATA_END $((project_mcu_flash_addr_fw_metadata_start + 4)))"
} >> "$out_cmakelists"

if [[ $boot_supported -ne 0 ]]
then
    {
        printf "%s%x%s\n" "set(PROJECT_MCU_FLASH_ADDR_BOOT_START 0x" "$boot_start_address" ")" >> "$out_cmakelists"
        printf "%s\n" "list(APPEND $cmake_mcu_defines_var PROJECT_MCU_FLASH_ADDR_BOOT_START=$boot_start_address)"
    } >> "$out_cmakelists"
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
        printf "%s\n" "set(PROJECT_MCU_FLASH_PAGE_FACTORY $factory_flash_page)"
        printf "%s\n" "set(PROJECT_MCU_FLASH_PAGE_EEPROM_1 $eeprom_flash_page_1)"
        printf "%s\n" "set(PROJECT_MCU_FLASH_PAGE_EEPROM_2 $eeprom_flash_page_2)"
        printf "%s\n" "list(APPEND $cmake_mcu_defines_var PROJECT_MCU_USE_EMU_EEPROM)"
        printf "%s\n" "list(APPEND $cmake_mcu_defines_var PROJECT_MCU_FLASH_PAGE_FACTORY=$factory_flash_page)"
        printf "%s\n" "list(APPEND $cmake_mcu_defines_var PROJECT_MCU_FLASH_PAGE_EEPROM_1=$eeprom_flash_page_1)"
        printf "%s\n" "list(APPEND $cmake_mcu_defines_var PROJECT_MCU_FLASH_PAGE_EEPROM_2=$eeprom_flash_page_2)"
    } >> "$out_cmakelists"

    if [[ $($yaml_parser "$core_yaml_file" flash.pages) == "null" ]]
    then
        # Common flash page size
        # Extract the size from core yaml config file so that the values are available directly without
        # requiring referencing other variables.
        common_size=$($yaml_parser "$core_yaml_file" flash.page-size)
        emueeprom_page_size=$(((eeprom_flash_page_2 - eeprom_flash_page_1) * common_size))
        emueeprom_flash_usage=$((common_size * 3))

        {
            printf "%s\n" "set(EMU_EEPROM_PAGE_SIZE $emueeprom_page_size)"
            printf "%s\n" "list(APPEND $cmake_mcu_defines_var EMU_EEPROM_PAGE_SIZE=$emueeprom_page_size)"
        } >> "$out_cmakelists"
    else
        emueeprom_factory_size=$($yaml_parser "$core_yaml_file" flash.pages.["$factory_flash_page"].size)
        emueeprom_page1_size=$($yaml_parser "$core_yaml_file" flash.pages.["$eeprom_flash_page_1"].size)
        emueeprom_page2_size=$($yaml_parser "$core_yaml_file" flash.pages.["$eeprom_flash_page_2"].size)

        emueeprom_page_size=$emueeprom_factory_size
        ((emueeprom_page_size-=factory_flash_page_offset))
        emueeprom_flash_usage=$((emueeprom_page_size + emueeprom_page1_size + emueeprom_page2_size))

        {
            printf "%s\n" "set(EMU_EEPROM_PAGE_SIZE $emueeprom_page_size)"
            printf "%s\n" "list(APPEND $cmake_mcu_defines_var EMU_EEPROM_PAGE_SIZE=$emueeprom_page_size)"
        } >> "$out_cmakelists"
    fi

    # When emulated EEPROM is used, one of the pages is factory page with
    # default settings. Database shouldn't be formatted in this case.
    # The values from factory page should be used as initial ones.
    printf "%s\n" "list(APPEND $cmake_mcu_defines_var PROJECT_MCU_DATABASE_INIT_DATA=0)" >> "$out_cmakelists"
else
    # Make the EEPROM size available as an project MCU define as well (useful when cmake file from core is not included).
    eeprom_size=$($yaml_parser "$core_yaml_file" eeprom.size)

    {
        printf "%s\n" "list(APPEND $cmake_mcu_defines_var PROJECT_MCU_DATABASE_INIT_DATA=1)"
        printf "%s\n" "list(APPEND $cmake_mcu_defines_var PROJECT_MCU_EEPROM_SIZE=$eeprom_size)"
    } >> "$out_cmakelists"
fi

if [[ $app_boot_jump_offset != "null" ]]
then
    printf "%s\n" "list(APPEND $cmake_mcu_defines_var PROJECT_MCU_FLASH_OFFSET_APP_JUMP_FROM_BOOTLOADER=$app_boot_jump_offset)" >> "$out_cmakelists"
fi

# Calculate boot size
# First boot page is known, but last isn't - determine

core_mcu_flash_size=$($yaml_parser "$core_yaml_file" flash.size)

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
        printf "%s\n" "set(PROJECT_MCU_FLASH_BOOT_SIZE $boot_size)"
        printf "%s\n" "set(PROJECT_MCU_FLASH_APP_SIZE $((core_mcu_flash_size - boot_size - emueeprom_flash_usage)))"
    } >> "$out_cmakelists"
else
    printf "%s\n" "set(PROJECT_MCU_FLASH_APP_SIZE $((core_mcu_flash_size - emueeprom_flash_usage)))" >> "$out_cmakelists"
fi