/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/flash_area/shared/deps.h"

#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>

namespace opendeck::flash_area
{
    /**
     * @brief Zephyr flash-map implementation of flash-area access.
     */
    class HwaHw : public Hwa
    {
        public:
        /**
         * @brief Opens a Zephyr flash-map area.
         *
         * @param area_id Zephyr flash-map area id.
         *
         * @return `true` if the area is open, otherwise `false`.
         */
        bool open(const uint8_t area_id) override
        {
            if (_area != nullptr)
            {
                return true;
            }

            return flash_area_open(area_id, &_area) == 0;
        }

        /**
         * @brief Returns whether a flash-map area has been opened.
         *
         * @return `true` if `open()` succeeded.
         */
        bool opened() const override
        {
            return _area != nullptr;
        }

        /**
         * @brief Returns the opened flash-map area size.
         *
         * @return Area size in bytes, or 0 if no area is open.
         */
        uint32_t size() const override
        {
            if (_area == nullptr)
            {
                return 0;
            }

            return _area->fa_size;
        }

        /**
         * @brief Returns the native write block size for the opened flash device.
         *
         * @return Write block size in bytes, or 0 if no area is open.
         */
        size_t write_block_size() const override
        {
            if (_area == nullptr)
            {
                return 0;
            }

            return flash_get_write_block_size(_area->fa_dev);
        }

        /**
         * @brief Reads bytes from the opened flash-map area.
         *
         * @param offset Byte offset relative to the area.
         * @param data Destination buffer.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool read(const uint32_t offset, std::span<uint8_t> data) const override
        {
            return (_area != nullptr) &&
                   range_valid(offset, data.size()) &&
                   (flash_area_read(_area, static_cast<off_t>(offset), data.data(), data.size()) == 0);
        }

        /**
         * @brief Writes bytes to the opened flash-map area.
         *
         * @param offset Byte offset relative to the area.
         * @param data Source buffer.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool write(const uint32_t offset, std::span<const uint8_t> data) const override
        {
            return (_area != nullptr) &&
                   range_valid(offset, data.size()) &&
                   (flash_area_write(_area, static_cast<off_t>(offset), data.data(), data.size()) == 0);
        }

        /**
         * @brief Erases bytes from the opened flash-map area.
         *
         * @param offset Byte offset relative to the area.
         * @param size Number of bytes to erase.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool erase(const uint32_t offset, const uint32_t size) const override
        {
            return (_area != nullptr) &&
                   range_valid(offset, size) &&
                   (flash_area_erase(_area, static_cast<off_t>(offset), size) == 0);
        }

        /**
         * @brief Returns the first sector in the opened flash-map area.
         *
         * @param sector Destination sector descriptor with area-relative offset.
         *
         * @return `true` if the sector was found, otherwise `false`.
         */
        bool first_sector(Sector& sector) const override
        {
            if (_area == nullptr)
            {
                return false;
            }

            flash_pages_info page_info = {};

            if (flash_get_page_info_by_offs(_area->fa_dev, _area->fa_off, &page_info) != 0)
            {
                return false;
            }

            sector = {
                .offset = static_cast<uint32_t>(page_info.start_offset - _area->fa_off),
                .size   = page_info.size,
            };

            return true;
        }

        /**
         * @brief Enumerates sectors in the opened flash-map area.
         *
         * @param sectors Destination sector buffer.
         * @param sector_count In/out sector count. On input this is the buffer
         * capacity; on success this becomes the number of sectors written.
         *
         * @return `true` if all area sectors fit and were enumerated.
         */
        bool sectors(std::span<Sector> sectors, size_t& sector_count) const override
        {
            if ((_area == nullptr) || sectors.empty())
            {
                return false;
            }

            const uint32_t area_end = _area->fa_off + _area->fa_size;
            uint32_t       offset   = _area->fa_off;
            size_t         count    = 0;

            while ((offset < area_end) && (count < sectors.size()))
            {
                flash_pages_info page_info = {};

                if (flash_get_page_info_by_offs(_area->fa_dev, static_cast<off_t>(offset), &page_info) != 0)
                {
                    return false;
                }

                sectors[count++] = {
                    .offset = static_cast<uint32_t>(page_info.start_offset - _area->fa_off),
                    .size   = page_info.size,
                };

                offset = page_info.start_offset + page_info.size;
            }

            sector_count = count;

            return offset >= area_end;
        }

        private:
        const struct ::flash_area* _area = nullptr;

        /**
         * @brief Checks whether an area-relative byte range is valid.
         *
         * @param offset Byte offset relative to the area.
         * @param size Number of bytes in the range.
         *
         * @return `true` if the range fits inside the opened area.
         */
        bool range_valid(const uint32_t offset, const size_t size) const
        {
            const uint32_t area_size = this->size();

            return (offset <= area_size) &&
                   (size <= (area_size - offset));
        }
    };
}    // namespace opendeck::flash_area
