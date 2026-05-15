/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef OPENDECK_TEST
#include "firmware/src/protocol/webconfig/builder_stub.h"
#else
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_WEBCONFIG
#include "firmware/src/protocol/webconfig/builder_hw.h"
#else
#include "firmware/src/protocol/webconfig/builder_stub.h"
#endif
#endif
