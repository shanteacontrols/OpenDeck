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

#ifndef HARDWARE_H_
#define HARDWARE_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include "../core/Core.h"
#include "pins/Pins.h"
#include "../Types.h"

#define ANALOG_BUFFER_SIZE  MAX_NUMBER_OF_ANALOG

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
inline encoderPosition_t readEncoder(uint8_t encoderID, uint8_t pairState) __attribute__((always_inline));

uint32_t rTimeMillis();
void wait(uint32_t time);
void disableWatchDog();

class Board {

    public:
    //init
    Board();
    void init();

    //digital
    bool buttonDataAvailable();
    bool encoderDataAvailable();
    bool getButtonState(uint8_t buttonIndex);
    encoderPosition_t getEncoderState(uint8_t encoderID);

    //analog
    bool analogDataAvailable();
    int16_t getAnalogValue(uint8_t analogID);

    protected:
    //LEDs
    uint8_t getLEDstate(uint8_t ledNumber);
    void setLEDstate(uint8_t ledNumber, ledColor_t color, bool blinkMode);
    void setLEDblinkTime(uint16_t blinkTime);
    void setLEDfadeTime(uint8_t transitionSteps);

    private:

    //init
    void initPins();
    void initAnalog();
    void initEncoders();
    void configureTimers();
    void checkBlinkLEDs();
    void ledBlinkingStart();
    void ledBlinkingStop();
    bool ledBlinkingActive();
    void handleLED(uint8_t ledNumber, ledColor_t color, bool blinkMode, ledType_t type);

};

extern Board board;

#endif
