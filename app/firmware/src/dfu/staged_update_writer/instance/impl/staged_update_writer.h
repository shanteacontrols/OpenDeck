/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/staged_update/shared/common.h"
#include "common/src/dfu/writer/instance/impl/dfu_writer.h"
#include "firmware/src/dfu/staged_update_writer/instance/impl/deps.h"

#include <array>
#include <optional>
#include <span>

namespace opendeck::firmware::dfu::staged_update_writer
{
    /**
     * @brief Stores a validated firmware payload in the configured staging partition.
     */
    class StagedUpdateWriter : public opendeck::common::dfu::writer::DfuWriter
    {
        public:
        /**
         * @brief Constructs staged-update storage around a flash backend.
         *
         * @param hwa Flash storage backend used by staged update.
         */
        explicit StagedUpdateWriter(opendeck::firmware::dfu::staged_update_writer::Hwa& hwa);

        /**
         * @brief Starts a staged upload and invalidates any old staged marker.
         */
        bool begin(const opendeck::common::dfu::dfu_stream_parser::Header& header, uint32_t expected_size) override;

        private:
        opendeck::firmware::dfu::staged_update_writer::Hwa& _hwa;

        /**
         * @brief Erases the first sector so the bootloader ignores any old upload.
         */
        bool erase_header_sector();

        /**
         * @brief Writes the accepted DFU header at the beginning of the staged partition.
         */
        bool write_header(const opendeck::common::dfu::dfu_stream_parser::Header& header);

        /**
         * @brief Writes the staged marker after payload bytes are flushed.
         */
        bool commit(const opendeck::common::dfu::dfu_stream_parser::Header& header, uint32_t expected_size) override;

        /**
         * @brief Invalidates staged update header after a rejected transfer.
         */
        void cancel() override;

        /**
         * @brief Returns the offset assigned to the first staged payload byte.
         */
        uint32_t payload_offset() const override;
    };
}    // namespace opendeck::firmware::dfu::staged_update_writer
