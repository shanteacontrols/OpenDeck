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

#include <inttypes.h>
#include <stddef.h>
#include <array>
#include "util/incdec/IncDec.h"

namespace Global
{
    class MIDIProgram
    {
        public:
        // This class can be used across various application modules but
        // state must be preserved - hence the singleton approach.

        static MIDIProgram& instance()
        {
            static MIDIProgram program;
            return program;
        }

        bool increment(uint8_t channel, uint8_t steps)
        {
            if (channel > 16)
            {
                return false;
            }

            auto newProgram = ProgramIncDec::increment(_program[channel],
                                                       steps,
                                                       ProgramIncDec::type_t::EDGE);

            if (newProgram != _program[channel])
            {
                _program[channel] = newProgram;
                return true;
            }

            return false;
        }

        bool decrement(uint8_t channel, uint8_t steps)
        {
            if (channel > 16)
            {
                return false;
            }

            auto newProgram = ProgramIncDec::decrement(_program[channel],
                                                       steps,
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

            if (program > 127)
            {
                return false;
            }

            _program[channel] = program;
            return true;
        }

        private:
        MIDIProgram() = default;

        using ProgramIncDec = Util::IncDec<uint8_t, 0, 127>;

        // channels use 1-16 range
        std::array<uint8_t, 17> _program;
    };
}    // namespace Global

#define MIDIProgram Global::MIDIProgram::instance()