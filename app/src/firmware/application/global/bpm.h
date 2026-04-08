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

#include "application/util/incdec/inc_dec.h"
#include "application/messaging/messaging.h"

namespace global
{
    class Bpm
    {
        public:
        // This class can be used across various application modules but
        // state must be preserved - hence the singleton approach.

        static Bpm& instance()
        {
            static Bpm bpm;
            return bpm;
        }

        bool increment(uint8_t steps)
        {
            auto newBpm = BpmIncDec::increment(_bpm,
                                               steps,
                                               BpmIncDec::type_t::EDGE);

            if (newBpm != _bpm)
            {
                set(newBpm);
                return true;
            }

            return false;
        }

        bool decrement(uint8_t steps)
        {
            auto newBpm = BpmIncDec::decrement(_bpm,
                                               steps,
                                               BpmIncDec::type_t::EDGE);

            if (newBpm != _bpm)
            {
                set(newBpm);
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
        Bpm() = default;

        using BpmIncDec = util::IncDec<uint8_t, 10, 255>;

        static constexpr uint32_t PPQN = 24;

        uint8_t _bpm = 120;

        void set(uint8_t bpm)
        {
            _bpm = bpm;

            messaging::Event event = {};
            event.systemMessage    = messaging::systemMessage_t::MIDI_BPM_CHANGE;
            event.value            = bpm;

            MidiDispatcher.notify(messaging::eventType_t::SYSTEM, event);
        }
    };
}    // namespace global

#define Bpm global::Bpm::instance()