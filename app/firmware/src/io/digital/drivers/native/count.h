/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <zephyr/devicetree.h>

#if DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(opendeck_switches))
#define OPENDECK_SWITCH_PHYSICAL_COUNT DT_PROP_LEN(DT_NODELABEL(opendeck_switches), native_gpios)
#else
#define OPENDECK_SWITCH_PHYSICAL_COUNT 0
#endif

#if DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(opendeck_switches)) && DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_switches), index_remap)
#define OPENDECK_SWITCH_HAS_REMAP     1
#define OPENDECK_SWITCH_LOGICAL_COUNT DT_PROP_LEN(DT_NODELABEL(opendeck_switches), index_remap)
#else
#define OPENDECK_SWITCH_HAS_REMAP     0
#define OPENDECK_SWITCH_LOGICAL_COUNT OPENDECK_SWITCH_PHYSICAL_COUNT
#endif

#define OPENDECK_ENCODER_COUNT       (OPENDECK_SWITCH_LOGICAL_COUNT / 2)
#define OPENDECK_DIGITAL_INPUT_COUNT OPENDECK_SWITCH_PHYSICAL_COUNT
