/*

Copyright 2015-2023 Igor Petrovic

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

#include <inttypes.h>
#include "application/util/incdec/IncDec.h"
#include "application/messaging/Messaging.h"

namespace global
{
    class BPM
    {
        public:
        // This class can be used across various application modules but
        // state must be preserved - hence the singleton approach.

        static BPM& instance()
        {
            static BPM bpm;
            return bpm;
        }

        bool increment(uint8_t steps)
        {
            auto newBPM = BPMIncDec::increment(_bpm,
                                               steps,
                                               BPMIncDec::type_t::EDGE);

            if (newBPM != _bpm)
            {
                set(newBPM);
                return true;
            }

            return false;
        }

        bool decrement(uint8_t steps)
        {
            auto newBPM = BPMIncDec::decrement(_bpm,
                                               steps,
                                               BPMIncDec::type_t::EDGE);

            if (newBPM != _bpm)
            {
                set(newBPM);
                return true;
            }

            return false;
        }

        uint8_t value()
        {
            return _bpm;
        }

        uint32_t bpmToUsec(uint32_t bpm)
        {
            return 60000000 / PPQN / bpm;
        }

        private:
        BPM() = default;

        void set(uint8_t bpm)
        {
            _bpm = bpm;

            messaging::event_t event;
            event.systemMessage = messaging::systemMessage_t::MIDI_BPM_CHANGE;
            event.value         = bpm;

            MIDIDispatcher.notify(messaging::eventType_t::SYSTEM, event);
        }

        using BPMIncDec                = util::IncDec<uint8_t, 10, 255>;
        static constexpr uint32_t PPQN = 24;
        uint8_t                   _bpm = 120;
    };
}    // namespace global

#define BPM global::BPM::instance()