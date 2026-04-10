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
#include "application/messaging/messaging.h"
#include "application/io/indicators/indicators.h"

#include "zlibs/utils/emueeprom/emueeprom.h"

#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/kernel.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/sys/reboot.h>

#include <array>
#include <cstddef>
#include <cstring>
#include <cstdint>
#include <optional>

namespace database
{
    class HwaHw : public Hwa
    {
        public:
        HwaHw()
        {
            messaging::subscribe<messaging::SystemSignal>(
                [this](const messaging::SystemSignal& signal)
                {
                    switch (signal.systemMessage)
                    {
                    case messaging::systemMessage_t::RESTORE_START:
                    {
                        _writeToCache = true;
                    }
                    break;

                    case messaging::systemMessage_t::RESTORE_END:
                    {
                        _emuEEPROM.write_cache_to_flash();
                        sys_reboot(SYS_REBOOT_COLD);
                    }
                    break;

                    default:
                        break;
                    }
                });
        }

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
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_MIDI_INDICATORS
            // It's possible that LED indicators are still on since
            // this command is most likely given via USB.
            // Wait until all indicators are turned off
            k_msleep(io::indicators::Indicators::TRAFFIC_INDICATOR_TIMEOUT_MS);
#endif

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
                return _emuEEPROM.write(address, write_value, _writeToCache) == zlibs::utils::emueeprom::WriteStatus::Ok;

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
        struct Partition
        {
            const struct device* dev;
            off_t                offset;
            size_t               size;
        };

        static constexpr Partition make_partition(const struct device* dev, off_t offset, size_t size)
        {
            return Partition{
                .dev    = dev,
                .offset = offset,
                .size   = size,
            };
        }

        class HwaEmuEeprom : public zlibs::utils::emueeprom::Hwa
        {
            public:
            static constexpr size_t PAGE_1_SIZE                = FIXED_PARTITION_SIZE(emueeprom_page1_partition);
            static constexpr size_t PAGE_2_SIZE                = FIXED_PARTITION_SIZE(emueeprom_page2_partition);
            static constexpr size_t FACTORY_SIZE               = FIXED_PARTITION_SIZE(factory_partition);
            static constexpr size_t MAX_FLASH_WRITE_BLOCK_SIZE = 256;

            bool init() override
            {
                return device_is_ready(_page1.dev) && device_is_ready(_page2.dev) && device_is_ready(_factory.dev);
            }

            bool erase_page(zlibs::utils::emueeprom::Page page) override
            {
                const auto& partition = partition_for(page);
                return flash_erase(partition.dev, partition.offset, partition.size) == 0;
            }

            bool write_32(zlibs::utils::emueeprom::Page page, uint32_t offset, uint32_t data) override
            {
                const auto& partition      = partition_for(page);
                const auto  writeBlockSize = flash_get_write_block_size(partition.dev);

                if ((offset + sizeof(data)) > partition.size)
                {
                    return false;
                }

                if (writeBlockSize <= sizeof(data))
                {
                    return flash_write(partition.dev, partition.offset + offset, &data, sizeof(data)) == 0;
                }

                if (writeBlockSize > MAX_FLASH_WRITE_BLOCK_SIZE)
                {
                    return false;
                }

                const uint32_t alignedOffset = (offset / writeBlockSize) * writeBlockSize;
                const uint32_t withinBlock   = offset - alignedOffset;

                if ((withinBlock + sizeof(data)) > writeBlockSize)
                {
                    return false;
                }

                std::array<uint8_t, MAX_FLASH_WRITE_BLOCK_SIZE> buffer = {};

                if (flash_read(partition.dev, partition.offset + alignedOffset, buffer.data(), writeBlockSize) != 0)
                {
                    return false;
                }

                std::memcpy(buffer.data() + withinBlock, &data, sizeof(data));

                return flash_write(partition.dev, partition.offset + alignedOffset, buffer.data(), writeBlockSize) == 0;
            }

            std::optional<uint32_t> read_32(zlibs::utils::emueeprom::Page page, uint32_t offset) override
            {
                const auto& partition = partition_for(page);

                if ((offset + sizeof(uint32_t)) > partition.size)
                {
                    return {};
                }

                uint32_t data = 0;

                if (flash_read(partition.dev, partition.offset + offset, &data, sizeof(data)) != 0)
                {
                    return {};
                }

                return data;
            }

            private:
            static const Partition& partition_for(zlibs::utils::emueeprom::Page page)
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

            static inline const Partition _page1   = make_partition(FIXED_PARTITION_DEVICE(emueeprom_page1_partition),
                                                                    FIXED_PARTITION_OFFSET(emueeprom_page1_partition),
                                                                    FIXED_PARTITION_SIZE(emueeprom_page1_partition));
            static inline const Partition _page2   = make_partition(FIXED_PARTITION_DEVICE(emueeprom_page2_partition),
                                                                    FIXED_PARTITION_OFFSET(emueeprom_page2_partition),
                                                                    FIXED_PARTITION_SIZE(emueeprom_page2_partition));
            static inline const Partition _factory = make_partition(FIXED_PARTITION_DEVICE(factory_partition),
                                                                    FIXED_PARTITION_OFFSET(factory_partition),
                                                                    FIXED_PARTITION_SIZE(factory_partition));
        };

        bool                               _writeToCache = false;
        HwaEmuEeprom                       _hwaEmuEeprom;
        zlibs::utils::emueeprom::EmuEeprom _emuEEPROM = zlibs::utils::emueeprom::EmuEeprom(_hwaEmuEeprom);
    };
}    // namespace database
