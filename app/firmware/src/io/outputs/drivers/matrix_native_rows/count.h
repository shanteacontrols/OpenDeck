/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <zephyr/devicetree.h>

#if DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(opendeck_outputs))
#define OPENDECK_OUTPUT_PHYSICAL_COUNT (DT_PROP(DT_NODELABEL(opendeck_outputs), rows) * DT_PROP(DT_NODELABEL(opendeck_outputs), columns))
#else
#define OPENDECK_OUTPUT_PHYSICAL_COUNT 0
#endif

#if DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(opendeck_outputs)) && DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_outputs), index_remap)
#define OPENDECK_OUTPUT_HAS_REMAP     1
#define OPENDECK_OUTPUT_LOGICAL_COUNT DT_PROP_LEN(DT_NODELABEL(opendeck_outputs), index_remap)
#else
#define OPENDECK_OUTPUT_HAS_REMAP     0
#define OPENDECK_OUTPUT_LOGICAL_COUNT OPENDECK_OUTPUT_PHYSICAL_COUNT
#endif
