/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef OPENDECK_TEST
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_DIGITAL_INPUTS
#include "io/digital/builder_test.h"
#else
#include "io/digital/builder_stub.h"
#endif
#else
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_DIGITAL_INPUTS
#include "io/digital/builder_hw.h"
#else
#include "io/digital/builder_stub.h"
#endif
#endif
