/*

Copyright Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#pragma once

#include "deps.h"

#include "core/mcu.h"
#include "core/util/util.h"

#include <limits>

namespace io::analog
{
    class FilterHw : public Filter
    {
        public:
        FilterHw(uint8_t adcBits)
            : _adcConfig(adcBits == ADC_RESOLUTION_12_BIT ? ADC_12BIT : ADC_10BIT)
            , _adcBits(adcBits == ADC_RESOLUTION_12_BIT ? ADC_RESOLUTION_12_BIT : ADC_RESOLUTION_10_BIT)
        {
            for (size_t i = 0; i < io::analog::Collection::SIZE(); i++)
            {
                reset(i);
            }
        }

        bool isFiltered(size_t index, Descriptor& descriptor) override
        {
            if (index >= io::analog::Collection::SIZE())
            {
                return false;
            }

            const uint16_t ADC_MIN_VALUE = lowerOffsetRaw(descriptor.lowerOffset);
            const uint16_t ADC_MAX_VALUE = upperOffsetRaw(descriptor.upperOffset);

            descriptor.value = core::util::CONSTRAIN(descriptor.value,
                                                     ADC_MIN_VALUE,
                                                     ADC_MAX_VALUE);

            if (descriptor.type == type_t::BUTTON)
            {
                return isButtonFiltered(index, descriptor);
            }

            const auto MOTION_CONTEXT = prepareMotionContext(index);
            auto       inputState     = prepareContinuousInputState(index,
                                                                    descriptor,
                                                                    ADC_MIN_VALUE,
                                                                    ADC_MAX_VALUE);

            if (!applyAdcStage(index,
                               inputState,
                               descriptor.value))
            {
                return false;
            }

            return applyOutputStage(index,
                                    MOTION_CONTEXT,
                                    inputState,
                                    ADC_MIN_VALUE,
                                    ADC_MAX_VALUE,
                                    descriptor);
        }

        void reset(size_t index) override
        {
            if (index >= io::analog::Collection::SIZE())
            {
                return;
            }

            clearMotionContext(index);

            _lastMovementTime[index]       = NO_TIME;
            _lastSampleValue[index]        = NO_VALUE;
            _lastValue[index]              = NO_VALUE;
            _lastMidiValue[index]          = NO_VALUE;
            _pendingMidiValue[index]       = NO_VALUE;
            _fastModeEntryCounter[index]   = 0;
            _directionChangeCounter[index] = 0;
            _slowMidiChangeCounter[index]  = 0;
        }

        private:
        struct adcConfig_t
        {
            const uint16_t ADC_MIN_VALUE;
            const uint16_t ADC_MAX_VALUE;
            const uint16_t FSR_MIN_VALUE;
            const uint16_t FSR_MAX_VALUE;
            const uint16_t AFTERTOUCH_MAX_VALUE;
            const uint16_t DIGITAL_VALUE_THRESHOLD_ON;
            const uint16_t DIGITAL_VALUE_THRESHOLD_OFF;
        };

        struct EndpointAssistState
        {
            bool enabled = false;
            bool upper   = false;
            bool lower   = false;
        };

        struct MotionContextState
        {
            uint32_t now         = NO_TIME;
            bool     idleContext = true;
        };

        struct ContinuousInputState
        {
            bool                hasLastFilteredValue = false;
            bool                hasLastMidiValue     = false;
            uint16_t            lastFilteredValue    = 0;
            uint16_t            oldMidiValue         = NO_VALUE;
            uint16_t            minStepDiff          = 1;
            bool                direction            = false;
            EndpointAssistState endpointAssist       = {};
        };

        static constexpr uint8_t  ADC_RESOLUTION_10_BIT                     = 10;
        static constexpr uint8_t  ADC_RESOLUTION_12_BIT                     = 12;
        static constexpr uint8_t  FAST_FILTER_STEP_MULTIPLIER               = 2;
        static constexpr uint8_t  FAST_FILTER_ENTER_MULTIPLIER              = 2;
        static constexpr uint8_t  FAST_FILTER_EXIT_MULTIPLIER               = 1;
        static constexpr uint8_t  FAST_FILTER_ENTER_SAMPLE_COUNT            = 2;
        static constexpr uint32_t MOTION_CONTEXT_TIMEOUT_MS                 = 100;
        static constexpr uint8_t  ENDPOINT_ASSIST_MIDI_RANGE                = 1;
        static constexpr uint8_t  MIDI_DIRECTION_CHANGE_THRESHOLD           = 3;
        static constexpr uint8_t  IDLE_MIDI_CHANGE_SAMPLE_COUNT             = 2;
        static constexpr uint32_t PERCENTAGE_DIVISOR                        = 100;
        static constexpr uint8_t  FAST_FILTER_DIRECTION_CHANGE_SAMPLE_COUNT = 2;
        static constexpr size_t   BITS_PER_STORAGE_UNIT                     = 8;
        static constexpr uint8_t  EMA_FILTER_PERCENT                        = 50;
        static constexpr uint8_t  MIDI_7_BIT_MAX_VALUE                      = 127;
        static constexpr uint8_t  ADC_RESOLUTION_10_BIT_SHIFT               = 3;
        static constexpr uint8_t  ADC_RESOLUTION_12_BIT_SHIFT               = 5;
        static constexpr uint16_t NO_VALUE                                  = 0xFFFF;
        static constexpr uint32_t NO_TIME                                   = std::numeric_limits<uint32_t>::max();

        static constexpr adcConfig_t ADC_10BIT = {
            .ADC_MIN_VALUE               = 0,
            .ADC_MAX_VALUE               = 1023,
            .FSR_MIN_VALUE               = 40,
            .FSR_MAX_VALUE               = 340,
            .AFTERTOUCH_MAX_VALUE        = 600,
            .DIGITAL_VALUE_THRESHOLD_ON  = 800,
            .DIGITAL_VALUE_THRESHOLD_OFF = 200,
        };

        static constexpr adcConfig_t ADC_12BIT = {
            .ADC_MIN_VALUE               = 0,
            .ADC_MAX_VALUE               = 4095,
            .FSR_MIN_VALUE               = 160,
            .FSR_MAX_VALUE               = 1360,
            .AFTERTOUCH_MAX_VALUE        = 2400,
            .DIGITAL_VALUE_THRESHOLD_ON  = 3000,
            .DIGITAL_VALUE_THRESHOLD_OFF = 1000,
        };

        const adcConfig_t& _adcConfig;
        const uint8_t      _adcBits;

        uint8_t  _directionChangeCounter[io::analog::Collection::SIZE()]                    = {};
        uint8_t  _fastModeEntryCounter[io::analog::Collection::SIZE()]                      = {};
        uint8_t  _slowMidiChangeCounter[io::analog::Collection::SIZE()]                     = {};
        uint32_t _lastMovementTime[io::analog::Collection::SIZE()]                          = {};
        uint8_t  _fastMode[io::analog::Collection::SIZE() / BITS_PER_STORAGE_UNIT + 1]      = {};
        uint8_t  _lastDirection[io::analog::Collection::SIZE() / BITS_PER_STORAGE_UNIT + 1] = {};
        uint16_t _lastSampleValue[io::analog::Collection::SIZE()]                           = {};
        uint16_t _lastValue[io::analog::Collection::SIZE()]                                 = {};
        uint16_t _lastMidiValue[io::analog::Collection::SIZE()]                             = {};
        uint16_t _pendingMidiValue[io::analog::Collection::SIZE()]                          = {};

        void setLastDirection(size_t index, bool state)
        {
            core::util::BIT_WRITE(_lastDirection[index / BITS_PER_STORAGE_UNIT],
                                  index % BITS_PER_STORAGE_UNIT,
                                  state);
        }

        void setFastMode(size_t index, bool state)
        {
            core::util::BIT_WRITE(_fastMode[index / BITS_PER_STORAGE_UNIT],
                                  index % BITS_PER_STORAGE_UNIT,
                                  state);
        }

        bool lastDirection(size_t index)
        {
            return core::util::BIT_READ(_lastDirection[index / BITS_PER_STORAGE_UNIT],
                                        index % BITS_PER_STORAGE_UNIT);
        }

        bool fastModeLatched(size_t index)
        {
            return core::util::BIT_READ(_fastMode[index / BITS_PER_STORAGE_UNIT],
                                        index % BITS_PER_STORAGE_UNIT);
        }

        bool hasRecentMotionContext(size_t index, uint32_t now)
        {
            return (_lastMovementTime[index] != NO_TIME) &&
                   ((now - _lastMovementTime[index]) <= MOTION_CONTEXT_TIMEOUT_MS);
        }

        bool motionContextExpired(size_t index, uint32_t now)
        {
            return (_lastMovementTime[index] != NO_TIME) &&
                   ((now - _lastMovementTime[index]) > MOTION_CONTEXT_TIMEOUT_MS);
        }

        void expireMotionContextIfNeeded(size_t index, uint32_t now)
        {
            if (motionContextExpired(index, now))
            {
                clearMotionContext(index);
                _lastMovementTime[index] = NO_TIME;
            }
        }

        void clearMotionContext(size_t index)
        {
            clearDirectionChangeState(index);
            clearPendingMidiState(index);
            _fastModeEntryCounter[index] = 0;
            setFastMode(index, false);
        }

        void clearDirectionChangeState(size_t index)
        {
            _directionChangeCounter[index] = 0;
        }

        void clearPendingMidiState(size_t index)
        {
            _slowMidiChangeCounter[index] = 0;
            _pendingMidiValue[index]      = NO_VALUE;
        }

        static constexpr uint16_t absDiff(uint16_t lhs, uint16_t rhs)
        {
            return lhs >= rhs ? static_cast<uint16_t>(lhs - rhs)
                              : static_cast<uint16_t>(rhs - lhs);
        }

        static constexpr uint16_t applyEma(uint16_t value,
                                           bool     hasLastFilteredValue,
                                           uint16_t lastFilteredValue)
        {
            if (!hasLastFilteredValue)
            {
                return value;
            }

            return static_cast<uint16_t>((static_cast<uint32_t>(EMA_FILTER_PERCENT) * value +
                                          static_cast<uint32_t>(PERCENTAGE_DIVISOR - EMA_FILTER_PERCENT) * lastFilteredValue) /
                                         PERCENTAGE_DIVISOR);
        }

        uint16_t adcToOutputValue(uint16_t value,
                                  uint16_t adcMinValue,
                                  uint16_t adcMaxValue,
                                  uint16_t maxValue) const
        {
            if ((adcMinValue == _adcConfig.ADC_MIN_VALUE) &&
                (adcMaxValue == _adcConfig.ADC_MAX_VALUE) &&
                (maxValue == MIDI_7_BIT_MAX_VALUE))
            {
                return _adcBits == ADC_RESOLUTION_12_BIT ? static_cast<uint16_t>(value >> ADC_RESOLUTION_12_BIT_SHIFT) : static_cast<uint16_t>(value >> ADC_RESOLUTION_10_BIT_SHIFT);
            }

            return static_cast<uint16_t>(core::util::MAP_RANGE(static_cast<uint32_t>(value),
                                                               static_cast<uint32_t>(adcMinValue),
                                                               static_cast<uint32_t>(adcMaxValue),
                                                               static_cast<uint32_t>(0),
                                                               static_cast<uint32_t>(maxValue)));
        }

        MotionContextState prepareMotionContext(size_t index)
        {
            MotionContextState state = {};

            if (_lastMovementTime[index] != NO_TIME)
            {
                state.now         = core::mcu::timing::ms();
                state.idleContext = !hasRecentMotionContext(index, state.now);
                expireMotionContextIfNeeded(index, state.now);
            }

            return state;
        }

        ContinuousInputState prepareContinuousInputState(size_t            index,
                                                         const Descriptor& descriptor,
                                                         uint16_t          adcMinValue,
                                                         uint16_t          adcMaxValue)
        {
            ContinuousInputState state            = {};
            const bool           HAS_LAST_SAMPLE  = _lastSampleValue[index] != NO_VALUE;
            const auto           RAW_MIDI_VALUE   = adcToOutputValue(descriptor.value,
                                                                     adcMinValue,
                                                                     adcMaxValue,
                                                                     descriptor.maxValue);
            const bool           RAW_AT_ENDPOINT  = (RAW_MIDI_VALUE == 0) || (RAW_MIDI_VALUE == descriptor.maxValue);
            const auto           BASE_STEP_DIFF   = stepDiff(adcMinValue,
                                                             adcMaxValue,
                                                             descriptor.maxValue);
            const auto           ADC_DELTA        = HAS_LAST_SAMPLE ? absDiff(descriptor.value, _lastSampleValue[index]) : static_cast<uint16_t>(0);
            const auto           FAST_ENTER_THRES = static_cast<uint16_t>(BASE_STEP_DIFF * FAST_FILTER_ENTER_MULTIPLIER);
            const auto           FAST_EXIT_THRES  = static_cast<uint16_t>(BASE_STEP_DIFF * FAST_FILTER_EXIT_MULTIPLIER);

            state.hasLastFilteredValue = _lastValue[index] != NO_VALUE;
            state.hasLastMidiValue     = _lastMidiValue[index] != NO_VALUE;
            state.lastFilteredValue    = state.hasLastFilteredValue ? _lastValue[index] : descriptor.value;
            state.oldMidiValue         = state.hasLastMidiValue ? _lastMidiValue[index] : NO_VALUE;
            state.direction            = descriptor.value >= state.lastFilteredValue;
            state.endpointAssist       = endpointAssistState(state.hasLastMidiValue,
                                                             state.oldMidiValue,
                                                             RAW_MIDI_VALUE,
                                                             descriptor.maxValue);

            updateFastFilterState(index,
                                  HAS_LAST_SAMPLE,
                                  ADC_DELTA,
                                  FAST_ENTER_THRES,
                                  FAST_EXIT_THRES);

            state.minStepDiff = minStepDiff(index,
                                            state.direction,
                                            RAW_AT_ENDPOINT,
                                            state.hasLastMidiValue,
                                            state.oldMidiValue,
                                            descriptor.maxValue,
                                            BASE_STEP_DIFF);

            return state;
        }

        bool applyAdcStage(size_t                      index,
                           const ContinuousInputState& state,
                           uint16_t&                   value)
        {
            _lastSampleValue[index] = value;

            if (rejectDeadbandSample(index,
                                     state.hasLastFilteredValue,
                                     value,
                                     state.lastFilteredValue,
                                     state.endpointAssist.enabled,
                                     state.minStepDiff))
            {
                return false;
            }

            if (!applyAdcFilter(index,
                                state.endpointAssist.enabled,
                                state.direction))
            {
                return false;
            }

            value = applyEma(value,
                             state.hasLastFilteredValue,
                             state.lastFilteredValue);

            return true;
        }

        bool applyOutputStage(size_t                    index,
                              const MotionContextState& motionContext,
                              ContinuousInputState&     state,
                              uint16_t                  adcMinValue,
                              uint16_t                  adcMaxValue,
                              Descriptor&               descriptor)
        {
            auto midiValue = adcToOutputValue(descriptor.value,
                                              adcMinValue,
                                              adcMaxValue,
                                              descriptor.maxValue);

            if (!applyMidiPublishPolicy(index,
                                        state.endpointAssist,
                                        motionContext.idleContext,
                                        state.hasLastMidiValue,
                                        state.oldMidiValue,
                                        descriptor.maxValue,
                                        state.direction,
                                        midiValue))
            {
                return false;
            }

            const auto NOW = motionContext.now == NO_TIME ? core::mcu::timing::ms()
                                                          : motionContext.now;

            commitFilteredState(index,
                                state.direction,
                                descriptor.value,
                                midiValue,
                                NOW);

            if (descriptor.type == type_t::FSR)
            {
                descriptor.value = scaleFsrValue(descriptor.value, descriptor.maxValue);
            }
            else
            {
                descriptor.value = midiValue;
            }

            return true;
        }

        void updateFastFilterState(size_t   index,
                                   bool     hasLastSampleValue,
                                   uint16_t adcDelta,
                                   uint16_t fastEnterThreshold,
                                   uint16_t fastExitThreshold)
        {
            if (!hasLastSampleValue)
            {
                return;
            }

            if (fastModeLatched(index))
            {
                const bool FAST_FILTER = adcDelta >= fastExitThreshold;

                setFastMode(index, FAST_FILTER);

                if (!FAST_FILTER)
                {
                    _fastModeEntryCounter[index] = 0;
                }

                return;
            }

            if (adcDelta >= fastEnterThreshold)
            {
                _fastModeEntryCounter[index]++;

                if (_fastModeEntryCounter[index] >= FAST_FILTER_ENTER_SAMPLE_COUNT)
                {
                    setFastMode(index, true);
                    _fastModeEntryCounter[index] = 0;
                    return;
                }
            }
            else
            {
                _fastModeEntryCounter[index] = 0;
            }
        }

        EndpointAssistState endpointAssistState(bool     hasLastMidiValue,
                                                uint16_t oldMidiValue,
                                                uint16_t rawMidiValue,
                                                uint16_t maxValue) const
        {
            const auto UPPER_ENDPOINT_FLOOR  = maxValue > ENDPOINT_ASSIST_MIDI_RANGE ? static_cast<uint16_t>(maxValue - ENDPOINT_ASSIST_MIDI_RANGE) : static_cast<uint16_t>(0);
            const bool NEAR_UPPER_ENDPOINT   = hasLastMidiValue && (oldMidiValue >= UPPER_ENDPOINT_FLOOR);
            const bool NEAR_LOWER_ENDPOINT   = hasLastMidiValue && (oldMidiValue <= ENDPOINT_ASSIST_MIDI_RANGE);
            const bool UPPER_ENDPOINT_ASSIST = NEAR_UPPER_ENDPOINT &&
                                               (rawMidiValue == maxValue) &&
                                               (oldMidiValue < maxValue);
            const bool LOWER_ENDPOINT_ASSIST = NEAR_LOWER_ENDPOINT &&
                                               (rawMidiValue == 0) &&
                                               (oldMidiValue > 0);

            return {
                .enabled = UPPER_ENDPOINT_ASSIST || LOWER_ENDPOINT_ASSIST,
                .upper   = UPPER_ENDPOINT_ASSIST,
                .lower   = LOWER_ENDPOINT_ASSIST,
            };
        }

        uint16_t minStepDiff(size_t   index,
                             bool     direction,
                             bool     rawAtMidiEndpoint,
                             bool     hasLastMidiValue,
                             uint16_t oldMidiValue,
                             uint16_t maxValue,
                             uint16_t baseStepDiff)
        {
            if ((direction != lastDirection(index)) &&
                hasLastMidiValue &&
                !rawAtMidiEndpoint &&
                ((oldMidiValue != 0) && (oldMidiValue != maxValue)))
            {
                return static_cast<uint16_t>(baseStepDiff * FAST_FILTER_STEP_MULTIPLIER);
            }

            return baseStepDiff;
        }

        bool rejectDeadbandSample(size_t   index,
                                  bool     hasLastFilteredValue,
                                  uint16_t value,
                                  uint16_t lastFilteredValue,
                                  bool     endpointAssist,
                                  uint16_t minStepDiff)
        {
            if (!hasLastFilteredValue ||
                endpointAssist ||
                (absDiff(value, lastFilteredValue) >= minStepDiff))
            {
                return false;
            }

            clearDirectionChangeState(index);
            clearPendingMidiState(index);
            _fastModeEntryCounter[index] = 0;
            setFastMode(index, false);

            return true;
        }

        bool applyAdcFilter(size_t index,
                            bool   endpointAssist,
                            bool   direction)
        {
            if (endpointAssist)
            {
                clearDirectionChangeState(index);
                return true;
            }

            if (direction != lastDirection(index))
            {
                _directionChangeCounter[index]++;

                if (_directionChangeCounter[index] < FAST_FILTER_DIRECTION_CHANGE_SAMPLE_COUNT)
                {
                    return false;
                }
            }
            else
            {
                clearDirectionChangeState(index);
            }

            return true;
        }

        bool applyMidiPublishPolicy(size_t                     index,
                                    const EndpointAssistState& endpointAssist,
                                    bool                       idleContext,
                                    bool                       hasLastMidiValue,
                                    uint16_t                   oldMidiValue,
                                    uint16_t                   maxValue,
                                    bool&                      direction,
                                    uint16_t&                  midiValue)
        {
            if (endpointAssist.upper)
            {
                midiValue = maxValue;
                direction = true;
            }
            else if (endpointAssist.lower)
            {
                midiValue = 0;
                direction = false;
            }

            if (hasLastMidiValue && (midiValue == oldMidiValue))
            {
                clearPendingMidiState(index);
                return false;
            }

            if (!endpointAssist.enabled && idleContext && hasLastMidiValue)
            {
                if (_pendingMidiValue[index] != midiValue)
                {
                    _pendingMidiValue[index]      = midiValue;
                    _slowMidiChangeCounter[index] = 1;
                    return false;
                }

                _slowMidiChangeCounter[index]++;

                if (_slowMidiChangeCounter[index] < IDLE_MIDI_CHANGE_SAMPLE_COUNT)
                {
                    return false;
                }
            }

            if (!endpointAssist.enabled &&
                hasLastMidiValue &&
                (direction != lastDirection(index)) &&
                (absDiff(midiValue, oldMidiValue) < MIDI_DIRECTION_CHANGE_THRESHOLD))
            {
                clearPendingMidiState(index);
                return false;
            }

            return true;
        }

        void commitFilteredState(size_t   index,
                                 bool     direction,
                                 uint16_t value,
                                 uint16_t midiValue,
                                 uint32_t now)
        {
            setLastDirection(index, direction);
            clearDirectionChangeState(index);
            clearPendingMidiState(index);
            _lastValue[index]        = value;
            _lastMidiValue[index]    = midiValue;
            _lastMovementTime[index] = now;
        }

        uint16_t stepDiff(uint16_t adcMinValue,
                          uint16_t adcMaxValue,
                          uint16_t maxValue) const
        {
            const auto ADC_SPAN     = static_cast<uint32_t>(adcMaxValue) - static_cast<uint32_t>(adcMinValue);
            const auto OUTPUT_STEPS = static_cast<uint32_t>(maxValue) + 1U;
            const auto STEP_DIFF    = ADC_SPAN / OUTPUT_STEPS;

            return STEP_DIFF == 0U ? 1U : static_cast<uint16_t>(STEP_DIFF);
        }

        uint32_t lowerOffsetRaw(uint8_t percentage) const
        {
            if (percentage != 0)
            {
                const auto ADC_SPAN = static_cast<uint32_t>(_adcConfig.ADC_MAX_VALUE) - static_cast<uint32_t>(_adcConfig.ADC_MIN_VALUE);

                return static_cast<uint32_t>(_adcConfig.ADC_MIN_VALUE) +
                       ((ADC_SPAN * percentage) / PERCENTAGE_DIVISOR);
            }

            return _adcConfig.ADC_MIN_VALUE;
        }

        uint32_t upperOffsetRaw(uint8_t percentage) const
        {
            if (percentage != 0)
            {
                const auto ADC_SPAN = static_cast<uint32_t>(_adcConfig.ADC_MAX_VALUE) - static_cast<uint32_t>(_adcConfig.ADC_MIN_VALUE);

                return static_cast<uint32_t>(_adcConfig.ADC_MAX_VALUE) -
                       ((ADC_SPAN * percentage) / PERCENTAGE_DIVISOR);
            }

            return _adcConfig.ADC_MAX_VALUE;
        }

        uint16_t scaleFsrValue(uint16_t value,
                               uint16_t maxValue) const
        {
            return static_cast<uint16_t>(core::util::MAP_RANGE(
                core::util::CONSTRAIN(static_cast<uint32_t>(value),
                                      static_cast<uint32_t>(_adcConfig.FSR_MIN_VALUE),
                                      static_cast<uint32_t>(_adcConfig.FSR_MAX_VALUE)),
                static_cast<uint32_t>(_adcConfig.FSR_MIN_VALUE),
                static_cast<uint32_t>(_adcConfig.FSR_MAX_VALUE),
                static_cast<uint32_t>(0),
                static_cast<uint32_t>(maxValue)));
        }

        bool isButtonFiltered(size_t index, Descriptor& descriptor)
        {
            bool newValue = false;

            if (descriptor.value < _adcConfig.DIGITAL_VALUE_THRESHOLD_OFF)
            {
                newValue = false;
            }
            else if (descriptor.value > _adcConfig.DIGITAL_VALUE_THRESHOLD_ON)
            {
                newValue = true;
            }
            else
            {
                return false;
            }

            if (_lastValue[index] == NO_VALUE)
            {
                _lastValue[index] = newValue;

                if (!newValue)
                {
                    return false;
                }

                descriptor.value = newValue;
                return true;
            }

            if (newValue == _lastValue[index])
            {
                return false;
            }

            _lastValue[index] = newValue;
            descriptor.value  = newValue;

            return true;
        }
    };
}    // namespace io::analog
