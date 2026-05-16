/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef OPENDECK_TEST
#include "firmware/src/protocol/midi/builder/test/builder_test.h"
#else
#include "firmware/src/protocol/midi/builder/hw/builder_hw.h"
#endif
