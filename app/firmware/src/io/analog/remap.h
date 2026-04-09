/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "drivers/count.h"

#include <array>
#include <cstddef>
#include <zephyr/devicetree.h>

namespace io::analog
{
    /**
     * @brief Resolves logical analog indices to the underlying physical scan order.
     */
    class Remap
    {
        public:
        /**
         * @brief Maps one logical analog index to the physical index.
         *
         * @param logical Logical analog index.
         *
         * @return Physical analog index.
         */
        static constexpr size_t physical(size_t logical)
        {
#if OPENDECK_ANALOG_HAS_REMAP
            return INDEX_REMAP[logical];
#else
            return logical;
#endif
        }

        /**
         * @brief Maps one physical analog index back to its logical index.
         *
         * @param physical Physical analog index.
         *
         * @return Logical analog index when remapped, otherwise the original index.
         */
        static constexpr size_t logical(size_t physical)
        {
#if OPENDECK_ANALOG_HAS_REMAP
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
#if OPENDECK_ANALOG_HAS_REMAP
#define OPENDECK_ANALOG_REMAP_ENTRY(index, _) DT_PROP_BY_IDX(DT_NODELABEL(opendeck_analog), index_remap, index)
        inline static constexpr std::array<size_t, OPENDECK_ANALOG_LOGICAL_COUNT> INDEX_REMAP = {
            LISTIFY(OPENDECK_ANALOG_LOGICAL_COUNT, OPENDECK_ANALOG_REMAP_ENTRY, (, ))
        };
#undef OPENDECK_ANALOG_REMAP_ENTRY
#endif
    };
}    // namespace io::analog
