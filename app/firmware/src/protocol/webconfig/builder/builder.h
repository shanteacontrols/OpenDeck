/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_WEBCONFIG
#include "firmware/src/protocol/webconfig/builder/hw/builder_hw.h"
#else
#include "firmware/src/protocol/webconfig/builder/stub/builder_stub.h"
#endif
