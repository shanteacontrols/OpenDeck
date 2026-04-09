/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <zephyr/devicetree.h>

#define OPENDECK_TOUCHSCREEN_NODE DT_NODELABEL(opendeck_touchscreen)

#if DT_NODE_HAS_STATUS_OKAY(OPENDECK_TOUCHSCREEN_NODE) && DT_NODE_HAS_PROP(OPENDECK_TOUCHSCREEN_NODE, component_count)
#define OPENDECK_TOUCHSCREEN_COMPONENT_COUNT DT_PROP(OPENDECK_TOUCHSCREEN_NODE, component_count)
#else
#define OPENDECK_TOUCHSCREEN_COMPONENT_COUNT 0
#endif
