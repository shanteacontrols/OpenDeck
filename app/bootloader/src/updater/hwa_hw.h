/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "indicators.h"
#include "webusb/transport.h"

#include <zephyr/devicetree.h>
#include <zephyr/devicetree/fixed-partitions.h>
#include <zephyr/kernel.h>
#include <zephyr/devicetree/partitions.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/sys/reboot.h>

#include <array>
#include <span>

#define OPENDECK_BOOTLOADER_NODE DT_NODELABEL(opendeck_bootloader)
#define APP_SLOT_NODE            DT_PHANDLE(OPENDECK_BOOTLOADER_NODE, app_partition)

namespace opendeck::updater
{
    /**
     * @brief Hardware-backed updater backend that writes the application image to flash.
     */
    class HwaHw : public Hwa
    {
        public:
        /**
         * @brief Constructs the hardware updater backend.
         *
         * @param cleanup_callback Callback invoked from `cleanup()`.
         */
        explicit HwaHw(CleanupCallback cleanup_callback)
            : _cleanup_callback(cleanup_callback)
        {
            reboot_instance = this;
            k_work_init_delayable(&_reboot_work, reboot_handler);
        }

        /**
         * @brief Returns the size of one application flash sector.
         *
         * @param index Sector index to query.
         *
         * @return Sector size in bytes, or `0` when the sector is unavailable.
         */
        uint32_t page_size(const size_t index) override
        {
            ensure_flash_area_ready();

            if (!_initialized || (index >= _sector_count))
            {
                return 0;
            }

            return _sectors[index].fs_size;
        }

        /**
         * @brief Returns the native write-block size of the application flash.
         *
         * @return Write-block size in bytes.
         */
        size_t write_block_size() override
        {
            return WRITE_BLOCK_SIZE;
        }

        /**
         * @brief Erases one application flash sector and prepares it for writes.
         *
         * @param index Sector index to erase.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool erase_page(const size_t index) override
        {
            if (!prepare_page(index))
            {
                return false;
            }

            const auto& sector      = _sectors[index];
            const int   return_code = flash_area_erase(_flash_area, sector.fs_off, sector.fs_size);

            if (return_code != 0)
            {
                webusb::status("Flash erase failed");
                return false;
            }

            _buffered_page = index;
            return true;
        }

        /**
         * @brief Writes one native flash block into the selected flash sector.
         *
         * @param index Sector index being prepared.
         * @param offset Byte offset within the sector image.
         * @param data Bytes to write.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool write_page(const size_t index, const uint32_t offset, std::span<const uint8_t> data) override
        {
            if (_buffered_page != index)
            {
                webusb::status("Invalid app page write");
                return false;
            }

            if ((data.size() != WRITE_BLOCK_SIZE) ||
                ((offset % WRITE_BLOCK_SIZE) != 0))
            {
                webusb::status("Invalid flash write block");
                return false;
            }

            if ((index >= _sector_count) ||
                (data.size() > _sectors[index].fs_size) ||
                (offset > (_sectors[index].fs_size - data.size())))
            {
                webusb::status("Flash buffer overflow");
                return false;
            }

            const auto& sector      = _sectors[index];
            const int   return_code = flash_area_write(
                _flash_area,
                static_cast<off_t>(sector.fs_off + offset),
                data.data(),
                data.size());

            if (return_code != 0)
            {
                webusb::status("Flash write failed");
                return false;
            }

            return true;
        }

        /**
         * @brief Finalizes the update and reboots when no flash errors occurred.
         */
        void apply() override
        {
            webusb::status("Firmware update complete, rebooting");
            k_work_reschedule(&_reboot_work, K_MSEC(REBOOT_DELAY_MS));
        }

        /**
         * @brief Invokes the optional platform cleanup callback.
         */
        void cleanup() override
        {
            if (_cleanup_callback != nullptr)
            {
                _cleanup_callback();
            }
        }

        /**
         * @brief Logs the beginning of a firmware update session.
         */
        void on_firmware_update_start() override
        {
            ensure_flash_area_ready();
            indicators::start_blinking_all();
            webusb::status("Firmware update started");
        }

        private:
        /**
         * @brief Flash write granularity reported by Zephyr for the application slot.
         */
        static constexpr size_t FLASH_WRITE_BLOCK_SIZE = DT_PROP(DT_MEM_FROM_FIXED_PARTITION(APP_SLOT_NODE), write_block_size);

        /**
         * @brief Smallest block size the updater writes through this backend.
         *
         * Firmware payloads are byte streams, so unlike EmuEEPROM there is no
         * larger logical-entry minimum here; the backend write size is exactly
         * the flash driver granularity.
         */
        static constexpr size_t WRITE_BLOCK_SIZE = FLASH_WRITE_BLOCK_SIZE;

        static_assert(WRITE_BLOCK_SIZE > 0, "Application flash write block size is invalid");

        static constexpr size_t   MAX_SECTOR_COUNT   = 1024;
        static constexpr size_t   INVALID_PAGE_INDEX = static_cast<size_t>(-1);
        static constexpr uint32_t REBOOT_DELAY_MS    = 100;

        /**
         * @brief Lazily initializes flash-area metadata when first needed.
         */
        void ensure_flash_area_ready()
        {
            if (!_initialized)
            {
                init_flash_area();
            }
        }

        /**
         * @brief Opens the application flash area and caches its sector layout.
         */
        void init_flash_area()
        {
            static constexpr uint8_t APP_SLOT_AREA_ID = DT_PARTITION_ID(APP_SLOT_NODE);

            const int open_rc = flash_area_open(APP_SLOT_AREA_ID, &_flash_area);

            if (open_rc != 0)
            {
                webusb::status("Failed to open app flash area");
                return;
            }

            _sector_count = MAX_SECTOR_COUNT;

            const int sectors_rc = flash_area_get_sectors(APP_SLOT_AREA_ID, &_sector_count, _sectors.data());

            if (sectors_rc != 0)
            {
                webusb::status("Failed to query app flash sectors");
                _sector_count = 0;
                return;
            }

            _initialized = true;
        }

        /**
         * @brief Validates the target sector and marks it ready for writes.
         *
         * @param index Sector index to prepare.
         *
         * @return `true` if the sector is valid and the buffer is ready, otherwise `false`.
         */
        bool prepare_page(const size_t index)
        {
            ensure_flash_area_ready();

            if (!_initialized || (index >= _sector_count))
            {
                webusb::status("Invalid app page index");
                return false;
            }

            return true;
        }

        /**
         * @brief Handles the delayed reboot work item.
         *
         * @param work Zephyr work item embedded in this updater backend.
         */
        static void reboot_handler([[maybe_unused]] k_work* work)
        {
            if (reboot_instance == nullptr)
            {
                return;
            }

            if (reboot_instance->_cleanup_callback != nullptr)
            {
                reboot_instance->_cleanup_callback();
            }

            sys_reboot(SYS_REBOOT_COLD);
        }

        const struct flash_area*                   _flash_area       = nullptr;
        std::array<flash_sector, MAX_SECTOR_COUNT> _sectors          = {};
        size_t                                     _sector_count     = 0;
        size_t                                     _buffered_page    = INVALID_PAGE_INDEX;
        CleanupCallback                            _cleanup_callback = nullptr;
        k_work_delayable                           _reboot_work      = {};
        bool                                       _initialized      = false;

        static inline HwaHw* reboot_instance = nullptr;
    };
}    // namespace opendeck::updater
