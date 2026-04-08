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
    class MidiProgram
    {
        public:
        // This class can be used across various application modules but
        // state must be preserved - hence the singleton approach.

        static MidiProgram& instance()
        {
            static MidiProgram program;
            return program;
        }

        bool incrementProgram(uint8_t channel, uint8_t steps)
        {
            if (channel > 16)
            {
                return false;
            }

            auto newProgram = ProgramIncDec::increment(_program[channel],
                                                       steps + _offset,
                                                       ProgramIncDec::type_t::EDGE);

            if (newProgram != _program[channel])
            {
                _program[channel] = newProgram;
                return true;
            }

            return false;
        }

        bool decrementProgram(uint8_t channel, uint8_t steps)
        {
            if (channel > 16)
            {
                return false;
            }

            auto newProgram = ProgramIncDec::decrement(_program[channel],
                                                       steps + _offset,
                                                       ProgramIncDec::type_t::EDGE);

            if (newProgram != _program[channel])
            {
                _program[channel] = newProgram;
                return true;
            }

            return false;
        }

        uint8_t program(uint8_t channel)
        {
            if (channel > 16)
            {
                return 0;
            }

            return _program[channel];
        }

        bool setProgram(uint8_t channel, uint8_t program)
        {
            if (channel > 16)
            {
                return false;
            }

            program += _offset;

            if (program > 127)
            {
                return false;
            }

            _program[channel] = program;
            return true;
        }

        bool incrementOffset(uint8_t steps)
        {
            auto newOffset = OffsetIncDec::increment(_offset,
                                                     steps,
                                                     OffsetIncDec::type_t::EDGE);

            if (newOffset != _offset)
            {
                setOffset(newOffset);
                return true;
            }

            return false;
        }

        bool decrementOffset(uint8_t steps)
        {
            auto newOffset = OffsetIncDec::decrement(_offset,
                                                     steps,
                                                     OffsetIncDec::type_t::EDGE);

            if (newOffset != _offset)
            {
                setOffset(newOffset);
                return true;
            }

            return false;
        }

        void setOffset(uint8_t offset)
        {
            _offset = offset;

            messaging::Event event = {};
            event.systemMessage    = messaging::systemMessage_t::MIDI_PROGRAM_OFFSET_CHANGE;
            event.value            = offset;

            MidiDispatcher.notify(messaging::eventType_t::SYSTEM, event);
        }

        uint8_t offset()
        {
            return _offset;
        }

        private:
        MidiProgram() = default;

        using ProgramIncDec = util::IncDec<uint8_t, 0, 127>;
        using OffsetIncDec  = util::IncDec<uint8_t, 0, 127>;

        // channels use 1-16 range
        std::array<uint8_t, 17> _program = {};
        uint8_t                 _offset  = 0;
    };
}    // namespace global

#define MidiProgram global::MidiProgram::instance()