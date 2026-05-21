/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/installer/shared/deps.h"
#include "common/src/flash_area/hwa/hw/hwa_hw.h"
#include "bootloader/src/indicators/instance/impl/indicators.h"
#include "bootloader/src/webusb/instance/impl/transport.h"

#include <zephyr/devicetree.h>
#include <zephyr/devicetree/partitions.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>

#include <array>
#include <span>

#define FIRMWARE_SLOT_NODE DT_NODELABEL(slot0_partition)

namespace opendeck::installer
{
    /**
     * @brief Hardware-backed installer backend that writes the firmware image to the primary slot.
     */
    class HwaHw : public Hwa
    {
        public:
        /**
         * @brief Constructs the hardware installer backend.
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
         * @brief Returns the size of one firmware slot sector.
         *
         * @param index Sector index to query.
         *
         * @return Sector size in bytes, or `0` when the sector is unavailable.
         */
        uint32_t sector_size(const size_t index) override
        {
            ensure_flash_area_ready();

            if (!_initialized || (index >= _sector_count))
            {
                return 0;
            }

            return _sectors[index].size;
        }

        /**
         * @brief Returns the native write-block size of the firmware slot.
         *
         * @return Write-block size in bytes.
         */
        size_t write_block_size() override
        {
            return WRITE_BLOCK_SIZE;
        }

        /**
         * @brief Erases one firmware slot sector and prepares it for writes.
         *
         * @param index Sector index to erase.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool erase_sector(const size_t index) override
        {
            if (!prepare_sector(index))
            {
                return false;
            }

            const auto& sector = _sectors[index];

            if (!_flash_area.erase(sector.offset, sector.size))
            {
                webusb::status("Flash erase failed");
                return false;
            }

            _buffered_sector = index;
            return true;
        }

        /**
         * @brief Writes one flash-aligned block into the selected flash sector.
         *
         * @param index Sector index being prepared.
         * @param offset Byte offset within the sector image.
         * @param data Bytes to write.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool write_sector(const size_t index, const uint32_t offset, std::span<const uint8_t> data) override
        {
            if (_buffered_sector != index)
            {
                webusb::status("Invalid firmware sector write");
                return false;
            }

            if (data.empty() ||
                ((data.size() % WRITE_BLOCK_SIZE) != 0) ||
                ((offset % WRITE_BLOCK_SIZE) != 0))
            {
                webusb::status("Invalid flash write block");
                return false;
            }

            if ((index >= _sector_count) ||
                (data.size() > _sectors[index].size) ||
                (offset > (_sectors[index].size - data.size())))
            {
                webusb::status("Flash buffer overflow");
                return false;
            }

            const auto& sector = _sectors[index];

            if (!_flash_area.write(sector.offset + offset, data))
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
         * @brief Flash write granularity reported by Zephyr for the firmware slot.
         */
        static constexpr size_t FLASH_WRITE_BLOCK_SIZE = DT_PROP(DT_MEM_FROM_PARTITION(FIRMWARE_SLOT_NODE), write_block_size);

        /**
         * @brief Native flash write granularity used for alignment checks.
         *
         * Firmware payloads are byte streams, so unlike EmuEEPROM there is no
         * larger logical-entry minimum here; the backend write size is exactly
         * the flash driver granularity.
         */
        static constexpr size_t WRITE_BLOCK_SIZE = FLASH_WRITE_BLOCK_SIZE;

        static_assert(WRITE_BLOCK_SIZE > 0, "Firmware slot write block size is invalid");

        static constexpr size_t   MAX_SECTOR_COUNT     = 1024;
        static constexpr size_t   INVALID_SECTOR_INDEX = static_cast<size_t>(-1);
        static constexpr uint32_t REBOOT_DELAY_MS      = 100;

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
         * @brief Opens the firmware slot and caches its sector layout.
         */
        void init_flash_area()
        {
            static constexpr uint8_t FIRMWARE_SLOT_AREA_ID = DT_PARTITION_ID(FIRMWARE_SLOT_NODE);

            if (!_flash_area.open(FIRMWARE_SLOT_AREA_ID))
            {
                webusb::status("Failed to open firmware slot");
                return;
            }

            _sector_count = MAX_SECTOR_COUNT;

            if (!_flash_area.sectors(_sectors, _sector_count))
            {
                webusb::status("Failed to query firmware slot sectors");
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
        bool prepare_sector(const size_t index)
        {
            ensure_flash_area_ready();

            if (!_initialized || (index >= _sector_count))
            {
                webusb::status("Invalid firmware sector index");
                return false;
            }

            return true;
        }

        /**
         * @brief Handles the delayed reboot work item.
         *
         * @param work Zephyr work item embedded in this installer backend.
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

        flash_area::HwaHw                                     _flash_area       = {};
        std::array<flash_area::Hwa::Sector, MAX_SECTOR_COUNT> _sectors          = {};
        size_t                                                _sector_count     = 0;
        size_t                                                _buffered_sector  = INVALID_SECTOR_INDEX;
        CleanupCallback                                       _cleanup_callback = nullptr;
        k_work_delayable                                      _reboot_work      = {};
        bool                                                  _initialized      = false;

        static inline HwaHw* reboot_instance = nullptr;
    };
}    // namespace opendeck::installer
