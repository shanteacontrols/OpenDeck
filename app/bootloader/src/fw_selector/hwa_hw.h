/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/fw_selector/fw_selector.h"
#include "common/src/retained/retained.h"

#include <zephyr/devicetree.h>
#include <zephyr/devicetree/fixed-partitions.h>
#include <zephyr/devicetree/partitions.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/storage/flash_map.h>

#include <cmsis_core.h>
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>

#define OPENDECK_BOOTLOADER_NODE DT_NODELABEL(opendeck_bootloader)
#define BOOT_SLOT_NODE           DT_PHANDLE(OPENDECK_BOOTLOADER_NODE, boot_partition)
#define APP_SLOT_NODE            DT_PHANDLE(OPENDECK_BOOTLOADER_NODE, app_partition)
#define BOOT_SLOT_START          DT_PARTITION_ADDR(BOOT_SLOT_NODE)
#define APP_SLOT_START           DT_PARTITION_ADDR(APP_SLOT_NODE)
#define APP_SLOT_SIZE            DT_REG_SIZE(APP_SLOT_NODE)
#define APP_SLOT_END             (APP_SLOT_START + APP_SLOT_SIZE)
#define APP_VECTOR_TABLE_START   (APP_SLOT_START + CONFIG_ROM_START_OFFSET)
#define OPENDECK_SRAM_START      DT_REG_ADDR(DT_CHOSEN(zephyr_sram))
#define OPENDECK_SRAM_SIZE       DT_REG_SIZE(DT_CHOSEN(zephyr_sram))
#define OPENDECK_SRAM_END        (OPENDECK_SRAM_START + OPENDECK_SRAM_SIZE)

#if !DT_NODE_EXISTS(OPENDECK_BOOTLOADER_NODE)
#error "Node label 'opendeck_bootloader' is required for bootloader image."
#endif

#if !DT_NODE_HAS_PROP(OPENDECK_BOOTLOADER_NODE, app_partition)
#error "opendeck-bootloader must define app-partition."
#endif

#if !DT_NODE_EXISTS(APP_SLOT_NODE)
#error "opendeck-bootloader app-partition phandle must reference a valid fixed partition."
#endif

#if !DT_NODE_HAS_PROP(OPENDECK_BOOTLOADER_NODE, boot_partition)
#error "opendeck-bootloader must define boot-partition."
#endif

namespace opendeck::fw_selector
{
    /**
     * @brief Hardware-backed firmware-selection backend.
     */
    class HwaHw : public Hwa
    {
        public:
        /**
         * @brief Delay applied before jumping to the application image.
         */
        static constexpr uint32_t APP_STARTUP_DELAY_MS = 20;

        /**
         * @brief Returns whether the optional bootloader switch is currently asserted.
         *
         * @return `true` when the hardware bootloader trigger is active, otherwise `false`.
         */
        bool is_hw_trigger_active() override
        {
#if !DT_NODE_HAS_PROP(OPENDECK_BOOTLOADER_NODE, button_gpios)
            return false;
#else
            if (!gpio_is_ready_dt(&_bootloader_button))
            {
                return false;
            }

            auto ret = gpio_pin_configure_dt(&_bootloader_button, GPIO_INPUT);

            if (ret != 0)
            {
                return false;
            }

            ret = gpio_pin_get_dt(&_bootloader_button);

            if (ret < 0)
            {
                return false;
            }

            return ret > 0;
#endif
        }

        /**
         * @brief Returns whether software requested a boot into the bootloader.
         *
         * @return `true` when the retained boot mode targets the bootloader, otherwise `false`.
         */
        bool is_sw_trigger_active() override
        {
            const bool bootloader_requested =
                retained::data.boot_mode.data() == static_cast<uint32_t>(FwType::Bootloader);

            retained::data.boot_mode.set(static_cast<uint32_t>(FwType::Application));

            return bootloader_requested;
        }

        bool read_app(const uint32_t offset, std::span<uint8_t> data) override
        {
            const struct flash_area* app_flash_area = nullptr;

            if (flash_area_open(APP_SLOT_AREA_ID, &app_flash_area) != 0)
            {
                return false;
            }

            const bool read = flash_area_read(app_flash_area, static_cast<off_t>(offset), data.data(), data.size()) == 0;
            flash_area_close(app_flash_area);

            return read;
        }

