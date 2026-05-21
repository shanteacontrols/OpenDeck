/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/staged_update_reader/shared/deps.h"

namespace opendeck::staged_update_reader
{
    /**
     * @brief Reads a staged dfu.bin update from storage and streams it into a consumer.
     */
    class StagedUpdateReader
    {
        public:
        /**
         * @brief Constructs a staged-update reader bound to staged storage.
         *
         * @param hwa Hardware abstraction used to access staged storage.
         */
        explicit StagedUpdateReader(Hwa& hwa);

        /**
         * @brief Streams a staged dfu.bin update into a consumer when a valid marker is present.
         *
         * @param consumer Destination for staged dfu.bin bytes.
         *
         * @return `true` when a staged update was found and consumed, otherwise `false`.
         */
        bool consume(Consumer& consumer);

        /**
         * @brief Invalidates the staged-update marker.
         */
        void clear_pending();

        private:
        Hwa& _hwa;
    };
}    // namespace opendeck::staged_update_reader
