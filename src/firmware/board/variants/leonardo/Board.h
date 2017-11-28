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

#ifdef BOARD_A_LEO

#pragma once

#include "../../common/Common.h"
#include "pins/Pins.h"
#include "pins/Map.h"
#include "../../../interface/digital/output/leds/DataTypes.h"

///
/// \addtogroup board
/// @{
///

class Board : BoardInterface
{
    public:
    //init
    Board();
    void init();

    //digital in
    bool digitalInputDataAvailable();
    void continueDigitalInReadout();

    //encoders
    uint8_t getEncoderPair(uint8_t buttonID);
    int8_t getEncoderState(uint8_t encoderID);

    //buttons
    bool getButtonState(uint8_t buttonID);

    //analog
    bool analogDataAvailable();
    int16_t getAnalogValue(uint8_t analogID);
    void continueADCreadout();

    //leds
    uint8_t getRGBaddress(uint8_t rgbID, rgbIndex_t index);
    uint8_t getRGBID(uint8_t ledNumber);

    void reboot(rebootType_t type);

    private:
    //init
    void initPins();
    void initAnalog();
    void initEncoders();
    void initUSB_MIDI();
    void configureTimers();
    int8_t readEncoder(uint8_t encoderID, uint8_t pairState);
};

extern Board board;
extern MIDI midi;

/// @}

#endif