/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

#include "zlibs/utils/misc/bit.h"
#include "zlibs/utils/misc/numeric.h"

#include <algorithm>
#include <array>
#include <limits>
#include <zephyr/kernel.h>

namespace opendeck::io::analog
{
    /**
     * @brief Hardware-oriented analog filter for potentiometers, FSRs, and analog buttons.
     *
     * Continuously varying analog inputs use a lightweight path tuned for real-time response on
     * both dense native ADC scans and large multiplexed matrices: bucket-sized ADC deadbanding,
     * raw-sample reversal confirmation, smoothing against the last accepted ADC value, endpoint
     * assist, and light position publish gating for idle drift. Button-style analog inputs
     * bypass continuous-value filtering and use threshold-based analog-to-digital conversion with
     * hysteresis instead.
     */
    class FilterHw : public Filter
    {
        public:
        /**
         * @brief Constructs a filter for the board ADC profile.
         */
        FilterHw()
        {
            for (size_t i = 0; i < io::analog::Collection::size(); i++)
            {
                _last_sample_value[i]      = NO_VALUE;
                _last_value[i]             = NO_VALUE;
                _last_position_value[i]    = NO_VALUE;
                _pending_position_value[i] = NO_VALUE;
                _last_movement_time[i]     = NO_TIME;
            }
        }

        /**
         * @brief Returns the inactivity timeout after which motion context expires.
         *
         * Once this timeout elapses without a newly accepted movement, fast-mode state, pending
         * reversal confirmation, and pending idle-drift position confirmation are discarded. The
         * last published position is kept.
         *
         * @return Motion-context timeout in milliseconds.
         */
        static constexpr uint32_t motion_context_timeout_ms()
        {
            return MOTION_CONTEXT_TIMEOUT_MS;
        }

        /**
         * @brief Returns whether fast-filter mode is currently active for one analog input.
         *
         * The returned state is time-aware: latched fast-mode state is considered inactive after
         * the motion-context timeout elapses, even before the next sample arrives.
         *
         * @param index Analog input index to query.
         *
         * @return `true` when fast mode is still active for the input, otherwise `false`.
         */
        bool fast_mode_active(size_t index)
        {
            return (index < io::analog::Collection::size()) &&
                   has_recent_motion_context(index, k_uptime_get_32()) &&
                   fast_mode_latched(index);
        }

        /**
         * @brief Filters a raw analog reading and converts it into a stable position value.
         *
         * Continuously varying analog inputs move through the following states:
         * - Step 1: clamp the raw reading to the expected minimum/maximum range
         * - Step 2: ignore tiny fluctuations by requiring roughly one position step worth of ADC
         *           movement before accepting a new sample; opposite-direction motion uses a
         *           wider threshold away from endpoints, while endpoint-assist samples bypass
         *           this deadband so edge snaps can still complete cleanly
         * - Step 3: if previously accepted motion has gone stale, drop fast-mode and pending
         *           confirmation state while preserving the last published value
         * - Step 4: track fast-mode state from consecutive-sample ADC delta magnitude so the
         *           filter can still expose active-motion state
         * - Step 5: confirm direction reversals from consecutive raw samples before accepting
         *           them; endpoint-assist samples bypass reversal confirmation because they
         *           already snap directly to the endpoint value
         * - Step 6: smooth the accepted sample by blending it with the last accepted ADC value
         * - Step 7: convert the filtered ADC value into a position value
         * - Step 8: apply position publish rules
         *   - if the position value is unchanged, suppress it
         *   - if the raw sample already reaches the lower/upper endpoint bucket and the current
         *     published value is one step away from that edge, snap to the endpoint immediately
         *   - after motion context expires, require one repeated sample before publishing a new
         *     idle value so rare baseline drift does not emit immediately
         *   - ignore tiny opposite-direction reversals that would briefly bounce the position
         * - Step 9: commit the new movement direction, filtered ADC value, position value, and
         *           motion timestamp as the current state
         *
         * @param index Analog input index being processed.
         * @param descriptor Runtime descriptor containing the current reading and configuration.
         *
         * @return `true` when the filtered reading should be processed further. Returns `false`
         *         for discarded samples and for out-of-range input indices.
         */
        bool is_filtered(size_t index, Descriptor& descriptor) override
        {
            if (index >= io::analog::Collection::size())
            {
                return false;
            }

            const uint16_t adc_min_value = lower_offset_raw(descriptor.lower_offset);
            const uint16_t adc_max_value = upper_offset_raw(descriptor.upper_offset);

            descriptor.value = zlibs::utils::misc::constrain(descriptor.value,
                                                             adc_min_value,
                                                             adc_max_value);

            // avoid full filtering in this case for faster response
            if (descriptor.type == Type::Button)
            {
                return is_button_filtered(index, descriptor);
            }

            const auto motion_context = prepare_motion_context(index);
            auto       input_state    = prepare_continuous_input_state(index,
                                                                       descriptor,
                                                                       adc_min_value,
                                                                       adc_max_value);

            if (!apply_adc_stage(index,
                                 input_state,
                                 descriptor.value))
            {
                return false;
            }

            return apply_position_stage(index,
                                        motion_context,
                                        input_state,
                                        adc_min_value,
                                        adc_max_value,
                                        descriptor);
        }