        fw_selector::AppInfo app_info() const override
        {
            return {
                .app = {
                    .start = APP_SLOT_START,
                    .end   = APP_SLOT_END,
                },
                .sram = {
                    .start = OPENDECK_SRAM_START,
                    .end   = OPENDECK_SRAM_END,
                },
                .vector_table_offset = CONFIG_ROM_START_OFFSET,
            };
        }

        /**
         * @brief Transfers control to the selected firmware image when supported.
         *
         * @param type Firmware image to load.
         */
        void load(FwType type) override
        {
            switch (type)
            {
            case FwType::Application:
            {
                if (read_app_vector_table())
                {
                    jump(_app_vector_table.stack_pointer, _app_vector_table.reset_vector);
                }
            }
            break;

            case FwType::Bootloader:
            default:
                break;
            }
        }

        private:
        /**
         * @brief Captures the application vector-table entry points used for validation and jumping.
         */
        struct AppVectorTable
        {
            uint32_t stack_pointer = 0;
            uint32_t reset_vector  = 0;
        };

        static constexpr uint8_t APP_SLOT_AREA_ID = DT_PARTITION_ID(APP_SLOT_NODE);

        /**
         * @brief Reads the application vector table used for jumping.
         *
         * @return `true` if the vector table was read, otherwise `false`.
         */
        bool read_app_vector_table()
        {
            const struct flash_area* app_flash_area = nullptr;
            std::array<uint32_t, 2>  vector_table   = {};

            if (flash_area_open(APP_SLOT_AREA_ID, &app_flash_area) != 0)
            {
                return false;
            }

            // Read the vector table through the flash-map API instead of
            // dereferencing a computed memory address. Some targets, such as
            // i.MX RT XIP images, place boot headers ahead of the actual
            // Cortex-M vector table, so read from the configured ROM start
            // offset within the application slot rather than assuming the
            // first bytes of the partition are SP/Reset.
            const bool read = flash_area_read(app_flash_area,
                                              CONFIG_ROM_START_OFFSET,
                                              vector_table.data(),
                                              sizeof(vector_table)) == 0;

            if (read)
            {
                _app_vector_table.stack_pointer = vector_table[0];
                _app_vector_table.reset_vector  = vector_table[1];
            }

            flash_area_close(app_flash_area);

            return read;
        }

