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

#pragma once

#include "../../core/Core.h"
#include "constants/Constants.h"
#include "../Common.h"
#include "pins/Pins.h"
#include "pins/Map.h"

//function prototypes
inline void setAnalogPin(uint8_t muxNumber) __attribute__((always_inline));
inline void setMuxInput(uint8_t muxInput) __attribute__((always_inline));
inline void ledRowsOff() __attribute__((always_inline));
inline void ledRowOn(uint8_t rowNumber, uint8_t intensity) __attribute__((always_inline));
inline void checkLEDs() __attribute__((always_inline));
inline void setBlinkState(uint8_t ledNumber, bool state) __attribute__((always_inline));
inline void activateInputColumn(uint8_t column) __attribute__((always_inline));
inline void activateOutputColumn(uint8_t column) __attribute__((always_inline));
inline void storeDigitalIn(uint8_t column, uint8_t bufferIndex) __attribute__((always_inline));
inline int8_t readEncoder(uint8_t encoderID, uint8_t pairState) __attribute__((always_inline));

class Board : BoardCommon
{
    public:
    //init
    Board();
    void init();

    //buttons
    bool buttonDataAvailable();
    bool getButtonState(uint8_t buttonIndex);

    //analog
    bool analogDataAvailable();
    uint16_t getAnalogValue(uint8_t analogID);
    void resetADC();

    //encoders
    uint8_t getEncoderPair(uint8_t buttonID);
    bool encoderDataAvailable();
    int8_t getEncoderState(uint8_t encoderID);

    //leds
    uint8_t getRGBaddress(uint8_t rgbID, rgbIndex_t index);
    uint8_t getRGBID(uint8_t ledNumber);

    void reboot(rebootType_t type);

    private:
    //init
    void initPins();
    void initAnalog();
    void initEncoders();
    void configureTimers();
    bool copyInputMatrixBuffer();
    void checkInputMatrixBufferCopy();
    bool getHysteresisState(uint8_t analogID);
    void setHysteresisState(uint8_t analogID, bool state);
};

extern Board board;
