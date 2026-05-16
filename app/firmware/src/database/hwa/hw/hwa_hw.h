/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/database/deps.h"
#include "firmware/src/database/layout.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/io/indicators/indicators.h"

#include "zlibs/utils/emueeprom/emueeprom.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/kernel.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/sys/reboot.h>

#include <array>
#include <cstddef>
#include <cstring>
#include <cstdint>
#include <optional>
#include <span>

namespace opendeck::database
{
    /**
     * @brief Hardware-backed database storage backend.
     */
    class HwaHw : public Hwa
    {
        public:
        /**
         * @brief Flash write granularity reported by Zephyr for the EEPROM partitions.
         */
        static constexpr size_t FLASH_WRITE_BLOCK_SIZE = DT_PROP(DT_MEM_FROM_FIXED_PARTITION(DT_NODELABEL(emueeprom_page1_partition)), write_block_size);

        /**
         * @brief Smallest block size EmuEEPROM writes through this backend.
         *
         * Some flash drivers report byte-programmable storage, but this backend
         * still uses a minimum logical write block for database storage.
         */
        static constexpr size_t WRITE_BLOCK_SIZE = FLASH_WRITE_BLOCK_SIZE < sizeof(zlibs::utils::emueeprom::Entry) ? sizeof(zlibs::utils::emueeprom::Entry) : FLASH_WRITE_BLOCK_SIZE;

        /**
         * @brief Physical size of each flash partition used by EmuEEPROM.
         */
        static constexpr size_t EMUEEPROM_PAGE1_PARTITION_SIZE   = PARTITION_SIZE(emueeprom_page1_partition);
        static constexpr size_t EMUEEPROM_PAGE2_PARTITION_SIZE   = PARTITION_SIZE(emueeprom_page2_partition);
        static constexpr size_t EMUEEPROM_FACTORY_PARTITION_SIZE = PARTITION_SIZE(factory_partition);

        static_assert((EMUEEPROM_PAGE1_PARTITION_SIZE == EMUEEPROM_PAGE2_PARTITION_SIZE) &&
                          (EMUEEPROM_PAGE1_PARTITION_SIZE == EMUEEPROM_FACTORY_PARTITION_SIZE),
                      "EmuEEPROM partitions must have matching physical sizes.");

        /**
         * @brief Logical size of each EmuEEPROM page.
         */
        static constexpr size_t EMUEEPROM_PAGE_SIZE = EMUEEPROM_PAGE1_PARTITION_SIZE;

        /**
         * @brief Number of logical addresses supported by the physical EmuEEPROM journal.
         */
        static constexpr uint32_t EMUEEPROM_ADDRESS_COUNT = static_cast<uint32_t>(zlibs::utils::emueeprom::address_count_for(EMUEEPROM_PAGE_SIZE, WRITE_BLOCK_SIZE));

        /**
         * @brief Number of preset slots supported by the configured database storage.
         */
        static constexpr size_t SUPPORTED_PRESET_COUNT = AppLayout::supported_preset_count_for(EMUEEPROM_ADDRESS_COUNT);

        /**
         * @brief Number of logical database addresses exposed through LessDB.
         */
        static constexpr uint32_t DATABASE_ADDRESS_COUNT = AppLayout::database_size_for_presets(SUPPORTED_PRESET_COUNT);

        static_assert(SUPPORTED_PRESET_COUNT > 0, "Database storage must support at least one preset.");

        /**
         * @brief Constructs the hardware-backed database backend and subscribes to restore notifications.
         */
        HwaHw()
        {
            signaling::subscribe<signaling::SystemSignal>(
                [this](const signaling::SystemSignal& signal)
                {
                    switch (signal.system_event)
                    {
                    case signaling::SystemEvent::BackupStart:
                    {
                        _backup_restore_active = true;
                    }
                    break;

                    case signaling::SystemEvent::BackupEnd:
                    {
                        _backup_restore_active = false;
                    }
                    break;

                    case signaling::SystemEvent::RestoreStart:
                    case signaling::SystemEvent::FactoryResetStart:
                    {
                        if (signal.system_event == signaling::SystemEvent::RestoreStart)
                        {
                            _backup_restore_active = true;
                        }

                        _bulk_write_active = true;
                        update_cached_write_state();
                    }
                    break;

                    case signaling::SystemEvent::RestoreEnd:
                    case signaling::SystemEvent::FactoryResetEnd:
                    {
                        _bulk_write_active = false;

                        if (signal.system_event == signaling::SystemEvent::RestoreEnd)
                        {
                            _backup_restore_active = false;
                        }

                        update_cached_write_state();
                    }
                    break;

                    case signaling::SystemEvent::PresetChanged:
                    {
                        if (!_backup_restore_active && !_bulk_write_active && !_configuration_session_open)
                        {
                            _emueeprom.flush();
                        }
                    }
                    break;

                    case signaling::SystemEvent::ConfigurationSessionOpened:
                    {
                        _configuration_session_open = true;
                        update_cached_write_state();
                    }
                    break;

                    case signaling::SystemEvent::ConfigurationSessionClosed:
                    {
                        _configuration_session_open = false;
                        update_cached_write_state();
                    }
                    break;

                    default:
                        break;
                    }
                });
        }

