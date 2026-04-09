/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// clang-format off

#ifdef OPENDECK_TEST
    #ifdef CONFIG_PROJECT_TARGET_SUPPORT_ENCODERS
        #include "io/digital/encoders/builder_test.h"
    #else
        #include "io/digital/encoders/builder_stub.h"
    #endif
#else
    #ifdef CONFIG_PROJECT_TARGET_SUPPORT_ENCODERS
        #include "io/digital/encoders/builder_hw.h"
    #else
        #include "io/digital/encoders/builder_stub.h"
    #endif
#endif

// clang-format on
