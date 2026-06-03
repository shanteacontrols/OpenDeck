/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

namespace opendeck::firmware::database
{
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
}    // namespace opendeck::firmware::database
