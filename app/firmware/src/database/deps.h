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
