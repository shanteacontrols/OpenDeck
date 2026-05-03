/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "drivers/count.h"

#include <array>
#include <cstddef>
#include <zephyr/devicetree.h>

namespace opendeck::io::leds
{
    /**
     * @brief Resolves logical LED indices against the physical driver order.
     */
    class Remap
    {
        public:
        /**
         * @brief Maps one logical LED index to the physical output index.
         *
         * @param logical Logical LED index.
         *
         * @return Physical LED output index.
         */
        static constexpr size_t physical(size_t logical)
        {
#if OPENDECK_LED_OUTPUT_HAS_REMAP
            return INDEX_REMAP[logical];
#else
            return logical;
#endif
        }

        /**
         * @brief Maps one physical LED output index back to its logical index.
         *
         * @param physical Physical LED output index.
         *
         * @return Logical LED index when remapped, otherwise the original index.
         */
        static constexpr size_t logical(size_t physical)
        {
#if OPENDECK_LED_OUTPUT_HAS_REMAP
            for (size_t i = 0; i < INDEX_REMAP.size(); i++)
            {
                if (INDEX_REMAP[i] == physical)
                {
                    return i;
                }
            }
#endif
            return physical;
        }

        private:
#if OPENDECK_LED_OUTPUT_HAS_REMAP
#define OPENDECK_LED_REMAP_ENTRY(index, _) DT_PROP_BY_IDX(DT_NODELABEL(opendeck_leds), index_remap, index)
        inline static constexpr std::array<size_t, OPENDECK_LED_OUTPUT_LOGICAL_COUNT> INDEX_REMAP = {
            LISTIFY(OPENDECK_LED_OUTPUT_LOGICAL_COUNT, OPENDECK_LED_REMAP_ENTRY, (, ))
        };
#undef OPENDECK_LED_REMAP_ENTRY
#endif
    };
}    // namespace opendeck::io::leds
