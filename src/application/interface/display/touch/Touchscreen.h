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

#include "DataTypes.h"

///
/// \brief Touchscreen control.
/// \defgroup interfaceLCDTouch Touchscreen
/// \ingroup interfaceLCD
/// @{

class Touchscreen
{
    public:
    Touchscreen();
    bool init(ts_t touchscreenType);
    void update();
    void setPage(uint8_t pageID);
    uint8_t getPage();
    void setButtonHandler(void(*fptr)(uint8_t index, bool state));

    friend void sdw_init(Touchscreen &base);
    friend bool sdw_update(Touchscreen &base);

    protected:
    void        (*buttonHandler)(uint8_t index, bool state);
    bool        (*displayUpdatePtr)(Touchscreen &instance);
    void        (*setPagePtr)(uint8_t pageID);
    uint8_t     displayRxBuffer[TOUCHSCREEN_RX_BUFFER_SIZE];
    uint8_t     bufferIndex_rx;
    uint8_t     activeButtonID;
    bool        activeButtonState;
    uint8_t     activePage;
};

/// @}
