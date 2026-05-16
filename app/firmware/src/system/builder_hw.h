/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/system/system.h"
#include "firmware/src/system/hwa_hw.h"
#include "firmware/src/database/builder.h"
#include "firmware/src/protocol/midi/builder.h"
#include "firmware/src/io/analog/builder.h"
#include "firmware/src/io/digital/builder.h"
#include "firmware/src/io/touchscreen/builder.h"
#include "firmware/src/io/i2c/builder.h"
#include "firmware/src/io/indicators/builder.h"
#include "firmware/src/io/outputs/builder.h"

namespace opendeck::sys
{
    static_assert(IS_ENABLED(CONFIG_PROJECT_TARGET_SUPPORT_CONFIG_INTERFACE),
                  "At least one OpenDeck configuration interface must be enabled.");

    /**
     * @brief Hardware-system builder that wires all production subsystem instances.
     */
    class Builder
    {
        public:
        Builder() = default;

        System& instance()
        {
            return _instance;
        }

        private:
        HwaHw  _hwa;
        System _instance = System(_hwa);
    };
}    // namespace opendeck::sys