        /**
         * @brief Initializes the emulated EEPROM backend.
         *
         * @return `true` if the underlying flash partitions are ready, otherwise `false`.
         */
        bool init() override
        {
            return _emueeprom.init();
        }

        /**
         * @brief Returns the maximum database address supported by the emulated EEPROM backend.
         *
         * @return Highest supported database address.
         */
        uint32_t address_count() override
        {
            return DATABASE_ADDRESS_COUNT;
        }

        /**
         * @brief Erases the emulated EEPROM contents.
         *
         * @return `true` if formatting succeeded, otherwise `false`.
         */
        bool clear() override
        {
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_TRAFFIC_INDICATORS
            // It's possible that OUTPUT indicators are still on since
            // this command is most likely given via USB.
            // Wait until all indicators are turned off
            k_msleep(io::indicators::Indicators::TRAFFIC_INDICATOR_TIMEOUT_MS);
#endif

            return _emueeprom.format();
        }

        /**
         * @brief Reads one database value from emulated EEPROM storage.
         *
         * @param address Database address to read.
         * @param type Database parameter type stored at `address`.
         *
         * @return Stored value, `0` when the variable is unset, or `std::nullopt` on read failure.
         */
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
            break;

            default:
                return {};
            }
        }

        /**
         * @brief Writes one database value to emulated EEPROM storage.
         *
         * @param address Database address to update.
         * @param value Value to store.
         * @param type Database parameter type stored at `address`.
         *
         * @return `true` if the value was written successfully, otherwise `false`.
         */
        bool write(uint32_t address, uint32_t value, zlibs::utils::lessdb::SectionParameterType type) override
        {
            uint16_t write_value = value;

            switch (type)
            {
            case zlibs::utils::lessdb::SectionParameterType::Bit:
            case zlibs::utils::lessdb::SectionParameterType::Byte:
            case zlibs::utils::lessdb::SectionParameterType::HalfByte:
            case zlibs::utils::lessdb::SectionParameterType::Word:
                return _emueeprom.write(zlibs::utils::emueeprom::make_entry(static_cast<uint16_t>(address), write_value),
                                        _write_to_cache) == zlibs::utils::emueeprom::WriteStatus::Ok;

            default:
                return false;
            }
        }

        /**
         * @brief Returns whether a valid factory snapshot is available.
         *
         * @return `true` when the factory page contains valid data, otherwise `false`.
         */
        bool has_factory_snapshot() override
        {
            return _emueeprom.page_status(zlibs::utils::emueeprom::Page::Factory) ==
                   zlibs::utils::emueeprom::PageStatus::Valid;
        }

        /**
         * @brief Stores the current database contents into the factory snapshot area.
         *
         * @return `true` if the factory snapshot was stored successfully, otherwise `false`.
         */
        bool store_factory_snapshot() override
        {
            return _emueeprom.store_to_factory();
        }

        /**
         * @brief Restores the database contents from the factory snapshot area.
         *
         * @return `true` if the factory snapshot was restored successfully, otherwise `false`.
         */
        bool restore_factory_snapshot() override
        {
            return _emueeprom.restore_from_factory();
        }

        private:
        using EmuEeprom = zlibs::utils::emueeprom::EmuEeprom<EMUEEPROM_PAGE_SIZE, WRITE_BLOCK_SIZE, DATABASE_ADDRESS_COUNT>;

        /**
         * @brief Updates cache-only write mode from active database write sessions.
         */
        void update_cached_write_state()
        {
            const bool was_cache_enabled = _write_to_cache;
            const bool enable_cache      = _bulk_write_active || _configuration_session_open;

            _write_to_cache = enable_cache;

            if (was_cache_enabled && !_write_to_cache)
            {
                _emueeprom.flush();
            }
        }

        /**
         * @brief Describes one flash partition used by the emulated EEPROM backend.
         */
        struct Partition
        {
            const struct device* dev;
            off_t                offset;
            size_t               size;
        };

        /**
         * @brief Creates a flash partition descriptor for the emulated EEPROM backend.
         *
         * @param dev Flash device that owns the partition.
         * @param offset Partition offset within `dev`.
         * @param size Partition size in bytes.
         *
         * @return Partition descriptor initialized with the provided values.
         */
        static constexpr Partition make_partition(const struct device* dev, off_t offset, size_t size)
        {
            return Partition{
                .dev    = dev,
                .offset = offset,
                .size   = size,
            };
        }

        /**
         * @brief Flash-backed emulated EEPROM hardware adapter used by the database backend.
         */
        class HwaEmuEeprom : public zlibs::utils::emueeprom::Hwa
        {
            public:
            /**
             * @brief Returns whether all flash devices used by the emulated EEPROM backend are ready.
             *
             * @return `true` if every partition device is ready, otherwise `false`.
             */
            bool init() override
            {
                return device_is_ready(PAGE_1.dev) &&
                       device_is_ready(PAGE_2.dev) &&
                       device_is_ready(FACTORY.dev);
            }

            /**
             * @brief Erases the flash partition backing one emulated EEPROM page.
             *
             * @param page Emulated EEPROM page to erase.
             *
             * @return `true` if the erase operation succeeded, otherwise `false`.
             */
            bool erase_page(zlibs::utils::emueeprom::Page page) override
            {
                const auto& partition = partition_for(page);
                return flash_erase(partition.dev, partition.offset, partition.size) == 0;
            }

            /**
             * @brief Writes one backend write block to the selected emulated EEPROM page.
             *
             * @param page Emulated EEPROM page to update.
             * @param offset Byte offset within `page`.
             * @param data Backend write-block bytes to store.
             *
             * @return `true` if the block was written successfully, otherwise `false`.
             */
            bool write(zlibs::utils::emueeprom::Page page, uint32_t offset, std::span<const uint8_t> data) override
            {
                const auto& partition = partition_for(page);

                if ((data.size() != HwaHw::WRITE_BLOCK_SIZE) ||
                    ((offset % HwaHw::WRITE_BLOCK_SIZE) != 0) ||
                    ((offset + data.size()) > HwaHw::EMUEEPROM_PAGE_SIZE))
                {
                    return false;
                }

                return flash_write(partition.dev, static_cast<off_t>(partition.offset + static_cast<off_t>(offset)), data.data(), data.size()) == 0;
            }

            /**
             * @brief Reads one or more backend write blocks from the selected emulated EEPROM page.
             *
             * @param page Emulated EEPROM page to read.
             * @param offset Byte offset within `page`.
             * @param data Output buffer populated with backend write-block bytes.
             *
             * @return `true` if the block was read successfully, otherwise `false`.
             */
            bool read(zlibs::utils::emueeprom::Page page, uint32_t offset, std::span<uint8_t> data) override
            {
                const auto& partition = partition_for(page);

                if ((data.empty()) ||
                    ((data.size() % HwaHw::WRITE_BLOCK_SIZE) != 0) ||
                    ((offset % HwaHw::WRITE_BLOCK_SIZE) != 0) ||
                    ((offset + data.size()) > HwaHw::EMUEEPROM_PAGE_SIZE))
                {
                    return false;
                }

                return flash_read(partition.dev, static_cast<off_t>(partition.offset + static_cast<off_t>(offset)), data.data(), data.size()) == 0;
            }

            private:
            /**
             * @brief Returns the flash partition descriptor that backs the selected emulated EEPROM page.
             *
             * @param page Emulated EEPROM page to resolve.
             *
             * @return Partition descriptor associated with `page`.
             */
            static const Partition& partition_for(zlibs::utils::emueeprom::Page page)
            {
                switch (page)
                {
                case zlibs::utils::emueeprom::Page::Page1:
                    return PAGE_1;

                case zlibs::utils::emueeprom::Page::Page2:
                    return PAGE_2;

                case zlibs::utils::emueeprom::Page::Factory:
                    return FACTORY;
                }

                return PAGE_1;
            }

            static constexpr Partition PAGE_1 = {
                .dev    = PARTITION_DEVICE(emueeprom_page1_partition),
                .offset = PARTITION_OFFSET(emueeprom_page1_partition),
                .size   = PARTITION_SIZE(emueeprom_page1_partition),
            };

            static constexpr Partition PAGE_2 = {
                .dev    = PARTITION_DEVICE(emueeprom_page2_partition),
                .offset = PARTITION_OFFSET(emueeprom_page2_partition),
                .size   = PARTITION_SIZE(emueeprom_page2_partition),
            };

            static constexpr Partition FACTORY = {
                .dev    = PARTITION_DEVICE(factory_partition),
                .offset = PARTITION_OFFSET(factory_partition),
                .size   = PARTITION_SIZE(factory_partition),
            };
        };

        bool         _bulk_write_active          = false;
        bool         _backup_restore_active      = false;
        bool         _configuration_session_open = false;
        bool         _write_to_cache             = false;
        HwaEmuEeprom _hwa_emueeprom;
        EmuEeprom    _emueeprom = EmuEeprom(_hwa_emueeprom);
    };
}    // namespace opendeck::database
