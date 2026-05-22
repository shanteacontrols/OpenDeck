/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/staged_update_reader/instance/stub/staged_update_reader_stub.h"

namespace opendeck::staged_update_reader
{
    /**
     * @brief Builder that wires stub staged-update storage.
     */
    class Builder
    {
        public:
        /**
         * @brief Returns the stub staged-update reader.
         *
         * @return Stub staged-update reader.
         */
        StagedUpdateReaderStub& instance()
        {
            return _instance;
        }

        private:
        StagedUpdateReaderStub _instance;
    };
}    // namespace opendeck::staged_update_reader
