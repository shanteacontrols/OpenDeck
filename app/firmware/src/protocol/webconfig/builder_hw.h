/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "hwa_hw.h"
#include "staged_update/builder.h"
#include "webconfig.h"

namespace opendeck::protocol::webconfig
{
    /**
     * @brief Convenience builder that wires the WebSocket configuration endpoint.
     */
    class Builder
    {
        public:
        Builder()
            : _instance(_hwa, _firmware_builder.instance())
        {}

        /**
         * @brief Returns the configured WebConfig protocol instance.
         *
         * @return WebConfig protocol instance.
         */
        WebConfig& instance()
        {
            return _instance;
        }

        private:
        staged_update::Builder _firmware_builder;
        HwaHw                  _hwa;
        WebConfig              _instance;
    };
}    // namespace opendeck::protocol::webconfig
