/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

#include <array>

namespace io::indicators
{
    /**
     * @brief Test indicator backend that accepts requests without side effects.
     */
    class HwaTest : public Hwa
    {
        public:
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
         * @brief Records indicator-enable requests in tests.
         *
         * @param type Indicator to enable.
         */
        void on(Type type) override
        {
            set_state(type, true);
        }

        /**
         * @brief Records indicator-disable requests in tests.
         *
         * @param type Indicator to disable.
         */
        void off(Type type) override
        {
            set_state(type, false);
        }

        /**
         * @brief Returns the current test state for one indicator.
         *
         * @param type Indicator to inspect.
         *
         * @return `true` when the indicator is on, otherwise `false`.
         */
        bool is_on(Type type) const
        {
            return _states.at(static_cast<size_t>(type));
        }

        private:
        static constexpr size_t INDICATOR_COUNT = static_cast<size_t>(Type::All);

        void set_state(Type type, bool state)
        {
            if (type == Type::All)
            {
                _states.fill(state);
                return;
            }

            _states.at(static_cast<size_t>(type)) = state;
        }

        std::array<bool, INDICATOR_COUNT> _states = {};
    };
}    // namespace io::indicators