        /**
         * @brief Resets filter state for one analog input.
         *
         * This clears fast-mode state, pending position confirmation, raw ADC history, and cached
         * last-published position value for the specified input.
         *
         * @param index Analog input index to reset.
         */
        void reset(size_t index) override
        {
            if (index >= io::analog::Collection::size())
            {
                return;
            }

            clear_motion_context(index);

            _last_movement_time[index]     = NO_TIME;
            _last_sample_value[index]      = NO_VALUE;
            _last_value[index]             = NO_VALUE;
            _last_position_value[index]    = NO_VALUE;
            _pending_position_value[index] = NO_VALUE;
        }

        private:
        /**
         * @brief Endpoint-assist decision for one filtered sample.
         */
        struct EndpointAssistState
        {
            bool enabled = false;
            bool upper   = false;
            bool lower   = false;
        };

        /**
         * @brief Motion-context timing state prepared for one sample.
         */
        struct MotionContextState
        {
            uint32_t now          = NO_TIME;
            bool     idle_context = true;
        };

        /**
         * @brief Cached per-sample state for one continuous analog input.
         */
        struct ContinuousInputState
        {
            bool                has_last_filtered_value = false;
            bool                has_last_position_value = false;
            uint16_t            last_filtered_value     = 0;
            uint16_t            old_position_value      = NO_VALUE;
            uint16_t            raw_position_value      = 0;
            uint16_t            min_step_diff           = 1;
            uint16_t            endpoint_range          = ENDPOINT_ASSIST_POSITION_RANGE;
            bool                direction               = false;
            EndpointAssistState endpoint_assist         = {};
        };

        static constexpr uint8_t  FAST_FILTER_STEP_MULTIPLIER               = 2;
        static constexpr uint8_t  FAST_FILTER_ENTER_MULTIPLIER              = 2;
        static constexpr uint8_t  FAST_FILTER_EXIT_MULTIPLIER               = 1;
        static constexpr uint8_t  FAST_FILTER_ENTER_SAMPLE_COUNT            = 2;
        static constexpr uint32_t MOTION_CONTEXT_TIMEOUT_MS                 = 100;
        static constexpr uint8_t  ENDPOINT_ASSIST_POSITION_RANGE            = 1;
        static constexpr uint8_t  POSITION_DIRECTION_CHANGE_THRESHOLD       = 3;
        static constexpr uint8_t  IDLE_WAKE_POSITION_THRESHOLD              = 4;
        static constexpr uint8_t  IDLE_POSITION_CHANGE_SAMPLE_COUNT         = 2;
        static constexpr uint32_t PERCENTAGE_DIVISOR                        = 100;
        static constexpr uint8_t  FAST_FILTER_DIRECTION_CHANGE_SAMPLE_COUNT = 2;
        static constexpr size_t   BITS_PER_STORAGE_UNIT                     = 8;
        static constexpr uint8_t  EMA_FILTER_PERCENT                        = 50;
        static constexpr uint16_t NO_VALUE                                  = 0xFFFF;
        static constexpr uint32_t NO_TIME                                   = std::numeric_limits<uint32_t>::max();
        static constexpr size_t   STORAGE_SIZE                              = io::analog::Collection::size() ? io::analog::Collection::size() : 1;
        static constexpr size_t   BIT_STORAGE_SIZE                          = (STORAGE_SIZE + BITS_PER_STORAGE_UNIT - 1) / BITS_PER_STORAGE_UNIT;

        std::array<uint8_t, STORAGE_SIZE>     _direction_change_counter     = {};
        std::array<uint8_t, STORAGE_SIZE>     _fast_mode_entry_counter      = {};
        std::array<uint8_t, STORAGE_SIZE>     _idle_position_change_counter = {};
        std::array<uint32_t, STORAGE_SIZE>    _last_movement_time           = {};
        std::array<uint8_t, BIT_STORAGE_SIZE> _fast_mode                    = {};
        std::array<uint8_t, BIT_STORAGE_SIZE> _last_direction               = {};
        std::array<uint16_t, STORAGE_SIZE>    _last_sample_value            = {};
        std::array<uint16_t, STORAGE_SIZE>    _last_value                   = {};
        std::array<uint16_t, STORAGE_SIZE>    _last_position_value          = {};
        std::array<uint16_t, STORAGE_SIZE>    _pending_position_value       = {};

