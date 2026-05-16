/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/staged_update/hwa/hw/hwa_hw.h"
#include "firmware/src/staged_update/instance/impl/staged_update.h"

namespace opendeck::staged_update
{
    /**
     * @brief Builder that wires staged-update storage to real flash.
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
        HwaHw        _hwa;
        StagedUpdate _instance;
    };
}    // namespace opendeck::staged_update
