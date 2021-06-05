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

#include <stdio.h>
#include <stdlib.h>
#include "core/src/general/Timing.h"
#include "io/analog/Analog.h"
#include "core/src/general/Helpers.h"

namespace IO
{
    class AnalogFilter : public IO::Analog::Filter
    {
        public:
        AnalogFilter(IO::Analog::adcType_t adcType)
            : _adcType(adcType)
            , _adcConfig(adcType == IO::Analog::adcType_t::adc10bit ? adc10bit : adc12bit)
            , _stepDiff7Bit(static_cast<uint16_t>(adcType) / 128)
        {}

        Analog::adcType_t adcType() override
        {
            return _adcType;
        }

        bool isFiltered(size_t index, Analog::type_t type, uint16_t value, uint16_t& filteredValue) override
        {
            value = CONSTRAIN(value, 0, _adcConfig.adcMaxValue);

            //avoid filtering in this case for faster response
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

            auto compare = [](const void* a, const void* b) {
                if (*(uint16_t*)a < *(uint16_t*)b)
                    return -1;
                else if (*(uint16_t*)a > *(uint16_t*)b)
                    return 1;

                return 0;
            };

            const bool     fastFilter = (index < MAX_NUMBER_OF_ANALOG) ? (core::timing::currentRunTimeMs() - _lastStableMovementTime[index]) < FAST_FILTER_ENABLE_AFTER_MS : true;
            const bool     use14bit   = (type == Analog::type_t::nrpn14b) || (type == Analog::type_t::pitchBend) || (type == Analog::type_t::cc14bit);
            const uint16_t maxLimit   = use14bit ? MIDI_14_BIT_VALUE_MAX : MIDI_7_BIT_VALUE_MAX;
            const bool     direction  = value >= _lastStableValue[index];
            const uint16_t stepDiff   = !use14bit || (direction != _lastStableDirection[index]) || !fastFilter ? _stepDiff7Bit : _adcConfig.stepDiff14Bit;

            if (abs(value - _lastStableValue[index]) < stepDiff)
            {
                if (index < MAX_NUMBER_OF_ANALOG)
                    _medianSampleCounter[index] = 0;

                return false;
            }

            if (index < MAX_NUMBER_OF_ANALOG)
            {
                //don't filter the readings for touchscreen data

                if (!fastFilter)
                {
                    _analogSample[index][_medianSampleCounter[index]++] = value;

                    //take the median value to avoid using outliers
                    if (_medianSampleCounter[index] == 5)
                    {
                        qsort(_analogSample[index], 5, sizeof(uint16_t), compare);
                        _medianSampleCounter[index] = 0;
                        filteredValue               = _analogSample[index][3];
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
            }
            else
            {
                filteredValue = value;
            }

            const auto midiValue    = core::misc::mapRange(static_cast<uint32_t>(filteredValue), static_cast<uint32_t>(0), static_cast<uint32_t>(_adcConfig.adcMaxValue), static_cast<uint32_t>(0), static_cast<uint32_t>(maxLimit));
            const auto oldMIDIvalue = core::misc::mapRange(static_cast<uint32_t>(_lastStableValue[index]), static_cast<uint32_t>(0), static_cast<uint32_t>(_adcConfig.adcMaxValue), static_cast<uint32_t>(0), static_cast<uint32_t>(maxLimit));

            if (midiValue == oldMIDIvalue)
                return false;

            _lastStableDirection[index] = direction;
            _lastStableValue[index]     = filteredValue;

            if (index < MAX_NUMBER_OF_ANALOG)
                _lastStableMovementTime[index] = core::timing::currentRunTimeMs();

            if (type == Analog::type_t::fsr)
            {
                filteredValue = core::misc::mapRange(CONSTRAIN(filteredValue, static_cast<uint32_t>(_adcConfig.fsrMinValue), static_cast<uint32_t>(_adcConfig.fsrMaxValue)), static_cast<uint32_t>(_adcConfig.fsrMinValue), static_cast<uint32_t>(_adcConfig.fsrMaxValue), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX));
            }
            else
            {
                filteredValue = midiValue;
            }

            //when edge values are reached, disable fast filter by resetting last movement time
            if (((midiValue == 0) || (midiValue == maxLimit)) && (index < MAX_NUMBER_OF_ANALOG))
                _lastStableMovementTime[index] = 0;

            return true;
        }

        void reset(size_t index) override
        {
            if (index < MAX_NUMBER_OF_ANALOG)
            {
                _medianSampleCounter[index]    = 0;
                _lastStableMovementTime[index] = 0;
            }

            _lastStableValue[index] = 0;
        }

        private:
        using adcConfig_t = struct
        {
            const uint16_t adcMaxValue;                 ///< Maxmimum raw ADC value.
            const uint16_t stepDiff14Bit;               ///< Minimum difference between two raw ADC readings to consider that value has been changed for 14-bit MIDI values.
            const uint16_t fsrMinValue;                 ///< Minimum raw ADC reading for FSR sensors.
            const uint16_t fsrMaxValue;                 ///< Maximum raw ADC reading for FSR sensors.
            const uint16_t aftertouchMaxValue;          ///< Maxmimum raw ADC reading for aftertouch on FSR sensors.
            const uint16_t digitalValueThresholdOn;     ///< Value above which buton connected to analog input is considered pressed.
            const uint16_t digitalValueThresholdOff;    ///< Value below which button connected to analog input is considered released.
        };

        adcConfig_t adc10bit = {
            .adcMaxValue              = 1000,
            .stepDiff14Bit            = 1,
            .fsrMinValue              = 40,
            .fsrMaxValue              = 340,
            .aftertouchMaxValue       = 600,
            .digitalValueThresholdOn  = 1000,
            .digitalValueThresholdOff = 600,
        };

        adcConfig_t adc12bit = {
            .adcMaxValue              = 4000,
            .stepDiff14Bit            = 2,
            .fsrMinValue              = 160,
            .fsrMaxValue              = 1360,
            .aftertouchMaxValue       = 2400,
            .digitalValueThresholdOn  = 4000,
            .digitalValueThresholdOff = 2400,
        };

        const IO::Analog::adcType_t _adcType;
        adcConfig_t&                _adcConfig;
        const uint16_t              _stepDiff7Bit;

        static constexpr uint32_t FAST_FILTER_ENABLE_AFTER_MS                                                       = 100;
        uint16_t                  _analogSample[MAX_NUMBER_OF_ANALOG][5]                                            = {};
        size_t                    _medianSampleCounter[MAX_NUMBER_OF_ANALOG]                                        = {};
        uint32_t                  _lastStableMovementTime[MAX_NUMBER_OF_ANALOG]                                     = {};
        bool                      _lastStableDirection[MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS] = {};
        uint16_t                  _lastStableValue[MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS]     = {};
    };    // namespace IO
}    // namespace IO