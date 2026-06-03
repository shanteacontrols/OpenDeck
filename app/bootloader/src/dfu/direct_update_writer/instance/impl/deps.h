/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace opendeck::bootloader::dfu::direct_update_writer
{
    /**
     * @brief Hardware action used after a direct firmware update is written.
     */
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        /**
         * @brief Applies the completed update.
         */
        virtual void apply() = 0;
    };
}    // namespace opendeck::bootloader::dfu::direct_update_writer