        /**
         * @brief Stores the last movement direction for one analog input.
         *
         * @param index Analog input index to update.
         * @param state `true` for rising direction, `false` for falling direction.
         */
        void set_last_direction(size_t index, bool state)
        {
            const auto location = io::common::bit_storage_location<BITS_PER_STORAGE_UNIT>(index);

            zlibs::utils::misc::bit_write(_last_direction[location.array_index], location.bit_index, state);
        }

        /**
         * @brief Stores whether one analog input is currently in fast-filter mode.
         *
         * @param index Analog input index to update.
         * @param state `true` when fast mode is active, otherwise `false`.
         */
        void set_fast_mode(size_t index, bool state)
        {
            const auto location = io::common::bit_storage_location<BITS_PER_STORAGE_UNIT>(index);

            zlibs::utils::misc::bit_write(_fast_mode[location.array_index], location.bit_index, state);
        }

        /**
         * @brief Returns the last stored movement direction for one analog input.
         *
         * @param index Analog input index to query.
         *
         * @return `true` for rising direction, `false` for falling direction.
         */
        bool last_direction(size_t index)
        {
            const auto location = io::common::bit_storage_location<BITS_PER_STORAGE_UNIT>(index);

            return zlibs::utils::misc::bit_read(_last_direction[location.array_index], location.bit_index);
        }

        /**
         * @brief Returns the raw stored fast-mode latch for one analog input.
         *
         * This helper intentionally exposes only the cached bit state. Unlike
         * `fast_mode_active()`, it does not apply the motion-context timeout and is used only
         * inside the filter state machine after stale motion context has been cleared.
         *
         * @param index Analog input index to query.
         *
         * @return `true` when the fast-mode latch bit is set, otherwise `false`.
         */
        bool fast_mode_latched(size_t index)
        {
            const auto location = io::common::bit_storage_location<BITS_PER_STORAGE_UNIT>(index);

            return zlibs::utils::misc::bit_read(_fast_mode[location.array_index], location.bit_index);
        }

        /**
         * @brief Returns whether the cached motion context is still valid for one analog input.
         *
         * @param index Analog input index to query.
         * @param now   Current uptime in milliseconds.
         *
         * @return `true` when recent motion context is still valid, otherwise `false`.
         */
        bool has_recent_motion_context(size_t   index,
                                       uint32_t now)
        {
            return (_last_movement_time[index] != NO_TIME) &&
                   ((now - _last_movement_time[index]) <= MOTION_CONTEXT_TIMEOUT_MS);
        }

        /**
         * @brief Returns whether previously accepted motion state has timed out.
         *
         * Returns `false` for inputs that have never established motion context, so this is not
         * simply the logical complement of `has_recent_motion_context()`.
         *
         * @param index Analog input index to query.
         * @param now   Current uptime in milliseconds.
         *
         * @return `true` when previously accepted motion context has expired.
         */
        bool motion_context_expired(size_t   index,
                                    uint32_t now)
        {
            return (_last_movement_time[index] != NO_TIME) &&
                   ((now - _last_movement_time[index]) > MOTION_CONTEXT_TIMEOUT_MS);
        }

        /**
         * @brief Clears stale motion context for one input when its timeout expires.
         *
         * @param index Analog input index to update.
         * @param now   Current uptime in milliseconds.
         */
        void expire_motion_context_if_needed(size_t   index,
                                             uint32_t now)
        {
            if (motion_context_expired(index, now))
            {
                clear_motion_context(index);
                _last_movement_time[index] = NO_TIME;
            }
        }

        /**
         * @brief Clears motion-tracking state while keeping the last published value intact.
         *
         * This is used when motion context expires so the next sample starts a fresh gesture
         * instead of continuing stale fast-mode, reversal-confirmation, or pending position
         * confirmation state.
         *
         * @param index Analog input index to reset.
         */
        void clear_motion_context(size_t index)
        {
            clear_direction_change_state(index);
            clear_pending_position_state(index);
            _fast_mode_entry_counter[index] = 0;
            set_fast_mode(index, false);
        }

        /**
         * @brief Clears in-progress direction-change confirmation state for one input.
         *
         * @param index Analog input index to update.
         */
        void clear_direction_change_state(size_t index)
        {
            _direction_change_counter[index] = 0;
        }

        /**
         * @brief Clears pending position publish confirmation state for one input.
         *
         * @param index Analog input index to update.
         */
        void clear_pending_position_state(size_t index)
        {
            _idle_position_change_counter[index] = 0;
            _pending_position_value[index]       = NO_VALUE;
        }

        /**
         * @brief Returns the absolute difference between two unsigned 16-bit values.
         *
         * @param lhs First value.
         * @param rhs Second value.
         *
         * @return Absolute distance between `lhs` and `rhs`.
         */
        static constexpr uint16_t abs_diff(uint16_t lhs,
                                           uint16_t rhs)
        {
            return lhs >= rhs ? static_cast<uint16_t>(lhs - rhs)
                              : static_cast<uint16_t>(rhs - lhs);
        }

