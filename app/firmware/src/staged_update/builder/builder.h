/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef OPENDECK_TEST
#include "firmware/src/staged_update/builder/test/builder_test.h"
#else
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_STAGED_UPDATE
#include "firmware/src/staged_update/builder/hw/builder_hw.h"
#else
#include "firmware/src/staged_update/builder/stub/builder_stub.h"
#endif
#endif
