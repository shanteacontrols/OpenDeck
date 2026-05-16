/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/staged_update/hwa/stub/hwa_stub.h"
#include "firmware/src/staged_update/instance/impl/staged_update.h"

namespace opendeck::staged_update
{
    /**
     * @brief Builder that wires staged-update storage to a no-op backend.
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

        private:
        HwaStub      _hwa;
        StagedUpdate _instance;
    };
}    // namespace opendeck::staged_update
