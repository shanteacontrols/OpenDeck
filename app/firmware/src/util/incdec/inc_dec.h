/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inttypes.h>
#include <concepts>
#include <functional>
#include <stddef.h>

namespace opendeck::util
{
    /**
     * @brief Performs bounded increment and decrement operations for an integral range.
     *
     * @tparam T Integral value type.
     * @tparam MinValue Minimum value allowed in the range.
     * @tparam MaxValue Maximum value allowed in the range.
     */
    template<std::integral T, T MinValue, T MaxValue>
    class IncDec
    {
        public:
        /**
         * @brief Selects whether out-of-range updates clamp to the edge or wrap around.
         */
        enum class Type : uint8_t
        {
            Edge,
            Overflow,
        };

        /**
         * @brief Prevents instantiation of this static bounded increment/decrement helper.
         */
        IncDec() = delete;

        /**
         * @brief Increments a value by the requested number of steps.
         *
         * @param value Starting value.
         * @param steps Number of steps to add.
         * @param type Overflow behavior to use when the range limit is exceeded.
         *
         * @return Incremented value after applying the selected overflow behavior.
         */
        static T increment(T value, T steps, Type type)
        {
            if ((MaxValue - value) < steps)
            {
                if (type == Type::Edge)
                {
                    return MaxValue;
                }

                // handle overflow
                return (value + (steps % MaxValue)) % (MaxValue + 1);
            }

            return value + steps;
        }

        /**
         * @brief Decrements a value by the requested number of steps.
         *
         * @param value Starting value.
         * @param steps Number of steps to subtract.
         * @param type Underflow behavior to use when the range limit is exceeded.
         *
         * @return Decremented value after applying the selected underflow behavior.
         */
        static T decrement(T value, T steps, Type type)
        {
            if (value < steps)
            {
                if (type == Type::Edge)
                {
                    return MinValue;
                }

                // handle overflow
                return (value + MaxValue + 1 - (steps % MaxValue)) % (MaxValue + 1);
            }

            return value - steps;
        }
    };
}    // namespace opendeck::util
