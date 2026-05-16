/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace opendeck::staged_update
{
    /**
     * @brief Flash access used by the staged-update writer.
     */
    class Hwa
    {
        public:
        struct Sector
        {
            uint32_t offset = 0;
            uint32_t size   = 0;
        };

        virtual ~Hwa() = default;

        /**
         * @brief Opens the staged DFU storage and prepares flash metadata.
         */
        virtual bool init() = 0;

        /**
         * @brief Returns the total staged DFU storage size.
         */
        virtual uint32_t size() const = 0;

        /**
         * @brief Returns the flash write block size.
         */
        virtual size_t write_block_size() const = 0;

        /**
         * @brief Returns one flash sector from the staged DFU storage.
         */
        virtual std::optional<Sector> sector(size_t index) const = 0;

        /**
         * @brief Erases bytes from the staged DFU storage.
         */
        virtual bool erase(uint32_t offset, uint32_t size) = 0;

        /**
         * @brief Writes bytes to the staged DFU storage.
         */
        virtual bool write(uint32_t offset, std::span<const uint8_t> data) = 0;
    };
}    // namespace opendeck::staged_update
