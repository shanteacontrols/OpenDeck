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

#include "../../board/Board.h"
#include "../../eeprom/Database.h"
#include "Colors.h"

//define extra sysex section for setting/getting led states
#define ledStateSection                 LED_SECTIONS

#define BLINK_TIME_MIN                  0x00
#define BLINK_TIME_MAX                  0x0F

#define START_UP_SWITCH_TIME_MIN        0x00
#define START_UP_SWITCH_TIME_MAX        0x78

#define FADE_TIME_MIN                   0x00
#define FADE_TIME_MAX                   0x0A

#define NUMBER_OF_START_UP_ANIMATIONS   5

class LEDs : Board
{
    public:
    LEDs();
    void init();
    void setState(uint8_t ledNumber, bool state);
    void setState(uint8_t ledNumber, rgb color);
    rgb velocityToColor(uint8_t receivedVelocity, bool blinkEnabled);
    bool velocity2blinkState(uint8_t receivedVelocity);
    void noteToLEDstate(uint8_t receivedNote, uint8_t receivedVelocity);
    void allOn();
    void allOff();
    uint8_t getState(uint8_t ledNumber);
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
