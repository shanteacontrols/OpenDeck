/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015, 2016 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MIDIHELPER_H_
#define MIDIHELPER_H_

#include "../../Types.h"
#include <inttypes.h>

class MIDI {

    public:
    MIDI();
    void init();
    void checkInput();

    void sendMIDInote(uint8_t note, bool state, uint8_t _velocity);
    void sendProgramChange(uint8_t program);
    void sendControlChange(uint8_t ccNumber, uint8_t value);
    void sendSysEx(uint8_t *sysExArray, uint8_t size, bool arrayContainsBoundaries, bool usbSend = true, bool dinSend = false);

    private:
    uint32_t            lastSysExMessageTime;
    midiInterfaceType_t source;

};

extern MIDI midi;

#endif
