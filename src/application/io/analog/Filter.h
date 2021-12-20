/*

Copyright 2015-2021 Igor Petrovic

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

#ifndef MEDIAN_SAMPLE_COUNT
#define MEDIAN_SAMPLE_COUNT 3
#endif

#ifndef MEDIAN_MIDDLE_VALUE
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
        static constexpr adcType_t ADC_RESOLUTION = adcType_t::adc10bit;
#endif

        AnalogFilter()
            : _adcConfig(ADC_RESOLUTION == adcType_t::adc10bit ? adc10bit : adc12bit)
            , _stepDiff7Bit((_adcConfig.adcMaxValue - _adcConfig.adcMinValue) / 128)
        {}

        bool isFiltered(size_t index, Analog::type_t type, uint16_t value, uint16_t& filteredValue) override
        {
            value = CONSTRAIN(value, _adcConfig.adcMinValue, _adcConfig.adcMaxValue);

            // avoid filtering in this case for faster response
            if (type == Analog::type_t::button)
            {
                if (value < _adcConfig.digitalValueThresholdOff)
                    filteredValue = 1;
                else if (value > _adcConfig.digitalValueThresholdOn)
                    filteredValue = 0;
                else
                    filteredValue = 0;

                return true;
            }

#ifdef ADC_SUPPORTED
            auto compare = [](const void* a, const void* b) {
                if (*(uint16_t*)a < *(uint16_t*)b)
                    return -1;
                else if (*(uint16_t*)a > *(uint16_t*)b)
                    return 1;

                return 0;
            };

            const bool fastFilter = (index < IO::Analog::Collection::size(IO::Analog::GROUP_ANALOG_INPUTS)) ? (core::timing::currentRunTimeMs() - _lastStableMovementTime[index]) < FAST_FILTER_ENABLE_AFTER_MS : true;
#else
            const bool fastFilter = true;
#endif

            const bool     use14bit     = (type == Analog::type_t::nrpn14bit) || (type == Analog::type_t::pitchBend) || (type == Analog::type_t::controlChange14bit);
            const uint16_t maxLimit     = use14bit ? MIDI::MIDI_14_BIT_VALUE_MAX : MIDI::MIDI_7_BIT_VALUE_MAX;
            const bool     direction    = value >= _lastStableValue[index];
            const auto     oldMIDIvalue = core::misc::mapRange(static_cast<uint32_t>(_lastStableValue[index]), static_cast<uint32_t>(_adcConfig.adcMinValue), static_cast<uint32_t>(_adcConfig.adcMaxValue), static_cast<uint32_t>(0), static_cast<uint32_t>(maxLimit));
            uint16_t       stepDiff     = 1;

            if (((direction != _lastStableDirection[index]) || !fastFilter) && ((oldMIDIvalue != 0) && (oldMIDIvalue != maxLimit)))
            {
                stepDiff = _stepDiff7Bit * 2;
            }

            if (abs(value - _lastStableValue[index]) < stepDiff)
            {
#ifdef ADC_SUPPORTED
                if (index < IO::Analog::Collection::size(IO::Analog::GROUP_ANALOG_INPUTS))
                    _medianSampleCounter[index] = 0;
#endif

                return false;
            }

#ifdef ADC_SUPPORTED
            if (index < IO::Analog::Collection::size(IO::Analog::GROUP_ANALOG_INPUTS))
            {
                // don't filter the readings for touchscreen data

                if (!fastFilter)
                {
                    _analogSample[index][_medianSampleCounter[index]++] = value;

                    // take the median value to avoid using outliers
                    if (_medianSampleCounter[index] == MEDIAN_SAMPLE_COUNT)
                    {
                        qsort(_analogSample[index], MEDIAN_SAMPLE_COUNT, sizeof(uint16_t), compare);
                        _medianSampleCounter[index] = 0;
                        filteredValue               = _analogSample[index][MEDIAN_MIDDLE_VALUE];
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    filteredValue = value;
                }

                if (index < IO::Analog::Collection::size(IO::Analog::GROUP_ANALOG_INPUTS))
                    filteredValue = _emaFilter[index].value(filteredValue);
            }
            else
            {
                filteredValue = value;
            }
#else
            filteredValue         = value;
#endif

            const auto midiValue = core::misc::mapRange(static_cast<uint32_t>(filteredValue), static_cast<uint32_t>(_adcConfig.adcMinValue), static_cast<uint32_t>(_adcConfig.adcMaxValue), static_cast<uint32_t>(0), static_cast<uint32_t>(maxLimit));

            if (midiValue == oldMIDIvalue)
                return false;

            _lastStableDirection[index] = direction;
            _lastStableValue[index]     = filteredValue;

#ifdef ADC_SUPPORTED
            if (index < IO::Analog::Collection::size(IO::Analog::GROUP_ANALOG_INPUTS))
            {
                // when edge values are reached, disable fast filter by resetting last movement time
                if ((midiValue == 0) || (midiValue == maxLimit))
                    _lastStableMovementTime[index] = 0;
                else
                    _lastStableMovementTime[index] = core::timing::currentRunTimeMs();
            }
#endif

            if (type == Analog::type_t::fsr)
            {
                filteredValue = core::misc::mapRange(CONSTRAIN(filteredValue, static_cast<uint32_t>(_adcConfig.fsrMinValue), static_cast<uint32_t>(_adcConfig.fsrMaxValue)), static_cast<uint32_t>(_adcConfig.fsrMinValue), static_cast<uint32_t>(_adcConfig.fsrMaxValue), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI::MIDI_7_BIT_VALUE_MAX));
            }
            else
            {
                filteredValue = midiValue;
            }

            return true;
        }

        void reset(size_t index) override
        {
#ifdef ADC_SUPPORTED
            if (index < IO::Analog::Collection::size(IO::Analog::GROUP_ANALOG_INPUTS))
            {
                _medianSampleCounter[index]    = 0;
                _lastStableMovementTime[index] = 0;
            }
#endif

            _lastStableValue[index] = 0;
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
#ifdef ADC_SUPPORTED
        EMA      _emaFilter[IO::Analog::Collection::size(IO::Analog::GROUP_ANALOG_INPUTS)];
        uint16_t _analogSample[IO::Analog::Collection::size(IO::Analog::GROUP_ANALOG_INPUTS)][MEDIAN_SAMPLE_COUNT] = {};
        size_t   _medianSampleCounter[IO::Analog::Collection::size(IO::Analog::GROUP_ANALOG_INPUTS)]               = {};
        uint32_t _lastStableMovementTime[IO::Analog::Collection::size(IO::Analog::GROUP_ANALOG_INPUTS)]            = {};
#endif

        bool     _lastStableDirection[IO::Analog::Collection::size()] = {};
        uint16_t _lastStableValue[IO::Analog::Collection::size()]     = {};
    };    // namespace IO
}    // namespace IO

#else
#include "stub/Filter.h"
#endif