        /**
         * @brief Applies EMA smoothing against the last accepted ADC value.
         *
         * The first accepted sample is passed through unchanged so stable startup values do not
         * walk from zero toward the real reading.
         *
         * @param value Current accepted ADC sample.
         * @param has_last_filtered_value Whether a previous filtered ADC value exists.
         * @param last_filtered_value Last accepted filtered ADC value.
         *
         * @return Smoothed ADC value.
         */
        static constexpr uint16_t apply_ema(uint16_t value,
                                            bool     has_last_filtered_value,
                                            uint16_t last_filtered_value)
        {
            if (!has_last_filtered_value)
            {
                return value;
            }

            return static_cast<uint16_t>((static_cast<uint32_t>(EMA_FILTER_PERCENT) * value +
                                          static_cast<uint32_t>(PERCENTAGE_DIVISOR - EMA_FILTER_PERCENT) * last_filtered_value) /
                                         PERCENTAGE_DIVISOR);
        }

        /**
         * @brief Maps one ADC sample into filter position space.
         *
         * @param value Input ADC sample.
         * @param adc_min_value Lower bound of the active ADC range.
         * @param adc_max_value Upper bound of the active ADC range.
         * @return Position-space value corresponding to the ADC sample.
         */
        static constexpr uint16_t adc_to_position_value(uint16_t value,
                                                        uint16_t adc_min_value,
                                                        uint16_t adc_max_value)
        {
            return static_cast<uint16_t>(zlibs::utils::misc::map_range(static_cast<uint32_t>(value),
                                                                       static_cast<uint32_t>(adc_min_value),
                                                                       static_cast<uint32_t>(adc_max_value),
                                                                       static_cast<uint32_t>(0),
                                                                       static_cast<uint32_t>(POSITION_MAX_VALUE)));
        }

        /**
         * @brief Prepares motion-context timing state for one sample.
         *
         * @param index Analog input index being processed.
         *
         * @return Motion-context state for the current sample.
         */
        MotionContextState prepare_motion_context(size_t index)
        {
            MotionContextState state = {};

            if (_last_movement_time[index] != NO_TIME)
            {
                state.now          = k_uptime_get_32();
                state.idle_context = !has_recent_motion_context(index, state.now);
                expire_motion_context_if_needed(index, state.now);
            }

            return state;
        }

        /**
         * @brief Computes cached per-sample state for one continuous analog input.
         *
         * @param index Analog input index being processed.
         * @param descriptor Runtime descriptor containing the current reading and configuration.
         * @param adc_min_value Lower bound of the active ADC range.
         * @param adc_max_value Upper bound of the active ADC range.
         *
         * @return Cached state for the current sample.
         */
        ContinuousInputState prepare_continuous_input_state(size_t            index,
                                                            const Descriptor& descriptor,
                                                            uint16_t          adc_min_value,
                                                            uint16_t          adc_max_value)
        {
            ContinuousInputState state              = {};
            const bool           has_last_sample    = _last_sample_value[index] != NO_VALUE;
            const auto           raw_position_value = adc_to_position_value(descriptor.value,
                                                                            adc_min_value,
                                                                            adc_max_value);
            const bool           raw_at_endpoint    = (raw_position_value == 0) || (raw_position_value == POSITION_MAX_VALUE);
            const auto           steps              = POSITION_STEP_COUNT;
            const auto           base_step_diff     = step_diff(adc_min_value,
                                                                adc_max_value,
                                                                steps);
            const auto           adc_delta          = has_last_sample ? abs_diff(descriptor.value, _last_sample_value[index]) : static_cast<uint16_t>(0);
            const auto           fast_enter_thres   = static_cast<uint16_t>(base_step_diff * FAST_FILTER_ENTER_MULTIPLIER);
            const auto           fast_exit_thres    = static_cast<uint16_t>(base_step_diff * FAST_FILTER_EXIT_MULTIPLIER);

            state.has_last_filtered_value = _last_value[index] != NO_VALUE;
            state.has_last_position_value = _last_position_value[index] != NO_VALUE;
            state.last_filtered_value     = state.has_last_filtered_value ? _last_value[index] : descriptor.value;
            state.old_position_value      = state.has_last_position_value ? _last_position_value[index] : NO_VALUE;
            state.raw_position_value      = raw_position_value;
            state.direction               = descriptor.value >= state.last_filtered_value;
            state.endpoint_range          = endpoint_assist_range();
            state.endpoint_assist         = endpoint_assist_state(state.has_last_position_value,
                                                                  state.old_position_value,
                                                                  raw_position_value,
                                                                  state.endpoint_range);

            update_fast_filter_state(index,
                                     has_last_sample,
                                     adc_delta,
                                     fast_enter_thres,
                                     fast_exit_thres);

            state.min_step_diff = min_step_diff(index,
                                                state.direction,
                                                raw_at_endpoint,
                                                state.has_last_position_value,
                                                state.old_position_value,
                                                base_step_diff);

            return state;
        }

