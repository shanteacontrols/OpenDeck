#!/usr/bin/env bash

yaml_file=$1
gen_dir=$2
yaml_parser="dasel -n -p yaml --plain -f"
out_header="$gen_dir"/MCU.h
out_makefile="$gen_dir"/MCU.mk

echo "Generating MCU definitions..."

mkdir -p "$gen_dir"
echo "" > "$out_header"
echo "" > "$out_makefile"

mcu=$(basename "$yaml_file" .yml)
arch=$($yaml_parser "$yaml_file" arch)
mcu_family=$($yaml_parser "$yaml_file" mcuFamily)
vendor=$($yaml_parser "$yaml_file" vendor)
cpu=$($yaml_parser "$yaml_file" cpu)
fpu=$($yaml_parser "$yaml_file" fpu)
float_abi=$($yaml_parser "$yaml_file" float-abi)
define_symbol=$($yaml_parser "$yaml_file" define-symbol)
app_start_address=$($yaml_parser "$yaml_file" flash.app-start)
boot_start_address=$($yaml_parser "$yaml_file" flash.boot-start)
app_metadata_offset=$($yaml_parser "$yaml_file" flash.app-metadata-offset)

{
    printf "%s\n\n" "#pragma once"

    if [[ $mcu == *"stm32"* ]]
    then
        printf "%s\n" "#include \"stm32f4xx_hal.h\""
    fi

    printf "%s\n" "#include \"board/Board.h\""
    printf "%s\n\n" "#include \"board/Internal.h\""
} >> "$out_header"

{
    printf "%s%s\n" '-include $(MAKEFILE_INCLUDE_PREFIX)$(GEN_DIR_ARCH_BASE)/' "$arch/Arch.mk"
    printf "%s%s\n" '-include $(MAKEFILE_INCLUDE_PREFIX)$(GEN_DIR_VENDOR_BASE)/' "$vendor/Vendor.mk"
    printf "%s\n" "ARCH := $arch"
    printf "%s\n" "MCU_FAMILY := $mcu_family"
    printf "%s\n" "VENDOR := $vendor"
    # base mcu without the variant-specific letters at the end
    printf "%s\n" "MCU_BASE := $(echo "$mcu" | rev | cut -c3- | rev)"
    printf "%s\n" "MCU := $mcu"
    printf "%s\n" "MCU_DIR := board/arch/$arch/$vendor/variants/$mcu_family/$mcu"
    printf "%s\n" "CPU := $cpu"
    printf "%s\n" "FPU := $fpu"
    printf "%s\n" "FLOAT-ABI := $float_abi"
    printf "%s\n" "DEFINES += $define_symbol"
    printf "%s%x\n" "APP_START_ADDR := 0x" "$app_start_address"
    printf "%s%x\n" "BOOT_START_ADDR := 0x" "$boot_start_address"
    printf "%s%x\n" "FW_METADATA_LOCATION := 0x" "$((app_start_address+app_metadata_offset))"
} >> "$out_makefile"

