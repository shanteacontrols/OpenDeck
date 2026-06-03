/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/staged_update_reader/instance/impl/deps.h"

namespace opendeck::bootloader::dfu::staged_update_reader
{
    /**
     * @brief Reads a staged firmware payload from storage and streams it into a DFU stream destination.
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
         * @brief Streams a staged firmware payload into a DFU stream destination when a valid marker is present.
         *
         * @param consumer Destination for staged firmware payload bytes.
         *
         * @return `true` when a staged update was found and consumed, otherwise `false`.
         */
        bool consume(opendeck::common::dfu::dfu_stream_parser::Destination& consumer);

        /**
         * @brief Invalidates the staged-update marker.
         */
        bool clear_pending();

        private:
        Hwa& _hwa;
    };
}    // namespace opendeck::bootloader::dfu::staged_update_reader
