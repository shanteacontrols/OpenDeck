/*

Copyright 2015-2022 Igor Petrovic

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

#ifdef ANALOG_SUPPORTED

#include <stdio.h>
#include <stdlib.h>
#include "core/src/general/Timing.h"
#include "io/analog/Analog.h"
#include "core/src/general/Helpers.h"

#ifdef ANALOG_USE_MEDIAN_FILTER
#define MEDIAN_SAMPLE_COUNT 3
#define MEDIAN_MIDDLE_VALUE 1
#endif

namespace IO
{
    class EMA
    {
        // exponential moving average filter
        public:
        EMA() = default;

        uint16_t value(uint16_t rawData)
        {
            _currentValue = (PERCENTAGE * static_cast<uint32_t>(rawData) + (100 - PERCENTAGE) * static_cast<uint32_t>(_currentValue)) / 100;
            return _currentValue;
        }

        void reset()
        {
            _currentValue = 0;
        }

        private:
        static constexpr uint32_t PERCENTAGE    = 50;
        uint16_t                  _currentValue = 0;
    };

    class AnalogFilter : public Analog::Filter
    {
        public:
#ifdef ADC_12_BIT
        static constexpr adcType_t ADC_RESOLUTION = adcType_t::ADC_12BIT;
#else
        static constexpr adcType_t ADC_RESOLUTION                                = adcType_t::ADC_10BIT;
#endif

        AnalogFilter()
            : _adcConfig(ADC_RESOLUTION == adcType_t::ADC_10BIT ? _adc10bit : _adc12bit)
            , STEP_DIFF_7BIT((_adcConfig.ADC_MAX_VALUE - _adcConfig.ADC_MIN_VALUE) / 128)
        {
            for (size_t i = 0; i < IO::Analog::Collection::size(); i++)
            {
                _lastStableValue[i] = 0xFFFF;
            }
        }

        bool isFiltered(size_t index, descriptor_t& descriptor) override
        {
            const uint32_t ADC_MIN_VALUE = lowerOffsetRaw(descriptor.lowerOffset);
            const uint32_t ADC_MAX_VALUE = upperOffsetRaw(descriptor.upperOffset);

            descriptor.value = CONSTRAIN(descriptor.value, ADC_MIN_VALUE, ADC_MAX_VALUE);

            // avoid filtering in this case for faster response
            if (descriptor.type == Analog::type_t::BUTTON)
            {
                if (descriptor.value < _adcConfig.DIGITAL_VALUE_THRESHOLD_OFF)
                {
                    descriptor.value = 1;
                }
                else if (descriptor.value > _adcConfig.DIGITAL_VALUE_THRESHOLD_ON)
                {
                    descriptor.value = 0;
                }
                else
                {
                    descriptor.value = 0;
                }

                return true;
            }

#ifdef ANALOG_USE_MEDIAN_FILTER
            auto compare = [](const void* a, const void* b)
            {
                if (*(uint16_t*)a < *(uint16_t*)b)
                {
                    return -1;
                }

                if (*(uint16_t*)a > *(uint16_t*)b)
                {
                    return 1;
                }

                return 0;
            };
#endif

            const bool FAST_FILTER    = (core::timing::currentRunTimeMs() - _lastStableMovementTime[index]) < FAST_FILTER_ENABLE_AFTER_MS;
            const bool DIRECTION      = descriptor.value >= _lastStableValue[index];
            const auto OLD_MID_IVALUE = core::misc::mapRange(static_cast<uint32_t>(_lastStableValue[index]), ADC_MIN_VALUE, ADC_MAX_VALUE, static_cast<uint32_t>(0), static_cast<uint32_t>(descriptor.maxValue));
            int16_t    stepDiff       = 1;

            if (((DIRECTION != lastStableDirection(index)) || !FAST_FILTER) && ((OLD_MID_IVALUE != 0) && (OLD_MID_IVALUE != descriptor.maxValue)))
            {
                stepDiff = STEP_DIFF_7BIT * 2;
            }

            if (abs(static_cast<int16_t>(descriptor.value) - static_cast<int16_t>(_lastStableValue[index])) < stepDiff)
            {
#ifdef ANALOG_USE_MEDIAN_FILTER
                _medianSampleCounter[index] = 0;
#endif

                return false;
            }

#ifdef ANALOG_USE_MEDIAN_FILTER
            if (!FAST_FILTER)
            {
                _analogSample[index][_medianSampleCounter[index]++] = descriptor.value;

                // take the median value to avoid using outliers
                if (_medianSampleCounter[index] == MEDIAN_SAMPLE_COUNT)
                {
                    qsort(_analogSample[index], MEDIAN_SAMPLE_COUNT, sizeof(uint16_t), compare);
                    _medianSampleCounter[index] = 0;
                    descriptor.value            = _analogSample[index][MEDIAN_MIDDLE_VALUE];
                }
                else
                {
                    return false;
                }
            }
#endif

#ifdef ANALOG_USE_EMA_FILTER
            descriptor.value = _emaFilter[index].value(descriptor.value);
#endif

            const auto MIDI_VALUE = core::misc::mapRange(static_cast<uint32_t>(descriptor.value), ADC_MIN_VALUE, ADC_MAX_VALUE, static_cast<uint32_t>(0), static_cast<uint32_t>(descriptor.maxValue));

            if (MIDI_VALUE == OLD_MID_IVALUE)
            {
                return false;
            }

            setLastStableDirection(index, DIRECTION);
            _lastStableValue[index] = descriptor.value;

            // when edge values are reached, disable fast filter by resetting last movement time
            if ((MIDI_VALUE == 0) || (MIDI_VALUE == descriptor.maxValue))
            {
                _lastStableMovementTime[index] = 0;
            }
            else
            {
                _lastStableMovementTime[index] = core::timing::currentRunTimeMs();
            }

            if (descriptor.type == Analog::type_t::FSR)
            {
                descriptor.value = core::misc::mapRange(CONSTRAIN(descriptor.value,
                                                                  static_cast<uint32_t>(_adcConfig.FSR_MIN_VALUE),
                                                                  static_cast<uint32_t>(_adcConfig.FSR_MAX_VALUE)),
                                                        static_cast<uint32_t>(_adcConfig.FSR_MIN_VALUE),
                                                        static_cast<uint32_t>(_adcConfig.FSR_MAX_VALUE),
                                                        static_cast<uint32_t>(0),
                                                        static_cast<uint32_t>(descriptor.maxValue));
            }
            else
            {
                descriptor.value = MIDI_VALUE;
            }

            return true;
        }

        uint16_t lastValue(size_t index) override
        {
            if (index < IO::Analog::Collection::size())
            {
                return _lastStableValue[index];
            }

            return 0;
        }

        void reset(size_t index) override
        {
            if (index < IO::Analog::Collection::size())
            {
#ifdef ANALOG_USE_MEDIAN_FILTER
                _medianSampleCounter[index] = 0;
#endif
                _lastStableMovementTime[index] = 0;
            }

            _lastStableValue[index] = 0xFFFF;
        }

        private:
        struct adcConfig_t
        {
            const uint16_t ADC_MIN_VALUE;                  ///< Minimum raw ADC value.
            const uint16_t ADC_MAX_VALUE;                  ///< Maxmimum raw ADC value.
            const uint16_t FSR_MIN_VALUE;                  ///< Minimum raw ADC reading for FSR sensors.
            const uint16_t FSR_MAX_VALUE;                  ///< Maximum raw ADC reading for FSR sensors.
            const uint16_t AFTERTOUCH_MAX_VALUE;           ///< Maxmimum raw ADC reading for aftertouch on FSR sensors.
            const uint16_t DIGITAL_VALUE_THRESHOLD_ON;     ///< Value above which buton connected to analog input is considered pressed.
            const uint16_t DIGITAL_VALUE_THRESHOLD_OFF;    ///< Value below which button connected to analog input is considered released.
        };

        void setLastStableDirection(size_t index, bool state)
        {
            uint8_t arrayIndex  = index / 8;
            uint8_t analogIndex = index - 8 * arrayIndex;

            BIT_WRITE(_lastStableDirection[arrayIndex], analogIndex, state);
        }

        bool lastStableDirection(size_t index)
        {
            uint8_t arrayIndex  = index / 8;
            uint8_t analogIndex = index - 8 * arrayIndex;

            return BIT_READ(_lastStableDirection[arrayIndex], analogIndex);
        }

        uint32_t lowerOffsetRaw(uint8_t percentage)
        {
            // calculate raw adc value based on percentage

            if (percentage != 0)
            {
                return static_cast<double>(_adcConfig.ADC_MAX_VALUE) * static_cast<double>(percentage / 100.0);
            }

            return _adcConfig.ADC_MIN_VALUE;
        }

        uint32_t upperOffsetRaw(uint8_t percentage)
        {
            // calculate raw adc value based on percentage

            if (percentage != 0)
            {
                return static_cast<double>(_adcConfig.ADC_MAX_VALUE) - static_cast<double>(_adcConfig.ADC_MAX_VALUE * static_cast<double>(percentage / 100.0));
            }

            return _adcConfig.ADC_MAX_VALUE;
        }

        adcConfig_t _adc10bit = {
            .ADC_MIN_VALUE               = 10,
            .ADC_MAX_VALUE               = 1000,
            .FSR_MIN_VALUE               = 40,
            .FSR_MAX_VALUE               = 340,
            .AFTERTOUCH_MAX_VALUE        = 600,
            .DIGITAL_VALUE_THRESHOLD_ON  = 1000,
            .DIGITAL_VALUE_THRESHOLD_OFF = 600,
        };

        adcConfig_t _adc12bit = {
            .ADC_MIN_VALUE               = 10,
            .ADC_MAX_VALUE               = 4000,
            .FSR_MIN_VALUE               = 160,
            .FSR_MAX_VALUE               = 1360,
            .AFTERTOUCH_MAX_VALUE        = 2400,
            .DIGITAL_VALUE_THRESHOLD_ON  = 4000,
            .DIGITAL_VALUE_THRESHOLD_OFF = 2400,
        };

        adcConfig_t&              _adcConfig;
        const uint16_t            STEP_DIFF_7BIT;
        static constexpr uint32_t FAST_FILTER_ENABLE_AFTER_MS = 50;

// some filtering is needed for adc only
#ifdef ANALOG_USE_EMA_FILTER
        EMA _emaFilter[IO::Analog::Collection::size()];
#endif

#ifdef ANALOG_USE_MEDIAN_FILTER
        uint16_t _analogSample[IO::Analog::Collection::size()][MEDIAN_SAMPLE_COUNT] = {};
        uint8_t  _medianSampleCounter[IO::Analog::Collection::size()]               = {};
#else
        uint16_t                   _analogSample[IO::Analog::Collection::size()] = {};
#endif
        uint32_t _lastStableMovementTime[IO::Analog::Collection::size()] = {};

        uint8_t  _lastStableDirection[IO::Analog::Collection::size() / 8 + 1] = {};
        uint16_t _lastStableValue[IO::Analog::Collection::size()]             = {};
    };    // namespace IO
}    // namespace IO

#else
#include "stub/Filter.h"
#endif
