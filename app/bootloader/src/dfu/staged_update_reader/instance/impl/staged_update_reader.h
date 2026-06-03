/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/staged_update_reader/instance/impl/deps.h"
#include "common/src/dfu/staged_update/shared/deps.h"
#include "common/src/dfu/writer/instance/impl/dfu_writer.h"

namespace opendeck::bootloader::dfu::staged_update_reader
{
    /**
     * @brief Reads a staged firmware payload from storage and streams it into a DFU writer.
     */
    class StagedUpdateReader : public opendeck::common::dfu::staged_update::StagedUpdate
    {
        public:
        /**
         * @brief Constructs a staged-update reader bound to staged storage.
         *
         * @param hwa Hardware abstraction used to access staged storage.
         */
        explicit StagedUpdateReader(Hwa& hwa);

        /**
         * @brief Streams a staged firmware payload into a DFU writer when a valid marker is present.
         *
         * @param writer Writer for staged firmware payload bytes.
         *
         * @return `true` when a staged update was found and consumed, otherwise `false`.
         */
        bool consume(opendeck::common::dfu::writer::DfuWriter& writer);

        private:
        Hwa& _hwa;

        /**
         * @brief Invalidates the staged-update marker.
         */
        bool clear_pending();

        /**
         * @brief Reads the staged DFU header from the start of the partition.
         */
        bool read_header(opendeck::common::dfu::dfu_stream_parser::Header& header);
    };
}    // namespace opendeck::bootloader::dfu::staged_update_reader
