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

#include "DataTypes.h"
#include "Constants.h"
#include "Colors.h"
#include "../../core/Core.h"

class LEDs
{
    public:
    LEDs();
    void init();
    void setState(uint8_t ledNumber, bool state);
    void setState(uint8_t ledNumber, rgbValue_t color);
    void setAllOn();
    void setAllOff();
    uint8_t getState(uint8_t ledNumber);
    void setBlinkTime(uint16_t blinkTime);
    void setFadeTime(uint8_t transitionSpeed);
    rgbValue_t velocityToColor(uint8_t receivedVelocity, bool blinkEnabled);
    bool velocityToblinkState(uint8_t receivedVelocity);
    void noteToState(uint8_t receivedNote, uint8_t receivedVelocity, bool ledID = false, bool local = false);

    private:
    //data processing
    void handleLED(uint8_t ledNumber, bool state, bool blinkMode);
    void handleLED(uint8_t ledNumber, rgbValue_t color, bool blinkMode);
    void startBlinking();
    void stopBlinking();
    bool blinkingActive();
    void checkBlinkLEDs();
    bool allLEDsOn();
    bool allLEDsOff();
    void setSingleLED(uint8_t ledNumber, bool state, bool blinkMode);
    void setRGBled(uint8_t ledNumber, rgbValue_t color, bool blinkMode);

    //animation
    void oneByOne(bool ledDirection, bool singleLED, bool turnOn);
    void startUpAnimation();
};

extern LEDs leds;
