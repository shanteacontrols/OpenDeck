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
        static constexpr adcType_t ADC_RESOLUTION = adcType_t::adc12bit;
#else
        static constexpr adcType_t ADC_RESOLUTION                                = adcType_t::adc10bit;
#endif

        AnalogFilter()
            : _adcConfig(ADC_RESOLUTION == adcType_t::adc10bit ? adc10bit : adc12bit)
            , _stepDiff7Bit((_adcConfig.adcMaxValue - _adcConfig.adcMinValue) / 128)
        {
            for (size_t i = 0; i < IO::Analog::Collection::size(); i++)
                _lastStableValue[i] = 0xFFFF;
        }

        bool isFiltered(size_t index, descriptor_t& descriptor) override
        {
            const uint32_t adcMinValue = lowerOffsetRaw(descriptor.lowerOffset);
            const uint32_t adcMaxValue = upperOffsetRaw(descriptor.upperOffset);

            descriptor.value = CONSTRAIN(descriptor.value, adcMinValue, adcMaxValue);

            // avoid filtering in this case for faster response
            if (descriptor.type == Analog::type_t::button)
            {
                if (descriptor.value < _adcConfig.digitalValueThresholdOff)
                    descriptor.value = 1;
                else if (descriptor.value > _adcConfig.digitalValueThresholdOn)
                    descriptor.value = 0;
                else
                    descriptor.value = 0;

                return true;
            }

#ifdef ANALOG_USE_MEDIAN_FILTER
            auto compare = [](const void* a, const void* b) {
                if (*(uint16_t*)a < *(uint16_t*)b)
                    return -1;
                else if (*(uint16_t*)a > *(uint16_t*)b)
                    return 1;

                return 0;
            };
#endif

            const bool fastFilter = (core::timing::currentRunTimeMs() - _lastStableMovementTime[index]) < FAST_FILTER_ENABLE_AFTER_MS;

            const bool     use14bit     = (descriptor.type == Analog::type_t::nrpn14bit) || (descriptor.type == Analog::type_t::pitchBend) || (descriptor.type == Analog::type_t::controlChange14bit);
            const uint16_t maxLimit     = use14bit ? MIDI::MIDI_14_BIT_VALUE_MAX : (descriptor.type == Analog::type_t::dmx) ? 255
                                                                                                                            : MIDI::MIDI_7_BIT_VALUE_MAX;
            const bool     direction    = descriptor.value >= _lastStableValue[index];
            const auto     oldMIDIvalue = core::misc::mapRange(static_cast<uint32_t>(_lastStableValue[index]), adcMinValue, adcMaxValue, static_cast<uint32_t>(0), static_cast<uint32_t>(maxLimit));
            uint16_t       stepDiff     = 1;

            if (((direction != lastStableDirection(index)) || !fastFilter) && ((oldMIDIvalue != 0) && (oldMIDIvalue != maxLimit)))
            {
                stepDiff = _stepDiff7Bit * 2;
            }

            if (abs(descriptor.value - _lastStableValue[index]) < stepDiff)
            {
#ifdef ANALOG_USE_MEDIAN_FILTER
                _medianSampleCounter[index] = 0;
#endif

                return false;
            }

#ifdef ANALOG_USE_MEDIAN_FILTER
            if (!fastFilter)
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

            const auto midiValue = core::misc::mapRange(static_cast<uint32_t>(descriptor.value), adcMinValue, adcMaxValue, static_cast<uint32_t>(0), static_cast<uint32_t>(maxLimit));

            if (midiValue == oldMIDIvalue)
                return false;

            setLastStableDirection(index, direction);
            _lastStableValue[index] = descriptor.value;

            // when edge values are reached, disable fast filter by resetting last movement time
            if ((midiValue == 0) || (midiValue == maxLimit))
                _lastStableMovementTime[index] = 0;
            else
                _lastStableMovementTime[index] = core::timing::currentRunTimeMs();

            if (descriptor.type == Analog::type_t::fsr)
            {
                descriptor.value = core::misc::mapRange(CONSTRAIN(descriptor.value, static_cast<uint32_t>(_adcConfig.fsrMinValue), static_cast<uint32_t>(_adcConfig.fsrMaxValue)), static_cast<uint32_t>(_adcConfig.fsrMinValue), static_cast<uint32_t>(_adcConfig.fsrMaxValue), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI::MIDI_7_BIT_VALUE_MAX));
            }
            else
            {
                descriptor.value = midiValue;
            }

            return true;
        }

        uint16_t lastValue(size_t index) override
        {
            if (index < IO::Analog::Collection::size())
                return _lastStableValue[index];

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
        using adcConfig_t = struct
        {
            const uint16_t adcMinValue;                 ///< Minimum raw ADC value.
            const uint16_t adcMaxValue;                 ///< Maxmimum raw ADC value.
            const uint16_t fsrMinValue;                 ///< Minimum raw ADC reading for FSR sensors.
            const uint16_t fsrMaxValue;                 ///< Maximum raw ADC reading for FSR sensors.
            const uint16_t aftertouchMaxValue;          ///< Maxmimum raw ADC reading for aftertouch on FSR sensors.
            const uint16_t digitalValueThresholdOn;     ///< Value above which buton connected to analog input is considered pressed.
            const uint16_t digitalValueThresholdOff;    ///< Value below which button connected to analog input is considered released.
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
                return static_cast<double>(_adcConfig.adcMaxValue) * static_cast<double>(percentage / 100.0);
            }
            else
            {
                return _adcConfig.adcMinValue;
            }
        }

        uint32_t upperOffsetRaw(uint8_t percentage)
        {
            // calculate raw adc value based on percentage

            if (percentage != 0)
            {
                return static_cast<double>(_adcConfig.adcMaxValue) - static_cast<double>(_adcConfig.adcMaxValue * static_cast<double>(percentage / 100.0));
            }
            else
            {
                return _adcConfig.adcMaxValue;
            }
        }

        adcConfig_t adc10bit = {
            .adcMinValue              = 10,
            .adcMaxValue              = 1000,
            .fsrMinValue              = 40,
            .fsrMaxValue              = 340,
            .aftertouchMaxValue       = 600,
            .digitalValueThresholdOn  = 1000,
            .digitalValueThresholdOff = 600,
        };

        adcConfig_t adc12bit = {
            .adcMinValue              = 10,
            .adcMaxValue              = 4000,
            .fsrMinValue              = 160,
            .fsrMaxValue              = 1360,
            .aftertouchMaxValue       = 2400,
            .digitalValueThresholdOn  = 4000,
            .digitalValueThresholdOff = 2400,
        };

        adcConfig_t&              _adcConfig;
        const uint16_t            _stepDiff7Bit;
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
