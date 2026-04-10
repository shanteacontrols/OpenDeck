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

#include "zlibs/utils/filters/filters.h"
#include "zlibs/utils/misc/bit.h"
#include "zlibs/utils/misc/numeric.h"

#include <array>
#include <zephyr/kernel.h>

namespace io::analog
{
    class FilterHw : public Filter
    {
        public:
        FilterHw(uint8_t adcBits)
            : _adcConfig(adcBits == 12 ? ADC_12BIT : ADC_10BIT)
            , STEP_DIFF_7BIT((_adcConfig.ADC_MAX_VALUE - _adcConfig.ADC_MIN_VALUE) / 128)
        {
            for (size_t i = 0; i < io::analog::Collection::SIZE(); i++)
            {
                _lastValue[i] = 0xFFFF;
            }
        }

        bool isFiltered(size_t index, Descriptor& descriptor) override
        {
            const uint16_t ADC_MIN_VALUE = lowerOffsetRaw(descriptor.lowerOffset);
            const uint16_t ADC_MAX_VALUE = upperOffsetRaw(descriptor.upperOffset);

            descriptor.value = zlibs::utils::misc::constrain(descriptor.value,
                                                             ADC_MIN_VALUE,
                                                             ADC_MAX_VALUE);

            // avoid full filtering in this case for faster response
            if (descriptor.type == type_t::BUTTON)
            {
                return isButtonFiltered(index, descriptor);
            }

            const bool FAST_FILTER    = (k_uptime_get() - _lastMovementTime[index]) < FAST_FILTER_ENABLE_AFTER_MS;
            const bool DIRECTION      = descriptor.value >= _lastValue[index];
            const auto OLD_MIDI_VALUE = zlibs::utils::misc::map_range(static_cast<uint32_t>(_lastValue[index]),
                                                                      static_cast<uint32_t>(ADC_MIN_VALUE),
                                                                      static_cast<uint32_t>(ADC_MAX_VALUE),
                                                                      static_cast<uint32_t>(0),
                                                                      static_cast<uint32_t>(descriptor.maxValue));
            int16_t    stepDiff       = 1;

            if (((DIRECTION != lastDirection(index)) || !FAST_FILTER) && ((OLD_MIDI_VALUE != 0) && (OLD_MIDI_VALUE != descriptor.maxValue)))
            {
                stepDiff = STEP_DIFF_7BIT * 2;
            }

            if (abs(static_cast<int16_t>(descriptor.value) - static_cast<int16_t>(_lastValue[index])) < stepDiff)
            {
                _medianFilter[index].reset();
                return false;
            }

            if (!FAST_FILTER)
            {
                auto median = _medianFilter[index].filter_value(descriptor.value);

                if (median.has_value())
                {
                    descriptor.value = median.value();
                }
                else
                {
                    return false;
                }
            }

            descriptor.value = _emaFilter[index].value(descriptor.value);

            const auto MIDI_VALUE = zlibs::utils::misc::map_range(static_cast<uint32_t>(descriptor.value),
                                                                  static_cast<uint32_t>(ADC_MIN_VALUE),
                                                                  static_cast<uint32_t>(ADC_MAX_VALUE),
                                                                  static_cast<uint32_t>(0),
                                                                  static_cast<uint32_t>(descriptor.maxValue));

            if (MIDI_VALUE == OLD_MIDI_VALUE)
            {
                return false;
            }

            setlastDirection(index, DIRECTION);
            _lastValue[index] = descriptor.value;

            // when edge values are reached, disable fast filter by resetting last movement time
            if ((MIDI_VALUE == 0) || (MIDI_VALUE == descriptor.maxValue))
            {
                _lastMovementTime[index] = 0;
            }
            else
            {
                _lastMovementTime[index] = k_uptime_get();
            }

            if (descriptor.type == type_t::FSR)
            {
                descriptor.value = zlibs::utils::misc::map_range(
                    zlibs::utils::misc::constrain(static_cast<uint32_t>(descriptor.value),
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

        void reset(size_t index) override
        {
            if (index < io::analog::Collection::SIZE())
            {
                _medianFilter[index].reset();
                _lastMovementTime[index] = 0;
            }

            _lastValue[index] = 0xFFFF;
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

        static constexpr adcConfig_t ADC_10BIT = {
            .ADC_MIN_VALUE               = 10,
            .ADC_MAX_VALUE               = 1000,
            .FSR_MIN_VALUE               = 40,
            .FSR_MAX_VALUE               = 340,
            .AFTERTOUCH_MAX_VALUE        = 600,
            .DIGITAL_VALUE_THRESHOLD_ON  = 800,
            .DIGITAL_VALUE_THRESHOLD_OFF = 200,
        };

        static constexpr adcConfig_t ADC_12BIT = {
            .ADC_MIN_VALUE               = 64,
            .ADC_MAX_VALUE               = 3950,
            .FSR_MIN_VALUE               = 160,
            .FSR_MAX_VALUE               = 1360,
            .AFTERTOUCH_MAX_VALUE        = 2400,
            .DIGITAL_VALUE_THRESHOLD_ON  = 3000,
            .DIGITAL_VALUE_THRESHOLD_OFF = 1000,
        };

        static constexpr uint32_t FAST_FILTER_ENABLE_AFTER_MS = 50;
        static constexpr size_t   MEDIAN_SAMPLE_COUNT         = 3;
        static constexpr size_t   MEDIAN_MIDDLE_VALUE         = 1;
        static constexpr uint8_t  BUTTON_TYPE_DEBOUNCED_MASK  = 0b00000010;
        static constexpr size_t   STORAGE_SIZE                = io::analog::Collection::SIZE() ? io::analog::Collection::SIZE() : 1;

        const adcConfig_t&                                                         _adcConfig;
        const uint16_t                                                             STEP_DIFF_7BIT;
        std::array<zlibs::utils::filters::EmaFilter<uint16_t, 50>, STORAGE_SIZE>   _emaFilter        = {};
        std::array<zlibs::utils::filters::MedianFilter<uint16_t, 3>, STORAGE_SIZE> _medianFilter     = {};
        std::array<uint32_t, STORAGE_SIZE>                                         _lastMovementTime = {};
        std::array<uint8_t, STORAGE_SIZE / 8 + 1>                                  _lastDirection    = {};
        std::array<uint16_t, STORAGE_SIZE>                                         _lastValue        = {};

        void setlastDirection(size_t index, bool state)
        {
            uint8_t arrayIndex  = index / 8;
            uint8_t analogIndex = index - 8 * arrayIndex;

            zlibs::utils::misc::bit_write(_lastDirection[arrayIndex], analogIndex, state);
        }

        bool lastDirection(size_t index)
        {
            uint8_t arrayIndex  = index / 8;
            uint8_t analogIndex = index - 8 * arrayIndex;

            return zlibs::utils::misc::bit_read(_lastDirection[arrayIndex], analogIndex);
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

            auto unmaskedLastValue = _lastValue[index] & ~BUTTON_TYPE_DEBOUNCED_MASK;

            if (newValue != unmaskedLastValue)
            {
                _lastValue[index]        = newValue;
                _lastMovementTime[index] = k_uptime_get();

                // not debounced yet
                return false;
            }

            if ((k_uptime_get() - _lastMovementTime[index]) < io::buttons::DEBOUNCE_TIME_MS)
            {
                return false;
            }

            if (_lastValue[index] & BUTTON_TYPE_DEBOUNCED_MASK)
            {
                // button debounced and event already sent
                return false;
            }

            _lastValue[index] |= BUTTON_TYPE_DEBOUNCED_MASK;
            descriptor.value = newValue;

            return true;
        }
    };
}    // namespace io::analog
