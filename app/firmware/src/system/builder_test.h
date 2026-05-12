/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "system.h"
#include "hwa_test.h"
#include "database/builder.h"
#include "protocol/midi/builder.h"
#include "io/analog/builder.h"
#include "io/digital/builder.h"
#include "io/touchscreen/builder.h"
#include "io/i2c/builder.h"
#include "io/indicators/builder.h"
#include "io/outputs/builder.h"

namespace opendeck::sys
{
    /**
     * @brief Test-system builder that wires all subsystem test implementations.
     */
    class Builder
    {
        public:
        Builder() = default;

        HwaTest _hwa;
        System  _instance = System(_hwa);
    };
}    // namespace opendeck::sys