        /**
         * @brief Applies ADC-domain filtering and smoothing for one continuous sample.
         *
         * @param index Analog input index being processed.
         * @param state Cached per-sample state.
         * @param value Current ADC sample. Replaced by the accepted smoothed ADC value.
         *
         * @return `true` when the sample survives ADC-domain filtering.
         */
        bool apply_adc_stage(size_t                      index,
                             const ContinuousInputState& state,
                             uint16_t&                   value)
        {
            _last_sample_value[index] = value;

            if (reject_deadband_sample(index,
                                       state.has_last_filtered_value,
                                       value,
                                       state.last_filtered_value,
                                       state.endpoint_assist.enabled,
                                       state.min_step_diff))
            {
                return false;
            }

            // Raw-sample reversal confirmation may reject brief opposite-direction blips.
            if (!apply_adc_filter(index,
                                  state.endpoint_assist.enabled,
                                  state.direction))
            {
                return false;
            }

            value = apply_ema(value,
                              state.has_last_filtered_value,
                              state.last_filtered_value);

            return true;
        }

        /**
         * @brief Applies position publish policy and commits one accepted continuous sample.
         *
         * @param index Analog input index being processed.
         * @param motion_context Motion-context state prepared for the current sample.
         * @param state Cached per-sample state. Direction may be revised by endpoint assist.
         * @param adc_min_value Lower bound of the active ADC range.
         * @param adc_max_value Upper bound of the active ADC range.
         * @param descriptor Runtime descriptor containing the current reading and configuration.
         *
         * @return `true` when the current sample should be published.
         */
        bool apply_position_stage(size_t                    index,
                                  const MotionContextState& motion_context,
                                  ContinuousInputState&     state,
                                  uint16_t                  adc_min_value,
                                  uint16_t                  adc_max_value,
                                  Descriptor&               descriptor)
        {
            auto position_value = adc_to_position_value(descriptor.value,
                                                        adc_min_value,
                                                        adc_max_value);

            if (!apply_position_publish_policy(index,
                                               state.endpoint_assist,
                                               motion_context.idle_context,
                                               descriptor.type != Type::Fsr,
                                               state.has_last_position_value,
                                               state.old_position_value,
                                               state.raw_position_value,
                                               state.endpoint_range,
                                               state.direction,
                                               position_value))
            {
                return false;
            }

            const auto now = motion_context.now == NO_TIME ? k_uptime_get_32()
                                                           : motion_context.now;

            commit_filtered_state(index,
                                  state.direction,
                                  descriptor.value,
                                  position_value,
                                  now);

            if (descriptor.type == Type::Fsr)
            {
                descriptor.value = scale_fsr_value(descriptor.value);
            }
            else
            {
                descriptor.value = position_value;
            }

            return true;
        }

        /**
         * @brief Updates fast-mode state from the latest raw ADC delta.
         *
         * @param index Analog input index being processed.
         * @param has_last_sample_value Whether a previous raw sample exists.
         * @param adc_delta Consecutive-sample raw ADC delta.
         * @param fast_enter_threshold Minimum delta required to enter fast mode.
         * @param fast_exit_threshold Minimum delta required to stay in fast mode.
         *
         */
        void update_fast_filter_state(size_t   index,
                                      bool     has_last_sample_value,
                                      uint16_t adc_delta,
                                      uint16_t fast_enter_threshold,
                                      uint16_t fast_exit_threshold)
        {
            if (!has_last_sample_value)
            {
                return;
            }

            if (fast_mode_latched(index))
            {
                const bool fast_filter = adc_delta >= fast_exit_threshold;

                set_fast_mode(index, fast_filter);

                if (!fast_filter)
                {
                    _fast_mode_entry_counter[index] = 0;
                }

                return;
            }

            if (adc_delta >= fast_enter_threshold)
            {
                _fast_mode_entry_counter[index]++;

                if (_fast_mode_entry_counter[index] >= FAST_FILTER_ENTER_SAMPLE_COUNT)
                {
                    set_fast_mode(index, true);
                    _fast_mode_entry_counter[index] = 0;
                    return;
                }
            }
            else
            {
                _fast_mode_entry_counter[index] = 0;
            }
        }

