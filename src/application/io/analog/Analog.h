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

#ifndef ANALOG_SUPPORTED
#include "Stub.h"
#else

#include "database/Database.h"
#include "midi/src/MIDI.h"
#include "io/leds/LEDs.h"
#include "io/display/Display.h"
#include "io/common/CInfo.h"

namespace IO
{
    ///
    /// \brief Analog components handling.
    /// \defgroup analog Analog
    /// \ingroup interface
    /// @{
    ///

    class Analog
    {
        public:
        enum class adcType_t : uint8_t
        {
            adc10bit,
            adc12bit
        };

        typedef struct
        {
            const uint16_t adcMaxValue;                 ///< Maxmimum raw ADC value.
            const uint16_t stepDiff7Bit;                ///< Minimum difference between two raw ADC readings to consider that value has been changed for 7-bit MIDI values.
            const uint16_t stepDiff14Bit;               ///< Minimum difference between two raw ADC readings to consider that value has been changed for 14-bit MIDI values.
            const uint16_t stepDiff7BitDirChange;       ///< Same as stepDiff7Bit, only used when the direction is different from the last one.
            const uint16_t stepDiff14BitDirChange;      ///< Same as stepDiff14Bit, only used when the direction is different from the last one.
            const uint16_t fsrMinValue;                 ///< Minimum raw ADC reading for FSR sensors.
            const uint16_t fsrMaxValue;                 ///< Maximum raw ADC reading for FSR sensors.
            const uint16_t aftertouchMaxValue;          ///< Maxmimum raw ADC reading for aftertouch on FSR sensors.
            const uint16_t digitalValueThresholdOn;     ///< Value above which buton connected to analog input is considered pressed.
            const uint16_t digitalValueThresholdOff;    ///< Value below which button connected to analog input is considered released.
        } adcConfig_t;

        enum class type_t : uint8_t
        {
            potentiometerControlChange,
            potentiometerNote,
            fsr,
            button,
            nrpn7b,
            nrpn14b,
            pitchBend,
            cc14bit,
            AMOUNT
        };

        enum class pressureType_t : uint8_t
        {
            velocity,
            aftertouch
        };

        class HWA
        {
            public:
            virtual uint16_t state(size_t index) = 0;
        };

        class Filter
        {
            public:
            virtual bool isFiltered(size_t index, uint16_t value, uint16_t& filteredValue) = 0;
            virtual void reset(size_t index)                                               = 0;
        };

        Analog(HWA&           hwa,
               adcType_t      adcType,
               Filter&        filter,
               Database&      database,
               MIDI&          midi,
               IO::LEDs&      leds,
               Display&       display,
               ComponentInfo& cInfo)
            : hwa(hwa)
            , adcConfig(adcType == adcType_t::adc10bit ? adc10bit : adc12bit)
            , filter(filter)
            , database(database)
            , midi(midi)
            , leds(leds)
            , display(display)
            , cInfo(cInfo)
            , adc7bitStep((adcConfig.adcMaxValue + 1) / 128)
        {}

        void         update();
        void         debounceReset(uint16_t index);
        void         setButtonHandler(void (*fptr)(uint8_t adcIndex, bool state));
        adcConfig_t& config();

        private:
        enum class potDirection_t : uint8_t
        {
            initial,
            decreasing,
            increasing
        };

        adcConfig_t adc10bit = {
            .adcMaxValue              = 1023,
            .stepDiff7Bit             = 6,
            .stepDiff14Bit            = 1,
            .stepDiff7BitDirChange    = 6,
            .stepDiff14BitDirChange   = 12,
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
            .stepDiff7BitDirChange    = 35,
            .stepDiff14BitDirChange   = 8,
            .fsrMinValue              = 160,
            .fsrMaxValue              = 1360,
            .aftertouchMaxValue       = 2400,
            .digitalValueThresholdOn  = 4000,
            .digitalValueThresholdOff = 2400,
        };

        void     checkPotentiometerValue(type_t analogType, uint8_t analogID, uint32_t value);
        void     checkFSRvalue(uint8_t analogID, uint16_t pressure);
        bool     fsrPressureStable(uint8_t analogID);
        bool     getFsrPressed(uint8_t fsrID);
        void     setFsrPressed(uint8_t fsrID, bool state);
        bool     getFsrDebounceTimerStarted(uint8_t fsrID);
        void     setFsrDebounceTimerStarted(uint8_t fsrID, bool state);
        uint32_t calibratePressure(uint32_t value, pressureType_t type);
        bool     digitalStateFromAnalogValue(uint16_t adcValue);

        HWA&           hwa;
        adcConfig_t&   adcConfig;
        Filter&        filter;
        Database&      database;
        MIDI&          midi;
        IO::LEDs&      leds;
        Display&       display;
        ComponentInfo& cInfo;

        void (*buttonHandler)(uint8_t adcIndex, bool state) = nullptr;

        uint16_t       lastAnalogueValue[MAX_NUMBER_OF_ANALOG] = {};
        uint8_t        fsrPressed[MAX_NUMBER_OF_ANALOG]        = {};
        potDirection_t lastDirection[MAX_NUMBER_OF_ANALOG]     = {};
        const uint16_t adc7bitStep;
    };

    /// @}
}    // namespace IO

#endif