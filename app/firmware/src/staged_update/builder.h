/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef OPENDECK_TEST
#include "firmware/src/staged_update/builder_test.h"
#else
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_STAGED_UPDATE
#include "firmware/src/staged_update/builder_hw.h"
#else
#include "firmware/src/staged_update/builder_stub.h"
#endif
#endif
