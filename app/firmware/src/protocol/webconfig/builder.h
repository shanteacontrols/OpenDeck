/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef OPENDECK_TEST
#include "protocol/webconfig/builder_stub.h"
#else
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_WEBCONFIG
#include "protocol/webconfig/builder_hw.h"
#else
#include "protocol/webconfig/builder_stub.h"
#endif
#endif
