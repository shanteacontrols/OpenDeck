/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

#include "zlibs/utils/emueeprom/emueeprom.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace database
{
    /**
     * @brief Test database storage backend.
     */
    class HwaTest : public Hwa
    {
        public:
        HwaTest() = default;

        bool init() override
        {
            return _emueeprom.init();
        }

        uint32_t size() override
        {
            return _emueeprom.max_address();
        }

        bool clear() override
        {
            return _emueeprom.format();
        }

        std::optional<uint32_t> read(uint32_t address, zlibs::utils::lessdb::SectionParameterType type) override
        {
            uint16_t value = 0;

            switch (type)
            {
            case zlibs::utils::lessdb::SectionParameterType::Bit:
            case zlibs::utils::lessdb::SectionParameterType::Byte:
            case zlibs::utils::lessdb::SectionParameterType::HalfByte:
            case zlibs::utils::lessdb::SectionParameterType::Word:
            {
                const auto read_status = _emueeprom.read(address, value);

                if (read_status == zlibs::utils::emueeprom::ReadStatus::Ok)
                {
                    return value;
                }

                if (read_status == zlibs::utils::emueeprom::ReadStatus::NoVariable)
                {
                    return 0;
                }

                return {};
            }

            default:
                return {};
            }
        }

        bool write(uint32_t address, uint32_t value, zlibs::utils::lessdb::SectionParameterType type) override
        {
            uint16_t write_value = value;

            switch (type)
            {
            case zlibs::utils::lessdb::SectionParameterType::Bit:
            case zlibs::utils::lessdb::SectionParameterType::Byte:
            case zlibs::utils::lessdb::SectionParameterType::HalfByte:
            case zlibs::utils::lessdb::SectionParameterType::Word:
                return _emueeprom.write(zlibs::utils::emueeprom::make_entry(static_cast<uint16_t>(address), write_value)) == zlibs::utils::emueeprom::WriteStatus::Ok;

            default:
                return false;
            }
        }

        bool has_factory_snapshot() override
        {
            return _emueeprom.page_status(zlibs::utils::emueeprom::Page::Factory) ==
                   zlibs::utils::emueeprom::PageStatus::Valid;
        }

        bool store_factory_snapshot() override
        {
            return _emueeprom.store_to_factory();
        }

        bool restore_factory_snapshot() override
        {
            return _emueeprom.restore_from_factory();
        }

        private:
        /**
         * @brief In-memory emulated EEPROM hardware adapter used by database tests.
         */
        class HwaEmuEeprom : public zlibs::utils::emueeprom::Hwa
        {
            public:
            /**
             * @brief Backend write granularity used by the test flash.
             */
            static constexpr size_t WRITE_BLOCK_SIZE = sizeof(uint32_t);

            /**
             * @brief Size of the first EEPROM emulation page used by the test backend.
             */
            static constexpr size_t PAGE_1_SIZE = CONFIG_ZLIBS_UTILS_EMUEEPROM_PAGE_SIZE;

            /**
             * @brief Size of the second EEPROM emulation page used by the test backend.
             */
            static constexpr size_t PAGE_2_SIZE = CONFIG_ZLIBS_UTILS_EMUEEPROM_PAGE_SIZE;

            /**
             * @brief Indicates that the test backend provides a factory snapshot page.
             */
            static constexpr bool HAS_FACTORY_PAGE = true;

            /**
             * @brief Size of the factory snapshot page used by the test backend.
             */
            static constexpr size_t FACTORY_SIZE = CONFIG_ZLIBS_UTILS_EMUEEPROM_PAGE_SIZE;

            /**
             * @brief Constructs the in-memory emulated EEPROM backend and initializes all pages to the erased state.
             */
            HwaEmuEeprom()
            {
                erase_page(zlibs::utils::emueeprom::Page::Page1);
                erase_page(zlibs::utils::emueeprom::Page::Page2);
                erase_page(zlibs::utils::emueeprom::Page::Factory);
            }

            /**
             * @brief Initializes the in-memory emulated EEPROM backend.
             *
             * @return Always `true`.
             */
            bool init() override
            {
                return true;
            }

            /**
             * @brief Erases one emulated EEPROM page by filling it with `0xFF`.
             *
             * @param page Emulated EEPROM page to erase.
             *
             * @return Always `true`.
             */
            bool erase_page(zlibs::utils::emueeprom::Page page) override
            {
                auto& storage = page_storage(page);

                std::fill(storage.begin(), storage.end(), static_cast<uint8_t>(0xFF));
                return true;
            }

            /**
             * @brief Writes one backend write block to the selected emulated EEPROM page.
             *
             * @param page Emulated EEPROM page to update.
             * @param offset Byte offset within `page`.
             * @param data Backend write-block bytes to store.
             *
             * @return `true` if the write fits within the page and only clears programmed bits, otherwise `false`.
             */
            bool write(zlibs::utils::emueeprom::Page page, uint32_t offset, std::span<const uint8_t> data) override
            {
                auto& storage = page_storage(page);

                if ((data.size() != WRITE_BLOCK_SIZE) ||
                    ((offset % WRITE_BLOCK_SIZE) != 0) ||
                    ((offset + data.size()) > storage.size()))
                {
                    return false;
                }

                for (size_t i = 0; i < data.size(); i++)
                {
                    if ((data[i] | storage.at(offset + i)) != storage.at(offset + i))
                    {
                        return false;
                    }
                }

                std::copy(data.begin(), data.end(), storage.begin() + offset);

                return true;
            }

            /**
             * @brief Reads one backend write block from the selected emulated EEPROM page.
             *
             * @param page Emulated EEPROM page to read.
             * @param offset Byte offset within `page`.
             * @param data Output buffer populated with backend write-block bytes.
             *
             * @return `true` if the read fits within the page, otherwise `false`.
             */
            bool read(zlibs::utils::emueeprom::Page page, uint32_t offset, std::span<uint8_t> data) override
            {
                const auto& storage = page_storage(page);

                if ((data.size() != WRITE_BLOCK_SIZE) ||
                    ((offset % WRITE_BLOCK_SIZE) != 0) ||
                    ((offset + data.size()) > storage.size()))
                {
                    return false;
                }

                std::copy(storage.begin() + offset, storage.begin() + offset + data.size(), data.begin());
                return true;
            }

            private:
            /**
             * @brief Byte-addressable storage type used for one in-memory emulated EEPROM page.
             */
            using PageStorage = std::array<uint8_t, CONFIG_ZLIBS_UTILS_EMUEEPROM_PAGE_SIZE>;

            /**
             * @brief Returns mutable storage for the selected emulated EEPROM page.
             *
             * @param page Emulated EEPROM page to resolve.
             *
             * @return Mutable storage backing `page`.
             */
            PageStorage& page_storage(zlibs::utils::emueeprom::Page page)
            {
                switch (page)
                {
                case zlibs::utils::emueeprom::Page::Page1:
                    return _page1;

                case zlibs::utils::emueeprom::Page::Page2:
                    return _page2;

                case zlibs::utils::emueeprom::Page::Factory:
                    return _factory;
                }

                return _page1;
            }

            /**
             * @brief Returns read-only storage for the selected emulated EEPROM page.
             *
             * @param page Emulated EEPROM page to resolve.
             *
             * @return Read-only storage backing `page`.
             */
            const PageStorage& page_storage(zlibs::utils::emueeprom::Page page) const
            {
                switch (page)
                {
                case zlibs::utils::emueeprom::Page::Page1:
                    return _page1;

                case zlibs::utils::emueeprom::Page::Page2:
                    return _page2;

                case zlibs::utils::emueeprom::Page::Factory:
                    return _factory;
                }

                return _page1;
            }

            PageStorage _page1   = {};
            PageStorage _page2   = {};
            PageStorage _factory = {};
        };

        HwaEmuEeprom                       _hwa_emueeprom;
        zlibs::utils::emueeprom::EmuEeprom _emueeprom = zlibs::utils::emueeprom::EmuEeprom(_hwa_emueeprom);
    };
}    // namespace database
