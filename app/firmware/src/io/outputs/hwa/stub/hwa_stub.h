/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/shared/deps.h"

namespace opendeck::firmware::io::outputs
{
    /**
     * @brief Stub output backend that accepts requests without driving hardware.
     */
    class HwaStub : public Hwa
    {
        public:
        HwaStub() = default;

        /**
         * @brief Ignores output state updates.
         *
         * @param index output index to update.
         * @param level Output level percentage that would be applied.
         */
        void set_level([[maybe_unused]] size_t  index,
                       [[maybe_unused]] uint8_t level) override
        {
        }
    };
}    // namespace opendeck::firmware::io::outputs
