/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace opendeck::flash_area
{
    /**
     * @brief Flash-map area access used by OpenDeck update paths.
     *
     * This interface wraps a single flash-map area and keeps users independent
     * from the concrete flash backend. All offsets exposed here are relative to
     * the opened area, not absolute flash-device offsets.
     */
    class Hwa
    {
        public:
        /**
         * @brief Flash sector descriptor with area-relative offset.
         */
        struct Sector
        {
            uint32_t offset = 0;
            uint32_t size   = 0;
        };

        virtual ~Hwa() = default;

        /**
         * @brief Opens a flash-map area.
         *
         * @param area_id Platform flash-map area id.
         *
         * @return `true` if the area is open, otherwise `false`.
         */
        virtual bool open(uint8_t area_id) = 0;

        /**
         * @brief Returns whether an area has been opened.
         *
         * @return `true` if an area is open.
         */
        virtual bool opened() const = 0;

        /**
         * @brief Returns the opened area size in bytes.
         *
         * @return Area size in bytes, or 0 if no area is open.
         */
        virtual uint32_t size() const = 0;

        /**
         * @brief Returns the native flash write block size.
         *
         * @return Write block size in bytes, or 0 if no area is open.
         */
        virtual size_t write_block_size() const = 0;

        /**
         * @brief Reads bytes from the opened area.
         *
         * @param offset Area-relative byte offset.
         * @param data Destination buffer.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool read(uint32_t offset, std::span<uint8_t> data) const = 0;

        /**
         * @brief Writes bytes to the opened area.
         *
         * @param offset Area-relative byte offset.
         * @param data Source buffer.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool write(uint32_t offset, std::span<const uint8_t> data) const = 0;

        /**
         * @brief Erases bytes from the opened area.
         *
         * @param offset Area-relative byte offset.
         * @param size Number of bytes to erase.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool erase(uint32_t offset, uint32_t size) const = 0;

        /**
         * @brief Returns one flash sector from the opened area.
         *
         * @param index Sector index to query.
         * @return Sector descriptor when found, otherwise `std::nullopt`.
         */
        virtual std::optional<Sector> sector(size_t index) const = 0;

        /**
         * @brief Fills a caller-provided sector buffer with the opened area's sectors.
         *
         * @param sectors Destination sector buffer.
         * @param sector_count Number of sectors written.
         *
         * @return `true` if all area sectors fit and were enumerated.
         */
        virtual bool sectors(std::span<Sector> sectors, size_t& sector_count) const = 0;
    };
}    // namespace opendeck::flash_area