if [[ $($yaml_parser "$yaml_file" flash) != "null" ]]
then
    declare -i number_of_flash_pages

    if [[ $($yaml_parser "$yaml_file" flash.pages) != "null" ]]
    then
        number_of_flash_pages=$($yaml_parser "$yaml_file" flash.pages --length)

        printf "%s\n\n" "#define TOTAL_FLASH_PAGES $number_of_flash_pages" >> "$out_header"

        for ((i=0; i<number_of_flash_pages; i++))
        do
            addressStart=$($yaml_parser "$yaml_file" flash.pages.["$i"].address)
            page_size=$($yaml_parser "$yaml_file" flash.pages.["$i"].size)
            addressEnd=$((addressStart+page_size-1))
            app_offset=$($yaml_parser "$yaml_file" flash.pages.["$i"].app-offset)
            app_size=$($yaml_parser "$yaml_file" flash.pages.["$i"].app-size)

            # Based on provided app start page address, find its index and create a new symbol in header.
            # This could be done in application as well, but it's done here to avoid extra processing and flash usage.
            if [[ ($addressStart -le $app_start_address) && ($app_start_address -le $addressEnd) ]]
            then
                printf "%s\n" "#define FLASH_PAGE_APP_START $i" >> "$out_header"
            fi

            if [[ $app_offset == "null" ]]
            then
                app_offset=0
            fi

            if [[ $app_size == "null" ]]
            then
                app_size=$page_size
            fi

            {
                printf "%s\n" "#ifdef FW_BOOT"
                printf "%s\n" "#define FLASH_PAGE_ADDRESS_${i} $addressStart"
                printf "%s\n" "#define FLASH_PAGE_SIZE_${i} $page_size"
                printf "%s\n" "#else"
                printf "%s\n" "#define FLASH_PAGE_ADDRESS_${i} (($addressStart+$app_offset))"
                printf "%s\n" "#define FLASH_PAGE_SIZE_${i} $app_size"
                printf "%s\n" "#endif"
            } >> "$out_header"
        done

        {
            printf "%s\n" "namespace {"
            printf "%s\n" "constexpr inline Board::detail::flash::flashPage_t pageDescriptor[TOTAL_FLASH_PAGES] = {"
        } >> "$out_header"

        for ((i=0; i<number_of_flash_pages; i++))
        do
            {
                printf "%s\n" "{"
                printf "%s\n" ".address = FLASH_PAGE_ADDRESS_${i}",
                printf "%s\n" ".size = FLASH_PAGE_SIZE_${i}",
                printf "%s\n" "},"
            } >> "$out_header"
        done

        {
            printf "%s\n" "};"
            printf "%s\n" "}"
        } >> "$out_header"

        {
            printf "%s\n" "#define _FLASH_PAGE_ADDRESS_GEN(x) FLASH_PAGE_ADDRESS_##x"
            printf "%s\n" "#define FLASH_PAGE_ADDRESS(x)      _FLASH_PAGE_ADDRESS_GEN(x)"

            printf "%s\n" "#define _FLASH_PAGE_SIZE_GEN(x) FLASH_PAGE_SIZE_##x"
            printf "%s\n" "#define FLASH_PAGE_SIZE(x)      _FLASH_PAGE_SIZE_GEN(x)"
        } >> "$out_header"
    else
        # all flash pages have common size
        page_size=$($yaml_parser "$yaml_file" flash.page-size)
        flash_size=$($yaml_parser "$yaml_file" flash.size)
        number_of_flash_pages=$((flash_size/page_size))

        {
            printf "%s\n" "#define TOTAL_FLASH_PAGES $number_of_flash_pages"
            printf "%s\n" "#define FLASH_PAGE_SIZE_COMMON $page_size"
            printf "%s\n" "#define FLASH_END $((flash_size-1))"
        } >> "$out_header"

        addressStart=$($yaml_parser "$yaml_file" flash.flash-start)

        for ((i=0; i<number_of_flash_pages; i++))
        do
            addressEnd=$((addressStart+page_size-1))

            if [[ ($addressStart -le $app_start_address) && ($app_start_address -le $addressEnd) ]]
            then
                printf "%s\n" "#define FLASH_PAGE_APP_START $i" >> "$out_header"
            fi

            ((addressStart+=page_size))
        done

        {
            printf "%s\n" "constexpr uint32_t FLASH_PAGE_SIZE(size_t page) {"
            printf "%s\n" "return FLASH_PAGE_SIZE_COMMON; }"

            printf "%s\n" "constexpr uint32_t FLASH_PAGE_ADDRESS(size_t page) {"
            printf "%s\n" "return FLASH_PAGE_SIZE_COMMON * page; }"
        } >> "$out_header"
    fi
fi

if [[ $($yaml_parser "$yaml_file" eeprom) != "null" ]]
then
    if [[ $($yaml_parser "$yaml_file" eeprom.emulated) != "null" ]]
    then
        factory_flash_page=$($yaml_parser "$yaml_file"  eeprom.emulated.factory-flash-page)
        eeprom_flash_page_1=$($yaml_parser "$yaml_file" eeprom.emulated.eeprom-flash-page1)
        eeprom_flash_page_2=$($yaml_parser "$yaml_file" eeprom.emulated.eeprom-flash-page2)

        {
            printf "%s\n" "#define FLASH_PAGE_FACTORY   $factory_flash_page"
            printf "%s\n" "#define FLASH_PAGE_EEPROM_1  $eeprom_flash_page_1"
            printf "%s\n" "#define FLASH_PAGE_EEPROM_2  $eeprom_flash_page_2"
        } >> "$out_header"
    else
        eeprom_size=$($yaml_parser "$yaml_file" eeprom.size)
        printf "%s\n" "#define EEPROM_END $((eeprom_size-1))" >> "$out_header"
    fi
