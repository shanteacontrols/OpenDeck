/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <zephyr/devicetree.h>

#if DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(opendeck_analog))
#if DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_analog), controller_s3_gpios)
#define OPENDECK_ANALOG_PHYSICAL_COUNT 256
#elif DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_analog), controller_s2_gpios)
#define OPENDECK_ANALOG_PHYSICAL_COUNT 128
#else
#define OPENDECK_ANALOG_PHYSICAL_COUNT 64
#endif
#else
#define OPENDECK_ANALOG_PHYSICAL_COUNT 0
#endif

#if DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(opendeck_analog)) && DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_analog), index_remap)
#define OPENDECK_ANALOG_HAS_REMAP     1
#define OPENDECK_ANALOG_LOGICAL_COUNT DT_PROP_LEN(DT_NODELABEL(opendeck_analog), index_remap)
#else
#define OPENDECK_ANALOG_HAS_REMAP     0
#define OPENDECK_ANALOG_LOGICAL_COUNT OPENDECK_ANALOG_PHYSICAL_COUNT
#endif
