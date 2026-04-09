/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "zlibs/utils/lessdb/lessdb.h"

namespace database
{
    /**
     * @brief Hardware abstraction interface for database storage access.
     */
    class Hwa : public zlibs::utils::lessdb::Hwa
    {
        public:
        /**
         * @brief Returns whether a valid factory snapshot is provisioned.
         *
         * @return `true` when the factory snapshot can be restored.
         */
        virtual bool has_factory_snapshot() = 0;

        /**
         * @brief Stores the current initialized database contents as a factory snapshot.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool store_factory_snapshot() = 0;

        /**
         * @brief Restores the active database contents from the factory snapshot.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool restore_factory_snapshot() = 0;
    };

    /**
     * @brief Supplies compile-time layout descriptors for the database regions.
     *
     * The layout is injected to avoid pulling application-wide dependencies
     * into the database implementation while still allowing the final layout
     * to be defined close to the modules that contribute sections.
     */
    class Layout
    {
        public:
        virtual ~Layout() = default;

        /**
         * @brief Returns the layout shared by all preset slots.
         *
         * @return Common-region layout descriptor.
         */
        virtual std::span<const zlibs::utils::lessdb::Block> common_layout() const = 0;

        /**
         * @brief Returns the layout repeated once per preset slot.
         *
         * @return Preset-region layout descriptor.
         */
        virtual std::span<const zlibs::utils::lessdb::Block> preset_layout() const = 0;

        /**
         * @brief Returns the common layout size in bytes.
         *
         * @return Common layout size.
         */
        virtual uint32_t common_layout_size() const = 0;

        /**
         * @brief Returns the preset layout size in bytes.
         *
         * @return Preset layout size.
         */
        virtual uint32_t preset_layout_size() const = 0;

        /**
         * @brief Returns the Uid for the common layout.
         *
         * @return Common layout Uid.
         */
        virtual uint16_t common_uid() const = 0;

        /**
         * @brief Returns the Uid for the preset layout.
         *
         * @return Preset layout Uid.
         */
        virtual uint16_t preset_uid() const = 0;
    };

    /**
     * @brief Observer interface for database lifecycle events.
     */
    class Handlers
    {
        public:
        virtual ~Handlers() = default;

        /**
         * @brief Notifies that the active preset changed.
         *
         * @param preset Newly selected preset.
         */
        virtual void preset_change(uint8_t preset) = 0;

        /**
         * @brief Notifies that a factory reset is starting.
         */
        virtual void factory_reset_start() = 0;

        /**
         * @brief Notifies that a factory reset finished.
         */
        virtual void factory_reset_done() = 0;

        /**
         * @brief Notifies that database initialization completed.
         */
        virtual void initialized() = 0;
    };
}    // namespace database
