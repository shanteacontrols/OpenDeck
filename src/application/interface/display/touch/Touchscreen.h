/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

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

///
/// \brief Touchscreen control.
/// \defgroup interfaceLCDTouch Touchscreen
/// \ingroup interfaceLCD
/// @{

class Touchscreen
{
    public:
    Touchscreen();
    static void init();
    static void update();
    void handleRead(int16_t(*fptr)());
    void handleWrite(int8_t(*fptr)(uint8_t data));

    private:
    static void read();
    static void parse();
    static void process(uint8_t buttonID, bool buttonState);
    int16_t (*sendReadCallback)();
    int8_t (*sendWriteCallback)(uint8_t data);
};

extern Touchscreen touchscreen;

/// @}