        /**
         * @brief Transfers execution to the application image using a reset-like CPU handoff.
         *
         * This is a chain-load, not a hardware reset. The bootloader has already
         * executed Zephyr startup, enabled interrupts, configured SysTick and, on
         * some targets, enabled MPU and configurable fault handlers. Entering the
         * application with that state still active can make the app fault before
         * its own reset path has a chance to run.
         *
         * The sequence below therefore tries to leave the core in a state that is
         * as close as practical to power-on reset:
         * - stop periodic exception sources inherited from the bootloader
         * - clear pending IRQ and fault state
         * - drop bootloader-specific MPU/fault configuration
         * - restore privileged thread mode on MSP
         * - repoint VTOR at the application's vector table
         * - clear CPU interrupt masks before branching to the app reset handler
         *
         * @param stack_pointer Initial application stack pointer read from the
         *                      first word of the app vector table.
         * @param reset_vector Application reset handler address read from the
         *                     second word of the app vector table.
         */
        static void jump(uint32_t stack_pointer, uint32_t reset_vector)
        {
            using AppResetHandler = void (*)();

            auto                          reset_handler = std::bit_cast<AppResetHandler>(static_cast<uintptr_t>(reset_vector));
            [[maybe_unused]] unsigned int key;
            k_msleep(APP_STARTUP_DELAY_MS);

            // Stop Zephyr bootloader execution from scheduling anything else
            // while we dismantle its runtime state. We intentionally do not
            // call irq_unlock(key) later; interrupt masks are cleared manually
            // once the NVIC and system exception state is sanitized.
            key = irq_lock();
            __disable_irq();

            // SysTick continues running across a plain branch. Disable and
            // clear it so the application does not inherit a pending tick
            // interrupt that belongs to the bootloader's scheduling context.
            SysTick->CTRL = 0U;
            SysTick->LOAD = 0U;
            SysTick->VAL  = 0U;

            // Clear pending system exceptions and any sticky fault status
            // latched while the bootloader was active. All Cortex-M variants
            // have ICSR, but the more detailed fault status registers only
            // exist on ARMv7-M / ARMv8-M Mainline cores.
            SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk | SCB_ICSR_PENDSVCLR_Msk;    // ICSR  - Interrupt Control and State Register

#if defined(CONFIG_ARMV7_M_ARMV8_M_MAINLINE)
            SCB->CFSR = SCB->CFSR;    // CFSR  - Configurable Fault Status Register
            SCB->HFSR = SCB->HFSR;    // HFSR  - HardFault Status Register
            SCB->DFSR = SCB->DFSR;    // DFSR  - Debug Fault Status Register
#endif

            // NVIC ICER/ICPR are 32 IRQs per register, rounded up
            constexpr size_t   NVIC_REG_COUNT    = (CONFIG_NUM_IRQS + 31) / 32;
            constexpr uint32_t ALL_NVIC_IRQ_BITS = 0xFFFFFFFFU;

            // Disable every external interrupt source and drop any pending
            // state so enabling interrupts for the application cannot service
            // a stale bootloader IRQ.
            for (size_t i = 0; i < NVIC_REG_COUNT; i++)
            {
                NVIC->ICER[i] = ALL_NVIC_IRQ_BITS;    // ICER - Interrupt Clear-Enable Register
                NVIC->ICPR[i] = ALL_NVIC_IRQ_BITS;    // ICPR - Interrupt Clear-Pending Register
            }

            // Zephyr enables the MPU (Memory Protection Unit) and fault
            // handlers in the bootloader. The app expects to start from reset
            // state and will reconfigure them during its own startup.
#if defined(__MPU_PRESENT) && (__MPU_PRESENT == 1U)
            MPU->CTRL = 0U;
            __DSB();    // DSB - Data Synchronization Barrier: wait until the MPU disable write has taken effect
            __ISB();    // ISB - Instruction Synchronization Barrier: flush prefetched instructions so execution uses the updated MPU state
#endif

            // Disable configurable fault handlers that may have been enabled by
            // the bootloader. These handler-enable bits exist only on
            // ARMv7-M / ARMv8-M Mainline cores; ARMv6-M / v8-M Baseline do
            // not expose them.
#if defined(CONFIG_ARMV7_M_ARMV8_M_MAINLINE)
            SCB->SHCSR &=                        // SHCSR - System Handler Control and State Register
                ~(SCB_SHCSR_MEMFAULTENA_Msk |    // MEMFAULTENA - MemManage fault enable
                  SCB_SHCSR_BUSFAULTENA_Msk |    // BUSFAULTENA - BusFault enable
                  SCB_SHCSR_USGFAULTENA_Msk);    // USGFAULTENA - UsageFault enable
#endif

            // Restore the execution model the Cortex-M reset handler expects:
            // privileged thread mode, MSP active, PSP unused, and VTOR
            // pointing at the application's vector table base.
            __set_CONTROL(0U);
            __set_PSP(0U);
            __set_MSP(stack_pointer);
            SCB->VTOR = APP_VECTOR_TABLE_START;

            // Ensure the core observes the updated stack/vector/MPU state
            // before executing the application's first instruction.

            // DSB - Data Synchronization Barrier: complete outstanding register writes before continuing
            __DSB();
            // ISB - Instruction Synchronization Barrier: refetch instructions using the new CONTROL/stack/VTOR state
            __ISB();

            // Zephyr's Cortex-M reset path expects to start with CPU interrupt
            // masks cleared. This is safe now because all pending external and
            // system interrupt state from the bootloader has already been
            // disabled/cleared above.

            // BASEPRI / FAULTMASK are only present on Mainline Cortex-M
            // variants. Baseline parts such as Cortex-M0+ only need PRIMASK
            // cleared via __enable_irq().
#if defined(CONFIG_CPU_CORTEX_M_HAS_BASEPRI)
            __set_BASEPRI(0U);
#endif

#if defined(CONFIG_ARMV7_M_ARMV8_M_MAINLINE)
            __set_FAULTMASK(0U);
#endif
            __enable_irq();

            // Branch directly to the application's Reset_Handler.
            reset_handler();
        }

#if DT_NODE_HAS_PROP(OPENDECK_BOOTLOADER_NODE, button_gpios)
        const gpio_dt_spec _bootloader_button = GPIO_DT_SPEC_GET(OPENDECK_BOOTLOADER_NODE, button_gpios);
#endif
        AppVectorTable _app_vector_table = {};
    };
}    // namespace opendeck::fw_selector
