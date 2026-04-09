/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef OPENDECK_TEST
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_BUTTONS
#include "io/digital/buttons/builder_test.h"
#else
#include "io/digital/buttons/builder_stub.h"
#endif
#else
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_BUTTONS
#include "io/digital/buttons/builder_hw.h"
#else
#include "io/digital/buttons/builder_stub.h"
#endif
#endif
