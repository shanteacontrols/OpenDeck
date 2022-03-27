/* Linker script for Nordic Semiconductor nRF devices
 *
 * Version: Sourcery G++ 4.5-1
 * Support: https://support.codesourcery.com/GNUToolchain/
 *
 * Copyright (c) 2007, 2008, 2009, 2010 CodeSourcery, Inc.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions.  No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */
 
SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = FLASH_START, LENGTH = 0x100000
  RAM (rwx) : ORIGIN = 0x20004000, LENGTH = 0x3C000
}

OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")

/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions FLASH and RAM.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 *
 * It defines following symbols, which code can use without definition:
 *   __exidx_start
 *   __exidx_end
 *   __etext
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapBase
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 */
ENTRY(Reset_Handler)

SECTIONS
{
    .text :
    {
        KEEP(*(.isr_vector))
        *(.fwMetadata)
        KEEP(*(.fwMetadata))
        *(.text*)

        KEEP(*(.init))
        KEEP(*(.fini))

        /* .ctors */
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)

        /* .dtors */
        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)

        *(.rodata*)

        KEEP(*(.eh_frame*))
    } > FLASH

    .mem_section_dummy_rom :
    {
    }

    .sdh_ble_observers :
    {
      PROVIDE(__start_sdh_ble_observers = .);
      KEEP(*(SORT(.sdh_ble_observers*)))
      PROVIDE(__stop_sdh_ble_observers = .);
    } > FLASH

    .sdh_soc_observers :
    {
      PROVIDE(__start_sdh_soc_observers = .);
      KEEP(*(SORT(.sdh_soc_observers*)))
      PROVIDE(__stop_sdh_soc_observers = .);
    } > FLASH

    .sdh_req_observers :
    {
      PROVIDE(__start_sdh_req_observers = .);
      KEEP(*(SORT(.sdh_req_observers*)))
      PROVIDE(__stop_sdh_req_observers = .);
    } > FLASH

    .sdh_state_observers :
    {
      PROVIDE(__start_sdh_state_observers = .);
      KEEP(*(SORT(.sdh_state_observers*)))
      PROVIDE(__stop_sdh_state_observers = .);
    } > FLASH

    .sdh_stack_observers :
    {
      PROVIDE(__start_sdh_stack_observers = .);
      KEEP(*(SORT(.sdh_stack_observers*)))
      PROVIDE(__stop_sdh_stack_observers = .);
    } > FLASH

    .nrf_queue :
    {
      PROVIDE(__start_nrf_queue = .);
      KEEP(*(.nrf_queue))
      PROVIDE(__stop_nrf_queue = .);
    } > FLASH

    .nrf_balloc :
    {
      PROVIDE(__start_nrf_balloc = .);
      KEEP(*(.nrf_balloc))
      PROVIDE(__stop_nrf_balloc = .);
    } > FLASH

    .cli_command :
    {
      PROVIDE(__start_cli_command = .);
      KEEP(*(.cli_command))
      PROVIDE(__stop_cli_command = .);
    } > FLASH

    .crypto_data :
    {
      PROVIDE(__start_crypto_data = .);
      KEEP(*(SORT(.crypto_data*)))
      PROVIDE(__stop_crypto_data = .);
    } > FLASH

    .pwr_mgmt_data :
    {
      PROVIDE(__start_pwr_mgmt_data = .);
      KEEP(*(SORT(.pwr_mgmt_data*)))
      PROVIDE(__stop_pwr_mgmt_data = .);
    } > FLASH

    .log_const_data :
    {
      PROVIDE(__start_log_const_data = .);
      KEEP(*(SORT(.log_const_data*)))
      PROVIDE(__stop_log_const_data = .);
    } > FLASH

    .log_backends :
    {
      PROVIDE(__start_log_backends = .);
      KEEP(*(SORT(.log_backends*)))
      PROVIDE(__stop_log_backends = .);
    } > FLASH

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > FLASH
    __exidx_end = .;

    __etext = .;

    .noinit (NOLOAD) :
    {
        *(.noinit)
        KEEP (*(.noinit))
        . = ALIGN(4);
    } > RAM

    .data : AT (__etext)
    {
        __data_start__ = .;
        *(vtable)
        *(.data*)

        . = ALIGN(4);
        /* preinit data */
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        . = ALIGN(4);
        /* init data */
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);


        . = ALIGN(4);
        /* finit data */
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);

        KEEP(*(.jcr*))
        . = ALIGN(4);
        /* All data end */
        __data_end__ = .;

    } > RAM

    .mem_section_dummy_ram :
    {
    }

    .cli_sorted_cmd_ptrs :
    {
      PROVIDE(__start_cli_sorted_cmd_ptrs = .);
      KEEP(*(.cli_sorted_cmd_ptrs))
      PROVIDE(__stop_cli_sorted_cmd_ptrs = .);
    } > RAM

    .fs_data :
    {
      PROVIDE(__start_fs_data = .);
      KEEP(*(.fs_data))
      PROVIDE(__stop_fs_data = .);
    } > RAM

    .log_dynamic_data :
    {
      PROVIDE(__start_log_dynamic_data = .);
      KEEP(*(SORT(.log_dynamic_data*)))
      PROVIDE(__stop_log_dynamic_data = .);
    } > RAM

    .log_filter_data :
    {
      PROVIDE(__start_log_filter_data = .);
      KEEP(*(SORT(.log_filter_data*)))
      PROVIDE(__stop_log_filter_data = .);
    } > RAM

    .bss :
    {
        . = ALIGN(4);
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > RAM
    
    .heap (COPY):
    {
        __HeapBase = .;
        __end__ = .;
        PROVIDE(end = .);
        KEEP(*(.heap*))
        __HeapLimit = .;
    } > RAM

    /* .stack_dummy section doesn't contains any symbols. It is only
     * used for linker to calculate size of stack sections, and assign
     * values to stack symbols later */
    .stack_dummy (COPY):
    {
        KEEP(*(.stack*))
    } > RAM

    /* Set stack top to end of RAM, and stack limit move down by
     * size of stack_dummy section */
    __StackTop = ORIGIN(RAM) + LENGTH(RAM);
    __StackLimit = __StackTop - SIZEOF(.stack_dummy);
    PROVIDE(__stack = __StackTop);

    /* Check if data + heap + stack exceeds RAM limit */
    ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")
    
    /* Check if text sections + data exceeds FLASH limit */
    DataInitFlashUsed = __bss_start__ - __data_start__;
    CodeFlashUsed = __etext - ORIGIN(FLASH);
    TotalFlashUsed = CodeFlashUsed + DataInitFlashUsed;
    ASSERT(TotalFlashUsed <= LENGTH(FLASH), "region FLASH overflowed with .data and user data")
}