        /**
         * @brief Returns endpoint-assist flags for the current raw sample.
         *
         * @param has_last_position_value Whether a previous position value exists.
         * @param old_position_value Last published position value.
         * @param raw_position_value Position value corresponding to the unclamped raw ADC sample.
         * @param endpoint_range How close a value must be to min/max before it snaps.
         *
         * @return Endpoint-assist state for the current sample.
         */
        EndpointAssistState endpoint_assist_state(bool     has_last_position_value,
                                                  uint16_t old_position_value,
                                                  uint16_t raw_position_value,
                                                  uint16_t endpoint_range) const
        {
            const auto upper_endpoint_floor  = POSITION_MAX_VALUE > endpoint_range ? static_cast<uint16_t>(POSITION_MAX_VALUE - endpoint_range) : static_cast<uint16_t>(0);
            const bool near_upper_endpoint   = has_last_position_value && (old_position_value >= upper_endpoint_floor);
            const bool near_lower_endpoint   = has_last_position_value && (old_position_value <= endpoint_range);
            const bool upper_endpoint_assist = near_upper_endpoint &&
                                               (raw_position_value == POSITION_MAX_VALUE) &&
                                               (old_position_value < POSITION_MAX_VALUE);
            const bool lower_endpoint_assist = near_lower_endpoint &&
                                               (raw_position_value == 0) &&
                                               (old_position_value > 0);

            return {
                .enabled = upper_endpoint_assist || lower_endpoint_assist,
                .upper   = upper_endpoint_assist,
                .lower   = lower_endpoint_assist,
            };
        }

        /**
         * @brief Returns how close a value must be to min/max before it snaps.
         *
         * @return Distance from min/max that still counts as the endpoint.
         */
        uint16_t endpoint_assist_range() const
        {
            return ENDPOINT_ASSIST_POSITION_RANGE;
        }

        /**
         * @brief Returns the ADC deadband threshold for the current sample.
         *
         * @param index Analog input index being processed.
         * @param direction Current movement direction inferred from ADC values.
         * @param raw_at_position_endpoint Whether the raw sample already maps to an endpoint.
         * @param has_last_position_value Whether a previous position value exists.
         * @param old_position_value Last published position value.
         * @param base_step_diff ADC delta corresponding to one position step.
         *
         * @return ADC deadband threshold for the current sample.
         */
        uint16_t min_step_diff(size_t   index,
                               bool     direction,
                               bool     raw_at_position_endpoint,
                               bool     has_last_position_value,
                               uint16_t old_position_value,
                               uint16_t base_step_diff)
        {
            if ((direction != last_direction(index)) &&
                has_last_position_value &&
                !raw_at_position_endpoint &&
                ((old_position_value != 0) && (old_position_value != POSITION_MAX_VALUE)))
            {
                return static_cast<uint16_t>(base_step_diff * FAST_FILTER_STEP_MULTIPLIER);
            }

            return base_step_diff;
        }

        /**
         * @brief Rejects one ADC sample when it falls inside the current deadband.
         *
         * Deadband rejection also clears in-progress reversal, idle-publish, and fast-mode entry
         * state. In the simplified filter this means sub-deadband movement is treated as loss of
         * sustained motion, so later large deltas must build fresh confirmation again.
         *
         * Endpoint-assist samples intentionally bypass this rejection so the filter can snap from
         * `1 -> 0` or `max - 1 -> max` even when the final ADC movement is smaller than the
         * normal deadband.
         *
         * @param index Analog input index being processed.
         * @param has_last_filtered_value Whether a previous filtered ADC value exists.
         * @param value Current ADC sample.
         * @param last_filtered_value Last accepted filtered ADC value.
         * @param endpoint_assist Whether endpoint assist is active for the current sample.
         * @param min_step_diff ADC deadband threshold for the current sample.
         *
         * @return `true` when the sample was rejected by the deadband.
         */
        bool reject_deadband_sample(size_t   index,
                                    bool     has_last_filtered_value,
                                    uint16_t value,
                                    uint16_t last_filtered_value,
                                    bool     endpoint_assist,
                                    uint16_t min_step_diff)
        {
            if (!has_last_filtered_value ||
                endpoint_assist ||
                (abs_diff(value, last_filtered_value) >= min_step_diff))
            {
                return false;
            }

            clear_direction_change_state(index);
            clear_pending_position_state(index);
            _fast_mode_entry_counter[index] = 0;
            set_fast_mode(index, false);

            return true;
        }

        /**
         * @brief Applies ADC-domain filtering for the current sample.
         *
         * @param index Analog input index being processed.
         * @param endpoint_assist Whether endpoint assist is active for the current sample.
         * @param direction Current movement direction.
         *
         * @return `true` when the sample survives ADC-domain filtering.
         */
        bool apply_adc_filter(size_t index,
                              bool   endpoint_assist,
                              bool   direction)
        {
            if (endpoint_assist)
            {
                clear_direction_change_state(index);
                return true;
            }

            if (direction != last_direction(index))
            {
                _direction_change_counter[index]++;

                if (_direction_change_counter[index] < FAST_FILTER_DIRECTION_CHANGE_SAMPLE_COUNT)
                {
                    return false;
                }
            }
            else
            {
                clear_direction_change_state(index);
            }

            return true;
        }

