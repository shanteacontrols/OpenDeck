/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/system/instance/impl/system.h"
#include "firmware/src/system/hwa/test/hwa_test.h"
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
