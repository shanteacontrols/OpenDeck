/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "drivers/count.h"

#include <array>
#include <cstddef>
#include <zephyr/devicetree.h>

namespace opendeck::io::outputs
{
    /**
     * @brief Resolves logical OUTPUT indices against the physical driver order.
     */
    class Remap
    {
        public:
        /**
         * @brief Maps one logical OUTPUT index to the physical output index.
         *
         * @param logical Logical OUTPUT index.
         *
         * @return Physical OUTPUT output index.
         */
        static constexpr size_t physical(size_t logical)
        {
#if OPENDECK_OUTPUT_HAS_REMAP
            return INDEX_REMAP[logical];
#else
            return logical;
#endif
        }

        /**
         * @brief Maps one physical OUTPUT output index back to its logical index.
         *
         * @param physical Physical OUTPUT output index.
         *
         * @return Logical OUTPUT index when remapped, otherwise the original index.
         */
        static constexpr size_t logical(size_t physical)
        {
#if OPENDECK_OUTPUT_HAS_REMAP
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
#if OPENDECK_OUTPUT_HAS_REMAP
#define OPENDECK_OUTPUT_REMAP_ENTRY(index, _) DT_PROP_BY_IDX(DT_NODELABEL(opendeck_outputs), index_remap, index)
        inline static constexpr std::array<size_t, OPENDECK_OUTPUT_LOGICAL_COUNT> INDEX_REMAP = {
            LISTIFY(OPENDECK_OUTPUT_LOGICAL_COUNT, OPENDECK_OUTPUT_REMAP_ENTRY, (, ))
        };
#undef OPENDECK_OUTPUT_REMAP_ENTRY
#endif
    };
}    // namespace opendeck::io::outputs
