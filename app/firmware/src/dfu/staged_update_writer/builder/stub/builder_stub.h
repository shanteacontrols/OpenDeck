/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/dfu/staged_update_writer/instance/stub/staged_update_writer_stub.h"

namespace opendeck::firmware::dfu::staged_update_writer
{
    /**
     * @brief Builder that exposes a disabled staged-update destination.
     */
    class Builder
    {
        public:
        Builder() = default;

        StagedUpdateWriter& instance()
        {
            return _instance;
        }

        private:
        StagedUpdateWriter _instance;
    };
}    // namespace opendeck::firmware::dfu::staged_update_writer
