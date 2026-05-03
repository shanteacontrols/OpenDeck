/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common.h"

namespace opendeck::io::indicators
{
    /**
     * @brief Hardware abstraction for indicator outputs.
     */
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        /**
         * @brief Initializes the indicator backend.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        virtual bool init() = 0;

        /**
         * @brief Turns on the selected indicator output.
         *
         * @param type Indicator output to enable.
         */
        virtual void on(Type type) = 0;

        /**
         * @brief Turns off the selected indicator output.
         *
         * @param type Indicator output to disable.
         */
        virtual void off(Type type) = 0;
    };
}    // namespace opendeck::io::indicators
