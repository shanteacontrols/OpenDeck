/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/shared/deps.h"

#include <gmock/gmock.h>

namespace opendeck::io::outputs
{
    /**
     * @brief Test OUTPUT backend with mockable state updates.
     */
    class HwaTest : public Hwa
    {
        public:
        HwaTest() = default;

        /**
         * @brief Flushes no additional state in the test backend.
         */
        void update() override
        {
        }

        MOCK_METHOD2(set_state, void(size_t index, Brightness brightness));

        /**
         * @brief Maps an RGB OUTPUT index and component to a physical output index.
         *
         * @param index RGB OUTPUT index to map.
         * @param component RGB component to map.
         *
         * @return Output index calculated as `index * 3 + component`.
         */
        size_t rgb_component_from_rgb(size_t index, RgbComponent component) override
        {
            return index * 3 + static_cast<uint8_t>(component);
        }

        /**
         * @brief Maps one physical output index to its RGB OUTPUT index.
         *
         * @param index Output index to map.
         *
         * @return RGB OUTPUT index calculated as `index / 3`.
         */
        size_t rgb_from_output(size_t index) override
        {
            return index / 3;
        }
    };
}    // namespace opendeck::io::outputs
