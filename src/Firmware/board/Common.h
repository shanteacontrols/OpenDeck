/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2017 Igor Petrovic

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

#pragma once

#include "DataTypes.h"
#include "Import.h"

class BoardCommon
{
    public:
    virtual void reboot(rebootType_t type);
    virtual bool buttonDataAvailable() = 0;
    virtual bool getButtonState(uint8_t buttonIndex) = 0;
    virtual bool analogDataAvailable() = 0;
    virtual uint16_t getAnalogValue(uint8_t analogID) = 0;
    virtual bool encoderDataAvailable() = 0;
    virtual int8_t getEncoderState(uint8_t encoderID) = 0;
};