fi

if [[ $($yaml_parser "$yaml_file" clocks) != "null" ]]
then
    pllm_8mhz=$($yaml_parser "$yaml_file" clocks.hse8MHz.pllm)
    plln_8mhz=$($yaml_parser "$yaml_file" clocks.hse8MHz.plln)
    pllq_8mhz=$($yaml_parser "$yaml_file" clocks.hse8MHz.pllq)
    pllp_8mhz=$($yaml_parser "$yaml_file" clocks.hse8MHz.pllp)
    ahb_clk_div_8mhz=$($yaml_parser "$yaml_file" clocks.hse8MHz.ahb_clk_div)
    apb1_clk_div_8mhz=$($yaml_parser "$yaml_file" clocks.hse8MHz.apb1_clk_div)
    apb2_clk_div_8mhz=$($yaml_parser "$yaml_file" clocks.hse8MHz.apb2_clk_div)

    pllm_12mhz=$($yaml_parser "$yaml_file" clocks.hse12MHz.pllm)
    plln_12mhz=$($yaml_parser "$yaml_file" clocks.hse12MHz.plln)
    pllq_12mhz=$($yaml_parser "$yaml_file" clocks.hse12MHz.pllq)
    pllp_12mhz=$($yaml_parser "$yaml_file" clocks.hse12MHz.pllp)
    ahb_clk_div_12mhz=$($yaml_parser "$yaml_file" clocks.hse12MHz.ahb_clk_div)
    apb1_clk_div_12mhz=$($yaml_parser "$yaml_file" clocks.hse12MHz.apb1_clk_div)
    apb2_clk_div_12mhz=$($yaml_parser "$yaml_file" clocks.hse12MHz.apb2_clk_div)

    pllm_16mhz=$($yaml_parser "$yaml_file" clocks.hse16MHz.pllm)
    plln_16mhz=$($yaml_parser "$yaml_file" clocks.hse16MHz.plln)
    pllq_16mhz=$($yaml_parser "$yaml_file" clocks.hse16MHz.pllq)
    pllp_16mhz=$($yaml_parser "$yaml_file" clocks.hse16MHz.pllp)
    ahb_clk_div_16mhz=$($yaml_parser "$yaml_file" clocks.hse16MHz.ahb_clk_div)
    apb1_clk_div_16mhz=$($yaml_parser "$yaml_file" clocks.hse16MHz.apb1_clk_div)
    apb2_clk_div_16mhz=$($yaml_parser "$yaml_file" clocks.hse16MHz.apb2_clk_div)

    pllm_25mhz=$($yaml_parser "$yaml_file" clocks.hse25MHz.pllm)
    plln_25mhz=$($yaml_parser "$yaml_file" clocks.hse25MHz.plln)
    pllq_25mhz=$($yaml_parser "$yaml_file" clocks.hse25MHz.pllq)
    pllp_25mhz=$($yaml_parser "$yaml_file" clocks.hse25MHz.pllp)
    ahb_clk_div_25mhz=$($yaml_parser "$yaml_file" clocks.hse25MHz.ahb_clk_div)
    apb1_clk_div_25mhz=$($yaml_parser "$yaml_file" clocks.hse25MHz.apb1_clk_div)
    apb2_clk_div_25mhz=$($yaml_parser "$yaml_file" clocks.hse25MHz.apb2_clk_div)

    {
        printf "%s\n" "#if (HSE_VALUE == 8000000)"
        printf "%s\n" "#define HSE_PLLM $pllm_8mhz"
        printf "%s\n" "#define HSE_PLLN $plln_8mhz"
        printf "%s\n" "#define HSE_PLLQ $pllq_8mhz"
        printf "%s\n" "#define HSE_PLLP $pllp_8mhz"
        printf "%s\n" "#define AHB_CLK_DIV $ahb_clk_div_8mhz"
        printf "%s\n" "#define APB1_CLK_DIV $apb1_clk_div_8mhz"
        printf "%s\n" "#define APB2_CLK_DIV $apb2_clk_div_8mhz"
        printf "%s\n" "#elif (HSE_VALUE == 12000000)"
        printf "%s\n" "#define HSE_PLLM $pllm_12mhz"
        printf "%s\n" "#define HSE_PLLN $plln_12mhz"
        printf "%s\n" "#define HSE_PLLQ $pllq_12mhz"
        printf "%s\n" "#define HSE_PLLP $pllp_12mhz"
        printf "%s\n" "#define AHB_CLK_DIV $ahb_clk_div_12mhz"
        printf "%s\n" "#define APB1_CLK_DIV $apb1_clk_div_12mhz"
        printf "%s\n" "#define APB2_CLK_DIV $apb2_clk_div_12mhz"
        printf "%s\n" "#elif (HSE_VALUE == 16000000)"
        printf "%s\n" "#define HSE_PLLM $pllm_16mhz"
        printf "%s\n" "#define HSE_PLLN $plln_16mhz"
        printf "%s\n" "#define HSE_PLLQ $pllq_16mhz"
        printf "%s\n" "#define HSE_PLLP $pllp_16mhz"
        printf "%s\n" "#define AHB_CLK_DIV $ahb_clk_div_16mhz"
        printf "%s\n" "#define APB1_CLK_DIV $apb1_clk_div_16mhz"
        printf "%s\n" "#define APB2_CLK_DIV $apb2_clk_div_16mhz"
        printf "%s\n" "#elif (HSE_VALUE == 25000000)"
        printf "%s\n" "#define HSE_PLLM $pllm_25mhz"
        printf "%s\n" "#define HSE_PLLN $plln_25mhz"
        printf "%s\n" "#define HSE_PLLQ $pllq_25mhz"
        printf "%s\n" "#define HSE_PLLP $pllp_25mhz"
        printf "%s\n" "#define AHB_CLK_DIV $ahb_clk_div_25mhz"
        printf "%s\n" "#define APB1_CLK_DIV $apb1_clk_div_25mhz"
        printf "%s\n" "#define APB2_CLK_DIV $apb2_clk_div_25mhz"
        printf "%s\n" "#else"
        printf "%s\n" "#error Invalid clock value"
        printf "%s\n" "#endif"
    } >> "$out_header"
