/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <zephyr/devicetree.h>

#if DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(opendeck_leds))
#define OPENDECK_LED_OUTPUT_PHYSICAL_COUNT (64 * DT_PROP(DT_NODELABEL(opendeck_leds), drivers))
#else
#define OPENDECK_LED_OUTPUT_PHYSICAL_COUNT 0
#endif

#if DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(opendeck_leds)) && DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_leds), index_remap)
#define OPENDECK_LED_OUTPUT_HAS_REMAP     1
#define OPENDECK_LED_OUTPUT_LOGICAL_COUNT DT_PROP_LEN(DT_NODELABEL(opendeck_leds), index_remap)
#else
#define OPENDECK_LED_OUTPUT_HAS_REMAP     0
#define OPENDECK_LED_OUTPUT_LOGICAL_COUNT OPENDECK_LED_OUTPUT_PHYSICAL_COUNT
#endif
