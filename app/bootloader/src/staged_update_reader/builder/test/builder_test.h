/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/staged_update_reader/hwa/test/hwa_test.h"
#include "bootloader/src/staged_update_reader/instance/impl/staged_update_reader.h"

namespace opendeck::staged_update_reader
{
    /**
     * @brief Builder that wires staged-update reader storage to an in-memory test backend.
     */
    class Builder
    {
        public:
        Builder()
            : _instance(_hwa)
        {}

        StagedUpdateReader& instance()
        {
            return _instance;
        }

        HwaTest _hwa;

        private:
        StagedUpdateReader _instance;
    };
}    // namespace opendeck::staged_update_reader