fi

if [[ $($yaml_parser "$yaml_file" voltage_scale) != "null" ]]
then
    vreg_scale=$($yaml_parser "$yaml_file" voltage_scale)
    printf "%s\n" "#define PWR_REGULATOR_VOLTAGE_SCALE $vreg_scale" >> "$out_header"
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

number_of_uart_interfaces=$($yaml_parser "$yaml_file" interfaces.uart)
number_of_i2c_interfaces=$($yaml_parser "$yaml_file" interfaces.i2c)

{
    printf "%s\n" "#define MAX_UART_INTERFACES  $number_of_uart_interfaces"
    printf "%s\n" "#define MAX_I2C_INTERFACES   $number_of_i2c_interfaces"
} >> "$out_header"

if [[ $($yaml_parser "$yaml_file" timers) != "null" ]]
then
    timer_period_main=$($yaml_parser "$yaml_file" timers.main.period)
    timer_channel_main=$($yaml_parser "$yaml_file" timers.main.channel)

    if [[ $timer_period_main == "null" ]]
    then
        echo "Main timer period is not optional"
        exit 1
    fi

    if [[ $timer_channel_main == "null" ]]
    then
        echo "Main timer channel is not optional"
        exit 1
    fi

    {
        printf "%s\n" "#define TIMER_PERIOD_MAIN    $timer_period_main"
        printf "%s\n" "#define TIMER_CHANNEL_MAIN   $timer_channel_main"
    } >> "$out_header"

    timer_period_pwm=$($yaml_parser "$yaml_file" timers.pwm.period)
    timer_channel_pwm=$($yaml_parser "$yaml_file" timers.pwm.channel)

    if [[ ($timer_period_pwm != "null") && ($timer_channel_pwm != "null") ]]
    then
        {
            printf "%s\n" "#define TIMER_PERIOD_PWM     $timer_period_pwm"
            printf "%s\n" "#define TIMER_CHANNEL_PWM    $timer_channel_pwm"
        } >> "$out_header"
    fi
else
    echo "Timer periods undefined"
    exit 1
fi

common_mcu_family_include="board/arch/$arch/$vendor/variants/$mcu_family/Map.h.include"
common_mcu_include="board/arch/$arch/$vendor/variants/$mcu_family/$mcu/Map.h.include"

if [[ -f $common_mcu_family_include ]]
then
    printf "%s\n" "#include \"$common_mcu_family_include\"" >> "$out_header"
fi

if [[ -f $common_mcu_include ]]
then
    printf "%s\n" "#include \"$common_mcu_include\"" >> "$out_header"
fi