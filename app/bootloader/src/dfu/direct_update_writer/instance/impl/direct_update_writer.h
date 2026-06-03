/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/direct_update_writer/instance/impl/deps.h"
#include "common/src/dfu/dfu_stream_parser/shared/common.h"
#include "common/src/dfu/flash_area/impl/deps.h"
#include "common/src/dfu/writer/instance/impl/dfu_writer.h"

namespace opendeck::bootloader::dfu::direct_update_writer
{
    /**
     * @brief Writes validated firmware payload bytes to the application slot.
     */
    class DirectUpdateWriter : public opendeck::common::dfu::writer::DfuWriter
    {
        public:
        /**
         * @brief Constructs a direct-update writer bound to the hardware abstraction.
         *
         * @param hwa Hardware abstraction used to store the incoming firmware image.
         */
        explicit DirectUpdateWriter(opendeck::common::dfu::flash_area::Hwa&               flash_hwa,
                                    opendeck::bootloader::dfu::direct_update_writer::Hwa& hwa);

        private:
        opendeck::bootloader::dfu::direct_update_writer::Hwa& _hwa;

        /**
         * @brief Applies the completed firmware payload.
         */
        bool commit(const opendeck::common::dfu::dfu_stream_parser::Header& header, uint32_t expected_size) override;
    };
}    // namespace opendeck::bootloader::dfu::direct_update_writer
