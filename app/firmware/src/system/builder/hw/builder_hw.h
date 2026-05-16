/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/system/instance/impl/system.h"
#include "firmware/src/system/hwa/hw/hwa_hw.h"
#include "firmware/src/database/builder/builder.h"
#include "firmware/src/protocol/midi/builder/builder.h"
#include "firmware/src/io/analog/builder/builder.h"
#include "firmware/src/io/digital/builder/builder.h"
#include "firmware/src/io/touchscreen/builder/builder.h"
#include "firmware/src/io/i2c/builder/builder.h"
#include "firmware/src/io/indicators/builder/builder.h"
#include "firmware/src/io/outputs/builder/builder.h"

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
