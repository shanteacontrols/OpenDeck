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
#include "board/board.h"

#include "core/util/util.h"

namespace io::encoders
{
    class HwaHw : public Hwa
    {
        public:
        HwaHw() = default;

        bool state(size_t index, uint8_t& numberOfReadings, uint16_t& states) override
        {
            if (!board::io::digital_in::state(board::io::digital_in::encoderComponentFromEncoder(index,
                                                                                                 board::io::digital_in::encoderComponent_t::A),
                                              _dInReadA))
            {
                return false;
            }

            if (!board::io::digital_in::state(board::io::digital_in::encoderComponentFromEncoder(index,
                                                                                                 board::io::digital_in::encoderComponent_t::B),
                                              _dInReadB))
            {
                return false;
            }

            numberOfReadings = _dInReadA.count > _dInReadB.count ? _dInReadA.count : _dInReadB.count;

            /*
                Construct encoder pair readings:
                * Encoder signal is made of A and B signals
                * Take each bit of A signal and B signal and append it to states variable in order
                * Latest encoder readings should be in LSB bits
            */

            for (uint8_t i = 0; i < numberOfReadings; i++)
            {
                core::util::BIT_WRITE(states, (i * 2) + 1, (_dInReadA.readings >> i & 0x01));
                core::util::BIT_WRITE(states, i * 2, (_dInReadB.readings >> i & 0x01));
            }

            return true;
        }

        private:
        board::io::digital_in::Readings _dInReadA;
        board::io::digital_in::Readings _dInReadB;
    };
}    // namespace io::encoders