        /**
         * @brief Applies position publish rules for the current sample.
         *
         * @param index Analog input index being processed.
         * @param endpoint_assist Endpoint-assist state for the current sample.
         * @param idle_context Whether recent motion context is absent.
         * @param apply_idle_wake_gate Whether tiny idle movements should be blocked.
         * @param has_last_position_value Whether a previous position value exists.
         * @param old_position_value Last published position value.
         * @param raw_position_value Current raw position before EMA smoothing.
         * @param endpoint_range How close a value must be to min/max before it snaps.
         * @param direction Current movement direction; may be updated by endpoint assist.
         * @param position_value Current position value; may be replaced by endpoint assist.
         *
         * @return `true` when the current position value should be published.
         */
        bool apply_position_publish_policy(size_t                     index,
                                           const EndpointAssistState& endpoint_assist,
                                           bool                       idle_context,
                                           bool                       apply_idle_wake_gate,
                                           bool                       has_last_position_value,
                                           uint16_t                   old_position_value,
                                           uint16_t                   raw_position_value,
                                           uint16_t                   endpoint_range,
                                           bool&                      direction,
                                           uint16_t&                  position_value)
        {
            if (endpoint_assist.upper)
            {
                position_value = POSITION_MAX_VALUE;
                direction      = true;
            }
            else if (endpoint_assist.lower)
            {
                position_value = 0;
                direction      = false;
            }

            if (endpoint_hold_active(has_last_position_value,
                                     old_position_value,
                                     position_value,
                                     endpoint_range))
            {
                clear_pending_position_state(index);
                return false;
            }

            if (has_last_position_value && (position_value == old_position_value))
            {
                clear_pending_position_state(index);
                return false;
            }

            if (!endpoint_assist.enabled && idle_context && has_last_position_value)
            {
                // While motion context is expired, ignore tiny position changes entirely.
                // Larger changes still need repeated-sample confirmation below; once accepted,
                // commit_filtered_state() refreshes motion context and this idle gate is skipped.
                if (apply_idle_wake_gate &&
                    (abs_diff(raw_position_value, old_position_value) < IDLE_WAKE_POSITION_THRESHOLD))
                {
                    clear_pending_position_state(index);
                    return false;
                }

                if (_pending_position_value[index] != position_value)
                {
                    _pending_position_value[index]       = position_value;
                    _idle_position_change_counter[index] = 1;
                    return false;
                }

                _idle_position_change_counter[index]++;

                if (_idle_position_change_counter[index] < IDLE_POSITION_CHANGE_SAMPLE_COUNT)
                {
                    return false;
                }
            }

            if (!endpoint_assist.enabled &&
                has_last_position_value &&
                (direction != last_direction(index)) &&
                (abs_diff(position_value, old_position_value) < POSITION_DIRECTION_CHANGE_THRESHOLD))
            {
                clear_pending_position_state(index);
                return false;
            }

            return true;
        }

        /**
         * @brief Returns whether a sample should stay latched to a previously published endpoint.
         *
         * Endpoint assist snaps a value to exact min/max when the raw reading reaches the edge.
         * Without this hold check, edge noise can immediately publish a small value next to the
         * endpoint and then snap back again on the next sample.
         *
         * @param has_last_position_value Whether a previous position value exists.
         * @param old_position_value Last published position value.
         * @param position_value Current position value.
         * @param endpoint_range How close a value must be to min/max before it snaps.
         *
         * @return `true` when the previous endpoint value should remain published.
         */
        bool endpoint_hold_active(bool     has_last_position_value,
                                  uint16_t old_position_value,
                                  uint16_t position_value,
                                  uint16_t endpoint_range) const
        {
            const auto hold_range = static_cast<uint16_t>(std::min<uint32_t>(
                POSITION_MAX_VALUE,
                static_cast<uint32_t>(endpoint_range) * FAST_FILTER_STEP_MULTIPLIER));

            return has_last_position_value &&
                   (((old_position_value == 0) && (position_value <= hold_range)) ||
                    ((old_position_value == POSITION_MAX_VALUE) && (position_value >= (POSITION_MAX_VALUE - hold_range))));
        }

        /**
         * @brief Commits one accepted filtered sample into per-input state.
         *
         * @param index Analog input index being processed.
         * @param direction Accepted movement direction.
         * @param value Accepted filtered ADC value.
         * @param position_value Accepted position value.
         * @param now Current uptime in milliseconds.
         */
        void commit_filtered_state(size_t   index,
                                   bool     direction,
                                   uint16_t value,
                                   uint16_t position_value,
                                   uint32_t now)
        {
            set_last_direction(index, direction);
            clear_direction_change_state(index);
            clear_pending_position_state(index);
            _last_value[index]          = value;
            _last_position_value[index] = position_value;
            _last_movement_time[index]  = now;
        }

