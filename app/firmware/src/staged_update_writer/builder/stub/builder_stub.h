/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/staged_update_writer/hwa/stub/hwa_stub.h"
#include "firmware/src/staged_update_writer/instance/impl/staged_update_writer.h"

namespace opendeck::staged_update_writer
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

        StagedUpdateWriter& instance()
        {
            return _instance;
        }

        private:
        HwaStub            _hwa;
        StagedUpdateWriter _instance;
    };
}    // namespace opendeck::staged_update_writer
