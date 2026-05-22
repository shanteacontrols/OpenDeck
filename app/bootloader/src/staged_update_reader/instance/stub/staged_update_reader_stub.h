/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/staged_update_reader/shared/deps.h"

namespace opendeck::staged_update_reader
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
        bool consume(dfu_stream::Sink&)
        {
            return false;
        }
    };
}    // namespace opendeck::staged_update_reader
