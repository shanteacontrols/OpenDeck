/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <stddef.h>
#include <stdint.h>

namespace opendeck::io::i2c
{
    /**
     * @brief Confirms sensor value changes without delaying by time.
     *
     * A multi-component value is treated as one reading. A change is measured
     * by the largest absolute delta among its components.
     */
    template<size_t Size>
    class ValueFilter
    {
        public:
        using Values = std::array<uint16_t, Size>;

        enum class ConfirmationMode : uint8_t
        {
            Exact,
            Nearby,
        };

        /**
         * @brief Processes a raw value tuple.
         *
         * @param current Current raw value tuple.
         * @param idle_threshold Minimum component delta required to wake from idle.
         * @param confirmation_samples Matching changed samples required before accepting.
         * @param moving_threshold Minimum component delta required while movement is already active.
         *
         * @return `true` when the value was accepted.
         */
        bool update(const Values& current,
                    uint16_t      idle_threshold,
                    uint8_t       confirmation_samples,
                    uint16_t      moving_threshold = 0)
        {
            return update(current, idle_threshold, confirmation_samples, ConfirmationMode::Nearby, moving_threshold);
        }

        /**
         * @brief Processes a raw value tuple.
         *
         * @param current Current raw value tuple.
         * @param idle_threshold Minimum component delta required to wake from idle.
         * @param confirmation_samples Matching changed samples required before accepting.
         * @param confirmation_mode How pending samples are matched during confirmation.
         * @param moving_threshold Minimum component delta required while movement is already active.
         *
         * @return `true` when the value was accepted.
         */
        bool update(const Values&    current,
                    uint16_t         idle_threshold,
                    uint8_t          confirmation_samples,
                    ConfirmationMode confirmation_mode,
                    uint16_t         moving_threshold = 0)
        {
            const auto active_threshold = (_moving && (moving_threshold != 0)) ? moving_threshold : idle_threshold;

            if (!_has_value)
            {
                commit(current, false);
                return true;
            }

            if (diff(current, _value) < active_threshold)
            {
                clear_pending();
                _moving = false;
                return false;
            }

            if (_moving)
            {
                commit(current, true);
                return true;
            }

            if (!_has_pending || !matches_pending(current, idle_threshold, confirmation_mode))
            {
                _has_pending   = true;
                _pending_value = current;
                _pending_count = 1;

                if (confirmation_samples > 1)
                {
                    return false;
                }

                commit(current, true);
                return true;
            }

            _pending_count++;

            if (_pending_count < confirmation_samples)
            {
                return false;
            }

            commit(current, true);
            return true;
        }

        /**
         * @brief Clears all stored value and pending confirmation state.
         */
        void reset()
        {
            _has_value = false;
            _value     = {};
            _moving    = false;
            clear_pending();
        }

        /**
         * @brief Returns whether a value has been committed.
         *
         * @return `true` after the first committed value tuple.
         */
        bool has_value() const
        {
            return _has_value;
        }

        /**
         * @brief Returns the latest committed value tuple.
         *
         * @return Latest committed value tuple.
         */
        const Values& value() const
        {
            return _value;
        }

        private:
        /**
         * @brief Checks whether a raw tuple confirms the pending candidate.
         *
         * @param current Current raw value tuple.
         * @param threshold Maximum component delta used for nearby confirmation.
         * @param confirmation_mode Confirmation comparison mode.
         *
         * @return `true` if the current tuple matches the pending candidate.
         */
        bool matches_pending(const Values& current, uint16_t threshold, ConfirmationMode confirmation_mode) const
        {
            switch (confirmation_mode)
            {
            case ConfirmationMode::Exact:
                return current == _pending_value;

            case ConfirmationMode::Nearby:
            default:
                return diff(current, _pending_value) < threshold;
            }
        }

        /**
         * @brief Stores an accepted tuple and clears pending confirmation state.
         *
         * @param current Accepted value tuple.
         * @param moving Whether the filter should enter moving state.
         */
        void commit(const Values& current, bool moving)
        {
            _has_value = true;
            _value     = current;
            _moving    = moving;
            clear_pending();
        }

        /**
         * @brief Clears the pending candidate tuple used for confirmation.
         */
        void clear_pending()
        {
            _has_pending   = false;
            _pending_value = {};
            _pending_count = 0;
        }

        /**
         * @brief Calculates the largest component delta between two tuples.
         *
         * @param current Current value tuple.
         * @param previous Previous value tuple.
         *
         * @return Largest absolute component delta.
         */
        static uint16_t diff(const Values& current, const Values& previous)
        {
            uint16_t max_diff = 0;

            for (size_t i = 0; i < Size; i++)
            {
                const auto component_diff = current[i] > previous[i] ? current[i] - previous[i] : previous[i] - current[i];

                if (component_diff > max_diff)
                {
                    max_diff = component_diff;
                }
            }

            return max_diff;
        }

        bool    _has_value     = false;
        Values  _value         = {};
        bool    _moving        = false;
        bool    _has_pending   = false;
        Values  _pending_value = {};
        uint8_t _pending_count = 0;
    };
}    // namespace opendeck::io::i2c
