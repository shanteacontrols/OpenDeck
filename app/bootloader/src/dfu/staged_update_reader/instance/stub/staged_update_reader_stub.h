/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/writer/instance/impl/dfu_writer.h"

namespace opendeck::bootloader::dfu::staged_update_reader
{
    /**
     * @brief Stub staged-update reader used when staged updates are disabled.
     */
    class StagedUpdateReaderStub
    {
        public:
        /**
         * @brief Treats disabled staged-update storage as empty.
         *
         * @return Always `false`.
         */
        bool consume(opendeck::common::dfu::writer::DfuWriter&)
        {
            return false;
        }
    };
}    // namespace opendeck::bootloader::dfu::staged_update_reader
