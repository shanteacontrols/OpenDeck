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
    class Analog
    {
        public:
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

        enum class adcType_t : uint16_t
        {
            adc10bit = 1023,
            adc12bit = 4095
        };

        class HWA
        {
            public:
            //should return true if the value has been refreshed, false otherwise
            virtual bool value(size_t index, uint16_t& value) = 0;
        };

        class Filter
        {
            public:
            virtual Analog::adcType_t adcType()                                                                              = 0;
            virtual bool              isFiltered(size_t index, Analog::type_t type, uint16_t value, uint16_t& filteredValue) = 0;
            virtual void              reset(size_t index)                                                                    = 0;
        };

        using buttonHandler_t = void (*)(uint8_t adcIndex, bool state);

        Analog(HWA&           hwa,
               Filter&        filter,
               Database&      database,
               MIDI&          midi,
               IO::LEDs&      leds,
               Display&       display,
               ComponentInfo& cInfo)
            : hwa(hwa)
            , filter(filter)
            , database(database)
            , midi(midi)
            , leds(leds)
            , display(display)
            , cInfo(cInfo)
        {
            //make sure the first value is sent even if 0
            for (size_t i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
                lastValue[i] = 0xFFFF;
        }

        void      update();
        void      debounceReset(uint16_t index);
        void      setButtonHandler(buttonHandler_t handler);
        adcType_t adcType();

        private:
        typedef struct
        {
            type_t   type;
            uint16_t lowerLimit;
            uint16_t upperLimit;
            uint16_t midiID;
            uint8_t  channel;
            bool     inverted;
        } analogDescriptor_t;

        bool checkPotentiometerValue(uint8_t analogID, analogDescriptor_t& descriptor, uint16_t& value);
        bool checkFSRvalue(uint8_t analogID, analogDescriptor_t& descriptor, uint16_t& value);
        void sendMessage(uint8_t analogID, analogDescriptor_t& descriptor, uint16_t value);

        HWA&           hwa;
        Filter&        filter;
        Database&      database;
        MIDI&          midi;
        IO::LEDs&      leds;
        Display&       display;
        ComponentInfo& cInfo;

        buttonHandler_t buttonHandler                    = nullptr;
        uint8_t         fsrPressed[MAX_NUMBER_OF_ANALOG] = {};
        uint16_t        lastValue[MAX_NUMBER_OF_ANALOG]  = {};
    };
}    // namespace IO

#endif