/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/direct_update_writer/instance/impl/deps.h"
#include "common/src/dfu/flash_area/hwa/hw/hwa_hw.h"
#include "common/src/mcu/shared/deps.h"
#include "common/src/system/shared/common.h"

#include "zlibs/utils/misc/kwork_delayable.h"

#include <zephyr/devicetree.h>
#include <zephyr/devicetree/partitions.h>
#include <zephyr/kernel.h>

#include <array>
#include <span>

#define FIRMWARE_SLOT_NODE DT_NODELABEL(slot0_partition)

namespace opendeck::bootloader::dfu::direct_update_writer
{
    /**
     * @brief Hardware-backed direct-update writer backend that writes the firmware image to the primary slot.
     */
    class HwaHw : public Hwa
    {
        public:
        /**
         * @brief Constructs the hardware direct-update writer backend.
         *
         * @param mcu Shared MCU services.
         */
        explicit HwaHw(opendeck::common::mcu::Hwa& mcu)
            : _mcu(mcu)
            , _reboot_work([this]()
                           {
                               reboot();
                           })
        {
        }

        /**
         * @brief Returns the writable firmware slot size.
         *
         * @return Firmware slot size in bytes.
         */
        uint32_t size() override
        {
            return FIRMWARE_SLOT_SIZE;
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
                return false;
            }

            if (data.empty() ||
                ((data.size() % WRITE_BLOCK_SIZE) != 0) ||
                ((offset % WRITE_BLOCK_SIZE) != 0))
            {
                return false;
            }

            if ((index >= _sector_count) ||
                (data.size() > _sectors[index].size) ||
                (offset > (_sectors[index].size - data.size())))
            {
                return false;
            }

            const auto& sector = _sectors[index];

            if (!_flash_area.write(sector.offset + offset, data))
            {
                return false;
            }

            return true;
        }

        /**
         * @brief Finalizes the update and reboots when no flash errors occurred.
         */
        void apply() override
        {
            _reboot_work.reschedule(opendeck::common::system::REBOOT_DELAY_MS);
        }

        private:
        /**
         * @brief Flash write granularity reported by Zephyr for the firmware slot.
         */
        static constexpr size_t FLASH_WRITE_BLOCK_SIZE = DT_PROP(DT_MEM_FROM_PARTITION(FIRMWARE_SLOT_NODE), write_block_size);

        /**
         * @brief Writable firmware slot size from the partition table.
         */
        static constexpr uint32_t FIRMWARE_SLOT_SIZE = DT_REG_SIZE(FIRMWARE_SLOT_NODE);

        /**
         * @brief Native flash write granularity used for alignment checks.
         *
         * Firmware payloads are byte streams, so unlike EmuEEPROM there is no
         * larger logical-entry minimum here; the backend write size is exactly
         * the flash driver granularity.
         */
        static constexpr size_t WRITE_BLOCK_SIZE = FLASH_WRITE_BLOCK_SIZE;

        static_assert(WRITE_BLOCK_SIZE > 0, "Firmware slot write block size is invalid");

        static constexpr size_t MAX_SECTOR_COUNT     = 1024;
        static constexpr size_t INVALID_SECTOR_INDEX = static_cast<size_t>(-1);

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
                return;
            }

            _sector_count = MAX_SECTOR_COUNT;

            if (!_flash_area.sectors(_sectors, _sector_count))
            {
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
                return false;
            }

            return true;
        }

        /**
         * @brief Performs the delayed reboot after a successful firmware update.
         */
        void reboot()
        {
            _mcu.reboot(opendeck::common::mcu::BootTarget::Application);
        }

        opendeck::common::dfu::flash_area::HwaHw                                     _flash_area = {};
        opendeck::common::mcu::Hwa&                                                  _mcu;
        std::array<opendeck::common::dfu::flash_area::Hwa::Sector, MAX_SECTOR_COUNT> _sectors         = {};
        size_t                                                                       _sector_count    = 0;
        size_t                                                                       _buffered_sector = INVALID_SECTOR_INDEX;
        zlibs::utils::misc::KworkDelayable                                           _reboot_work;
        bool                                                                         _initialized = false;
    };
}    // namespace opendeck::bootloader::dfu::direct_update_writer
