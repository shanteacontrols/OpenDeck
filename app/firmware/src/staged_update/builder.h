/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef OPENDECK_TEST
#include "builder_test.h"
#else
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_STAGED_UPDATE
#include "builder_hw.h"
#else
#include "builder_stub.h"
#endif
#endif
