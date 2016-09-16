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

#ifndef LEDS_H_
#define LEDS_H_

#include "../../board/Board.h"
#include "../../BitManipulation.h"
#include "../../eeprom/Configuration.h"

#include "LEDcolors.h"

class LEDs : Board {

    public:
    LEDs();
    void init();
    ledColor_t velocity2color(bool blinEnabled, uint8_t receivedVelocity);
    bool velocity2blinkState(uint8_t receivedVelocity);
    void noteToLEDstate(uint8_t receivedNote, uint8_t receivedVelocity);
    void allOn();
    void allOff();
    uint8_t getState(uint8_t ledNumber);
    void setState(uint8_t ledNumber, ledColor_t color, bool blinkMode);
    void setBlinkTime(uint16_t blinkTime);
    void setFadeTime(uint8_t fadeTime);

    private:
    //data processing
    void handleLED(bool newLEDstate, bool blinkMode, uint8_t ledNumber);
    bool checkLEDsOn();
    bool checkLEDsOff();

    //animation
    void oneByOneLED(bool ledDirection, bool singleLED, bool turnOn);
    void startUpAnimation();

};

extern LEDs leds;

#endif
