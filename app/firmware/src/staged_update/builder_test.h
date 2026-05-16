/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/staged_update/hwa/test/hwa_test.h"
#include "firmware/src/staged_update/staged_update.h"

namespace opendeck::staged_update
{
    /**
     * @brief Builder that wires staged-update storage to the in-memory test backend.
     */
    class Builder
    {
        public:
        Builder()
            : _instance(_hwa)
        {}

        StagedUpdate& instance()
        {
            return _instance;
        }

        HwaTest& hwa()
        {
            return _hwa;
        }

        private:
        HwaTest      _hwa;
        StagedUpdate _instance;
    };
}    // namespace opendeck::staged_update
