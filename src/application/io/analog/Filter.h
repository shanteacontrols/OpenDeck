/*

Copyright 2015-2020 Igor Petrovic

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
    class EMA
    {
        //exponential moving average filter
        public:
        EMA() = default;

        uint16_t value(uint16_t rawData)
        {
            _currentValue = (_percentage * static_cast<uint32_t>(rawData) + (100 - _percentage) * static_cast<uint32_t>(_currentValue)) / 100;
            return _currentValue;
        }

        void reset()
        {
            _currentValue = 0;
        }

        private:
        uint16_t                  _currentValue = 0;
        static constexpr uint32_t _percentage   = 60;
    };

    class AnalogFilter : public IO::Analog::Filter
    {
        public:
        AnalogFilter(IO::Analog::Filter::adcType_t adcType, size_t stableValueRepetitions)
            : _adcType(adcType)
            , _adcConfig(adcType == IO::Analog::Filter::adcType_t::adc10bit ? adc10bit : adc12bit)
            , _stableValueRepetitions(stableValueRepetitions)
        {}

        bool isFiltered(size_t index, Analog::type_t type, uint16_t value, uint16_t& filteredValue) override
        {
            auto compare = [](const void* a, const void* b) {
                if (*(uint16_t*)a < *(uint16_t*)b)
                    return -1;
                else if (*(uint16_t*)a > *(uint16_t*)b)
                    return 1;

                return 0;
            };

            if (_adcType == Analog::Filter::adcType_t::adc12bit)
            {
                //on 12bit ADCs, when the value is above 90%, add small value
                //to offset the reading error and possible situation that the maximum
                //MIDI value cannot be reached
                //empirical
                if ((value >= 3686) && (value <= 4087))
                    value += 8;
            }

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

            bool fastFilter = true;

            if ((core::timing::currentRunTimeMs() - _lastMovementTime[index]) > _fastFilterEnableAfter)
            {
                fastFilter                                    = false;
                _analogSample[index][_sampleCounter[index]++] = value;

                //take the median value to avoid using outliers
                if (_sampleCounter[index] == 3)
                {
                    qsort(_analogSample[index], 3, sizeof(uint16_t), compare);
                    _sampleCounter[index] = 0;
                    filteredValue         = _analogSample[index][1];
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

            //pass the value through exponential moving average filter for increased stability
            filteredValue = _emaFilter[index].value(filteredValue);

            bool     use14bit = false;
            uint16_t maxLimit;
            uint16_t stepDiff;

            if ((type == Analog::type_t::nrpn14b) || (type == Analog::type_t::pitchBend) || (type == Analog::type_t::cc14bit))
                use14bit = true;

            if (use14bit)
            {
                maxLimit = MIDI_14_BIT_VALUE_MAX;
                stepDiff = _adcConfig.stepDiff14Bit;
            }
            else
            {
                maxLimit = MIDI_7_BIT_VALUE_MAX;
                stepDiff = _adcConfig.stepDiff7Bit;
            }

            //if the first read value is 0, mark it as increasing
            valDirection_t direction = filteredValue >= _lastStableValue[index] ? valDirection_t::increasing : valDirection_t::decreasing;

            //don't perform these checks on initial value readout
            if (_lastDirection[index] != valDirection_t::initial)
            {
                if (direction != _lastDirection[index])
                    stepDiff = _adcConfig.stepDiffDirChange;

                if (abs(filteredValue - _lastStableValue[index]) < stepDiff)
                {
                    _stableSampleCount[index] = 0;
                    return false;
                }
            }

            auto midiValue    = core::misc::mapRange(static_cast<uint32_t>(filteredValue), static_cast<uint32_t>(0), static_cast<uint32_t>(_adcConfig.adcMaxValue), static_cast<uint32_t>(0), static_cast<uint32_t>(maxLimit));
            auto oldMIDIvalue = core::misc::mapRange(static_cast<uint32_t>(_lastStableValue[index]), static_cast<uint32_t>(0), static_cast<uint32_t>(_adcConfig.adcMaxValue), static_cast<uint32_t>(0), static_cast<uint32_t>(maxLimit));

            //this will allow value 0 as the first sent value
            if ((midiValue == oldMIDIvalue) && (_lastDirection[index] != valDirection_t::initial))
            {
                _stableSampleCount[index] = 0;
                return false;
            }

            auto acceptNewValue = [&]() {
                _stableSampleCount[index] = 0;
                _lastDirection[index]     = direction;
                _lastStableValue[index]   = filteredValue;
                _lastMovementTime[index]  = core::timing::currentRunTimeMs();
            };

            if (fastFilter)
            {
                acceptNewValue();
            }
            else
            {
                if (++_stableSampleCount[index] >= _stableValueRepetitions)
                    acceptNewValue();
                else
                    return false;
            }

            if (type == Analog::type_t::fsr)
            {
                filteredValue = core::misc::mapRange(CONSTRAIN(filteredValue, static_cast<uint32_t>(_adcConfig.fsrMinValue), static_cast<uint32_t>(_adcConfig.fsrMaxValue)), static_cast<uint32_t>(_adcConfig.fsrMinValue), static_cast<uint32_t>(_adcConfig.fsrMaxValue), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX));
            }
            else
            {
                filteredValue = midiValue;
            }

            return true;
        }

        void reset(size_t index) override
        {
            _sampleCounter[index] = 0;
            _emaFilter[index].reset();
        }

        private:
        using adcConfig_t = struct
        {
            const uint16_t adcMaxValue;                 ///< Maxmimum raw ADC value.
            const uint16_t stepDiff7Bit;                ///< Minimum difference between two raw ADC readings to consider that value has been changed for 7-bit MIDI values.
            const uint16_t stepDiff14Bit;               ///< Minimum difference between two raw ADC readings to consider that value has been changed for 14-bit MIDI values.
            const uint16_t stepDiffDirChange;           ///< Same as stepDiff7Bit and stepDiff14Bit, only used when the direction is different from the last one.
            const uint16_t fsrMinValue;                 ///< Minimum raw ADC reading for FSR sensors.
            const uint16_t fsrMaxValue;                 ///< Maximum raw ADC reading for FSR sensors.
            const uint16_t aftertouchMaxValue;          ///< Maxmimum raw ADC reading for aftertouch on FSR sensors.
            const uint16_t digitalValueThresholdOn;     ///< Value above which buton connected to analog input is considered pressed.
            const uint16_t digitalValueThresholdOff;    ///< Value below which button connected to analog input is considered released.
        };

        enum class valDirection_t : uint8_t
        {
            initial,
            decreasing,
            increasing
        };

        adcConfig_t adc10bit = {
            .adcMaxValue              = 1023,
            .stepDiff7Bit             = 6,
            .stepDiff14Bit            = 1,
            .stepDiffDirChange        = 6,
            .fsrMinValue              = 40,
            .fsrMaxValue              = 340,
            .aftertouchMaxValue       = 600,
            .digitalValueThresholdOn  = 1000,
            .digitalValueThresholdOff = 600,
        };

        adcConfig_t adc12bit = {
            .adcMaxValue              = 4095,
            .stepDiff7Bit             = 24,
            .stepDiff14Bit            = 2,
            .stepDiffDirChange        = 24,
            .fsrMinValue              = 160,
            .fsrMaxValue              = 1360,
            .aftertouchMaxValue       = 2400,
            .digitalValueThresholdOn  = 4000,
            .digitalValueThresholdOff = 2400,
        };

        const IO::Analog::Filter::adcType_t _adcType;
        adcConfig_t&                        _adcConfig;
        const size_t                        _stableValueRepetitions;
        const uint32_t                      _fastFilterEnableAfter = 500;
        EMA                                 _emaFilter[MAX_NUMBER_OF_ANALOG];
        uint16_t                            _analogSample[MAX_NUMBER_OF_ANALOG][3]   = {};
        size_t                              _sampleCounter[MAX_NUMBER_OF_ANALOG]     = {};
        valDirection_t                      _lastDirection[MAX_NUMBER_OF_ANALOG]     = {};
        uint16_t                            _lastStableValue[MAX_NUMBER_OF_ANALOG]   = {};
        uint8_t                             _stableSampleCount[MAX_NUMBER_OF_ANALOG] = {};
        uint32_t                            _lastMovementTime[MAX_NUMBER_OF_ANALOG]  = {};
    };    // namespace IO
}    // namespace IO