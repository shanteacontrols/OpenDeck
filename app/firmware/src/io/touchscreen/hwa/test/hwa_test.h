/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/touchscreen/deps.h"

namespace opendeck::io::touchscreen
{
    /**
     * @brief Test touchscreen backend that accepts traffic without touching hardware.
     */
    class HwaTest : public Hwa
    {
        public:
        HwaTest() = default;

        /**
         * @brief Initializes the test backend.
         *
         * @return Always `true`.
         */
        bool init() override
        {
            return true;
        }

        /**
         * @brief Deinitializes the test backend.
         *
         * @return Always `true`.
         */
        bool deinit() override
        {
            return true;
        }

        /**
         * @brief Accepts all write requests in tests.
         *
         * @param value Byte that would be transmitted.
         *
         * @return Always `true`.
         */
        bool write(uint8_t value) override
        {
            return true;
        }

        /**
         * @brief Returns no received data by default.
         *
         * @return Always `std::nullopt`.
         */
        std::optional<uint8_t> read() override
        {
            return {};
        }

        /**
         * @brief Reports that no allocatable interface is owned in tests.
         *
         * @param interface Interface to query.
         *
         * @return Always `false`.
         */
        bool allocated(io::common::Allocatable::Interface interface) override
        {
            return false;
        }
    };
}    // namespace opendeck::io::touchscreen
