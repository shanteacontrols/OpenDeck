/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/osc/osc_stub.h"
#include "firmware/src/database/database.h"

namespace opendeck::protocol::osc
{
    /**
     * @brief Stub builder that wires OSC to a no-op backend.
     */
    class Builder
    {
        public:
        Builder() = default;

        /**
         * @brief Constructs the OSC stub builder with the same signature as the hardware builder.
         */
        explicit Builder([[maybe_unused]] database::Admin& database)
        {}

        /**
         * @brief Returns the constructed OSC stub backend.
         *
         * @return No-op OSC protocol instance.
         */
        Osc& instance()
        {
            return _instance;
        }

        private:
        Osc _instance;
    };
}    // namespace opendeck::protocol::osc
