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

#ifdef HW_SUPPORT_ADC

#include <stdio.h>
#include <stdlib.h>
#include "core/src/Timing.h"
#include "io/analog/Analog.h"
#include "core/src/util/Util.h"
#include "core/src/util/Filters.h"
#include "core/src/MCU.h"

namespace IO
{
    class AnalogFilter : public Analog::Filter
    {
        public:
        static_assert(CORE_MCU_ADC_MAX_VALUE == 1023 || CORE_MCU_ADC_MAX_VALUE == 4095, "Unsupported ADC resolution");

        AnalogFilter()
            : _adcConfig(CORE_MCU_ADC_MAX_VALUE == 1023 ? _adc10bit : _adc12bit)
            , STEP_DIFF_7BIT((_adcConfig.ADC_MAX_VALUE - _adcConfig.ADC_MIN_VALUE) / 128)
        {
            for (size_t i = 0; i < IO::Analog::Collection::size(); i++)
            {
                _lastStableValue[i] = 0xFFFF;
            }
        }

        bool isFiltered(size_t index, descriptor_t& descriptor) override
        {
            const uint16_t ADC_MIN_VALUE = lowerOffsetRaw(descriptor.lowerOffset);
            const uint16_t ADC_MAX_VALUE = upperOffsetRaw(descriptor.upperOffset);

            descriptor.value = core::util::CONSTRAIN(descriptor.value,
                                                     ADC_MIN_VALUE,
                                                     ADC_MAX_VALUE);

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

            const bool FAST_FILTER    = (core::timing::ms() - _lastStableMovementTime[index]) < FAST_FILTER_ENABLE_AFTER_MS;
            const bool DIRECTION      = descriptor.value >= _lastStableValue[index];
            const auto OLD_MIDI_VALUE = core::util::MAP_RANGE(static_cast<uint32_t>(_lastStableValue[index]),
                                                              static_cast<uint32_t>(ADC_MIN_VALUE),
                                                              static_cast<uint32_t>(ADC_MAX_VALUE),
                                                              static_cast<uint32_t>(0),
                                                              static_cast<uint32_t>(descriptor.maxValue));
            int16_t    stepDiff       = 1;

            if (((DIRECTION != lastStableDirection(index)) || !FAST_FILTER) && ((OLD_MIDI_VALUE != 0) && (OLD_MIDI_VALUE != descriptor.maxValue)))
            {
                stepDiff = STEP_DIFF_7BIT * 2;
            }

            if (abs(static_cast<int16_t>(descriptor.value) - static_cast<int16_t>(_lastStableValue[index])) < stepDiff)
            {
#ifdef ANALOG_FILTER_MEDIAN
                _medianFilter[index].reset();
#endif

                return false;
            }

#ifdef ANALOG_FILTER_MEDIAN
            if (!FAST_FILTER)
            {
                auto median = _medianFilter[index].value(descriptor.value);

                if (median.has_value())
                {
                    descriptor.value = median.value();
                }
                else
                {
                    return false;
                }
            }
#endif

#ifdef ANALOG_FILTER_EMA
            descriptor.value = _emaFilter[index].value(descriptor.value);
#endif

            const auto MIDI_VALUE = core::util::MAP_RANGE(static_cast<uint32_t>(descriptor.value),
                                                          static_cast<uint32_t>(ADC_MIN_VALUE),
                                                          static_cast<uint32_t>(ADC_MAX_VALUE),
                                                          static_cast<uint32_t>(0),
                                                          static_cast<uint32_t>(descriptor.maxValue));

            if (MIDI_VALUE == OLD_MIDI_VALUE)
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
                _lastStableMovementTime[index] = core::timing::ms();
            }

            if (descriptor.type == Analog::type_t::FSR)
            {
                descriptor.value = core::util::MAP_RANGE(core::util::CONSTRAIN(static_cast<uint32_t>(descriptor.value),
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
#ifdef ANALOG_FILTER_MEDIAN
                _medianFilter[index].reset();
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

            core::util::BIT_WRITE(_lastStableDirection[arrayIndex], analogIndex, state);
        }

        bool lastStableDirection(size_t index)
        {
            uint8_t arrayIndex  = index / 8;
            uint8_t analogIndex = index - 8 * arrayIndex;

            return core::util::BIT_READ(_lastStableDirection[arrayIndex], analogIndex);
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
            .ADC_MAX_VALUE               = 3950,
            .FSR_MIN_VALUE               = 160,
            .FSR_MAX_VALUE               = 1360,
            .AFTERTOUCH_MAX_VALUE        = 2400,
            .DIGITAL_VALUE_THRESHOLD_ON  = 3950,
            .DIGITAL_VALUE_THRESHOLD_OFF = 2400,
        };

        adcConfig_t&              _adcConfig;
        const uint16_t            STEP_DIFF_7BIT;
        static constexpr uint32_t FAST_FILTER_ENABLE_AFTER_MS = 50;
        static constexpr size_t   MEDIAN_SAMPLE_COUNT         = 3;
        static constexpr size_t   MEDIAN_MIDDLE_VALUE         = 1;

// some filtering is needed for adc only
#ifdef ANALOG_FILTER_EMA
        core::util::EMAFilter<uint16_t, 50> _emaFilter[IO::Analog::Collection::size()];
#endif

#ifdef ANALOG_FILTER_MEDIAN
        core::util::MedianFilter<uint16_t, 3> _medianFilter[IO::Analog::Collection::size()];
#else
        uint16_t _analogSample[IO::Analog::Collection::size()] = {};
#endif
        uint32_t _lastStableMovementTime[IO::Analog::Collection::size()] = {};

        uint8_t  _lastStableDirection[IO::Analog::Collection::size() / 8 + 1] = {};
        uint16_t _lastStableValue[IO::Analog::Collection::size()]             = {};
    };    // namespace IO
}    // namespace IO

#else
#include "stub/Filter.h"
#endif
