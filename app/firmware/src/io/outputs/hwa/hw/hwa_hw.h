/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/instance/impl/deps.h"
#include "firmware/src/io/outputs/drivers/remap.h"

namespace opendeck::firmware::io::outputs
{
    /**
     * @brief Hardware-backed output backend that proxies to a low-level output driver.
     */
    class HwaHw : public Hwa
    {
        public:
        /**
         * @brief Constructs the backend around a low-level output driver.
         *
         * @param driver Low-level output driver implementation.
         */
        explicit HwaHw(Hwa& driver)
            : _driver(driver)
        {}

        /**
         * @brief Sets the level of one output.
         *
         * @param index output index to update.
         * @param level Output level percentage in the range [0, 100].
         */
        void set_level(size_t index, uint8_t level) override
        {
            _driver.set_level(Remap::physical(index), level);
        }

        private:
        Hwa& _driver;
    };
}    // namespace opendeck::firmware::io::outputs
