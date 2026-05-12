/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

namespace opendeck::io::outputs
{
    /**
     * @brief Stub OUTPUT backend that accepts requests without driving hardware.
     */
    class HwaStub : public Hwa
    {
        public:
        HwaStub() = default;

        /**
         * @brief Flushes no state because the stub backend keeps none.
         */
        void update() override
        {
        }

        /**
         * @brief Ignores OUTPUT state updates.
         *
         * @param index OUTPUT output index to update.
         * @param brightness Brightness value that would be applied.
         */
        void set_state([[maybe_unused]] size_t     index,
                       [[maybe_unused]] Brightness brightness) override
        {
        }

        /**
         * @brief Returns no RGB mapping.
         *
         * @param index OUTPUT output index to map.
         *
         * @return Always `0`.
         */
        size_t rgb_from_output([[maybe_unused]] size_t index) override
        {
            return 0;
        }

        /**
         * @brief Returns no RGB-component mapping.
         *
         * @param index RGB OUTPUT index to map.
         * @param component RGB component to map.
         *
         * @return Always `0`.
         */
        size_t rgb_component_from_rgb([[maybe_unused]] size_t       index,
                                      [[maybe_unused]] RgbComponent component) override
        {
            return 0;
        }
    };
}    // namespace opendeck::io::outputs
