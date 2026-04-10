/*

Copyright Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#pragma once

#include "deps.h"

#include "zlibs/utils/emueeprom/emueeprom.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace database
{
    class HwaTest : public Hwa
    {
        public:
        HwaTest() = default;

        bool init() override
        {
            return _emuEEPROM.init();
        }

        uint32_t size() override
        {
            return _emuEEPROM.max_address();
        }

        bool clear() override
        {
            return _emuEEPROM.format();
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
                const auto read_status = _emuEEPROM.read(address, value);

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
                return _emuEEPROM.write(address, write_value) == zlibs::utils::emueeprom::WriteStatus::Ok;

            default:
                return false;
            }
        }

        bool hasFactorySnapshot() override
        {
            return _emuEEPROM.page_status(zlibs::utils::emueeprom::Page::Factory) ==
                   zlibs::utils::emueeprom::PageStatus::Valid;
        }

        bool storeFactorySnapshot() override
        {
            _emuEEPROM.write_cache_to_flash();
            return _emuEEPROM.store_to_factory();
        }

        bool restoreFactorySnapshot() override
        {
            return _emuEEPROM.restore_from_factory();
        }

        private:
        class HwaEmuEeprom : public zlibs::utils::emueeprom::Hwa
        {
            public:
            static constexpr size_t PAGE_1_SIZE  = CONFIG_ZLIBS_UTILS_EMUEEPROM_PAGE_SIZE;
            static constexpr size_t PAGE_2_SIZE  = CONFIG_ZLIBS_UTILS_EMUEEPROM_PAGE_SIZE;
            static constexpr size_t FACTORY_SIZE = CONFIG_ZLIBS_UTILS_EMUEEPROM_PAGE_SIZE;

            HwaEmuEeprom()
            {
                erase_page(zlibs::utils::emueeprom::Page::Page1);
                erase_page(zlibs::utils::emueeprom::Page::Page2);
                erase_page(zlibs::utils::emueeprom::Page::Factory);
            }

            bool init() override
            {
                return true;
            }

            bool erase_page(zlibs::utils::emueeprom::Page page) override
            {
                auto& storage = page_storage(page);

                std::fill(storage.begin(), storage.end(), static_cast<uint8_t>(0xFF));
                return true;
            }

            bool write_32(zlibs::utils::emueeprom::Page page, uint32_t offset, uint32_t data) override
            {
                auto& storage = page_storage(page);

                if ((offset + sizeof(data)) > storage.size())
                {
                    return false;
                }

                auto current = read_32(page, offset);

                if (!current.has_value() || (data > current.value()))
                {
                    return false;
                }

                storage.at(offset + 0) = static_cast<uint8_t>((data >> 0) & 0xFF);
                storage.at(offset + 1) = static_cast<uint8_t>((data >> 8) & 0xFF);
                storage.at(offset + 2) = static_cast<uint8_t>((data >> 16) & 0xFF);
                storage.at(offset + 3) = static_cast<uint8_t>((data >> 24) & 0xFF);

                return true;
            }

            std::optional<uint32_t> read_32(zlibs::utils::emueeprom::Page page, uint32_t offset) override
            {
                const auto& storage = page_storage(page);

                if ((offset + sizeof(uint32_t)) > storage.size())
                {
                    return {};
                }

                uint32_t data = storage.at(offset + 3);
                data <<= 8;
                data |= storage.at(offset + 2);
                data <<= 8;
                data |= storage.at(offset + 1);
                data <<= 8;
                data |= storage.at(offset + 0);

                return data;
            }

            private:
            using PageStorage = std::array<uint8_t, CONFIG_ZLIBS_UTILS_EMUEEPROM_PAGE_SIZE>;

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

        HwaEmuEeprom                       _hwaEmuEeprom;
        zlibs::utils::emueeprom::EmuEeprom _emuEEPROM = zlibs::utils::emueeprom::EmuEeprom(_hwaEmuEeprom);
    };
}    // namespace database