        /**
         * @brief Returns the minimum ADC delta corresponding to one position step.
         *
         * The value is derived from the effective ADC span after offsets and the fixed filter
         * position scale. It is clamped to at least one ADC step.
         *
         * @param adc_min_value Lower bound of the active ADC range.
         * @param adc_max_value Upper bound of the active ADC range.
         * @param position_steps Position steps used by the analog motion filter.
         *
         * @return Minimum ADC delta corresponding to one position step.
         */
        uint16_t step_diff(uint16_t adc_min_value,
                           uint16_t adc_max_value,
                           uint16_t position_steps) const
        {
            if (position_steps == 0U)
            {
                return 1U;
            }

            const auto adc_span  = static_cast<uint32_t>(adc_max_value) - static_cast<uint32_t>(adc_min_value);
            const auto step_diff = adc_span / static_cast<uint32_t>(position_steps);

            return step_diff == 0U ? 1U : static_cast<uint16_t>(step_diff);
        }

        /**
         * @brief Converts a lower offset percentage into a raw ADC threshold.
         *
         * Offsets are applied relative to the configured usable ADC span.
         *
         * @param percentage Lower offset percentage.
         *
         * @return Raw ADC threshold for the lower bound.
         */
        static constexpr uint32_t lower_offset_raw(uint8_t percentage)
        {
            if (percentage != 0)
            {
                const auto adc_span = static_cast<uint32_t>(AdcConfig::ADC_MAX_VALUE) - static_cast<uint32_t>(AdcConfig::ADC_MIN_VALUE);

                return static_cast<uint32_t>(AdcConfig::ADC_MIN_VALUE) +
                       ((adc_span * percentage) / PERCENTAGE_DIVISOR);
            }

            return AdcConfig::ADC_MIN_VALUE;
        }

        /**
         * @brief Converts an upper offset percentage into a raw ADC threshold.
         *
         * Offsets are applied relative to the configured usable ADC span.
         *
         * @param percentage Upper offset percentage.
         *
         * @return Raw ADC threshold for the upper bound.
         */
        static constexpr uint32_t upper_offset_raw(uint8_t percentage)
        {
            if (percentage != 0)
            {
                const auto adc_span = static_cast<uint32_t>(AdcConfig::ADC_MAX_VALUE) - static_cast<uint32_t>(AdcConfig::ADC_MIN_VALUE);

                return static_cast<uint32_t>(AdcConfig::ADC_MAX_VALUE) -
                       ((adc_span * percentage) / PERCENTAGE_DIVISOR);
            }

            return AdcConfig::ADC_MAX_VALUE;
        }

        /**
         * @brief Scales one FSR value into filter position space.
         *
         * @param value Raw FSR reading.
         *
         * @return Scaled FSR value.
         */
        uint16_t scale_fsr_value(uint16_t value) const
        {
            return static_cast<uint16_t>(zlibs::utils::misc::map_range(
                zlibs::utils::misc::constrain(static_cast<uint32_t>(value),
                                              static_cast<uint32_t>(AdcConfig::FSR_MIN_VALUE),
                                              static_cast<uint32_t>(AdcConfig::FSR_MAX_VALUE)),
                static_cast<uint32_t>(AdcConfig::FSR_MIN_VALUE),
                static_cast<uint32_t>(AdcConfig::FSR_MAX_VALUE),
                static_cast<uint32_t>(0),
                static_cast<uint32_t>(POSITION_MAX_VALUE)));
        }

        /**
         * @brief Returns the lower button threshold.
         *
         * @return Raw ADC threshold below which the button is treated as released.
         */
        uint16_t digital_value_threshold_off() const
        {
            return AdcConfig::DIGITAL_VALUE_THRESHOLD_OFF;
        }

        /**
         * @brief Returns the upper button threshold.
         *
         * @return Raw ADC threshold above which the button is treated as pressed.
         */
        uint16_t digital_value_threshold_on() const
        {
            return AdcConfig::DIGITAL_VALUE_THRESHOLD_ON;
        }

        /**
         * @brief Applies button-style filtering for analog inputs configured as buttons.
         *
         * The analog reading is converted into a digital state using ADC hysteresis. The first
         * resolved released state only seeds internal state, while the
         * first resolved pressed state is emitted immediately so startup-held buttons behave like
         * the digital button path.
         *
         * @param index Analog input index being processed.
         * @param descriptor Runtime descriptor containing the current reading and configuration.
         *
         * @return `true` when the hysteresis-converted button state changed and should be processed,
         *         otherwise `false`.
         */
        bool is_button_filtered(size_t index, Descriptor& descriptor)
        {
            bool new_value = false;

            if (descriptor.value < digital_value_threshold_off())
            {
                new_value = false;
            }
            else if (descriptor.value > digital_value_threshold_on())
            {
                new_value = true;
            }
            else
            {
                return false;
            }

            if (_last_value[index] == NO_VALUE)
            {
                _last_value[index] = new_value;

                if (!new_value)
                {
                    return false;
                }

                descriptor.value = new_value;
                return true;
            }

            if (new_value == _last_value[index])
            {
                return false;
            }

            _last_value[index] = new_value;
            descriptor.value   = new_value;

            return true;
        }
    };
}    // namespace opendeck::io::analog
