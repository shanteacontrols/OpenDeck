/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "drivers/count.h"

#include <array>
#include <cstddef>
#include <zephyr/devicetree.h>

namespace opendeck::io::digital
{
    /**
     * @brief Resolves logical digital indices to the underlying physical scan order.
     */
    class Remap
    {
        public:
        /**
         * @brief Maps one logical button/digital input index to the physical index.
         *
         * @param logical Logical digital input index.
         *
         * @return Physical digital input index.
         */
        static constexpr size_t physical(size_t logical)
        {
#if OPENDECK_BUTTON_HAS_REMAP
            return INDEX_REMAP[logical];
#else
            return logical;
#endif
        }

        /**
         * @brief Maps one physical button/digital input index back to its logical index.
         *
         * @param physical Physical digital input index.
         *
         * @return Logical digital input index when remapped, otherwise the original index.
         */
        static constexpr size_t logical(size_t physical)
        {
#if OPENDECK_BUTTON_HAS_REMAP
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
#if OPENDECK_BUTTON_HAS_REMAP
#define OPENDECK_BUTTON_REMAP_ENTRY(index, _) DT_PROP_BY_IDX(DT_NODELABEL(opendeck_buttons), index_remap, index)
        inline static constexpr std::array<size_t, OPENDECK_BUTTON_LOGICAL_COUNT> INDEX_REMAP = {
            LISTIFY(OPENDECK_BUTTON_LOGICAL_COUNT, OPENDECK_BUTTON_REMAP_ENTRY, (, ))
        };
#undef OPENDECK_BUTTON_REMAP_ENTRY
#endif
    };
}    // namespace opendeck::io::digital
