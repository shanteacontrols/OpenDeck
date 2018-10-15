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

#include "board/Board.h"
#include "database/Database.h"
#include "DataTypes.h"
#include "Constants.h"

///
/// \brief LED handling.
/// \defgroup leds LEDs
/// \ingroup interface
/// @{
///
class LEDs
{
    public:
    LEDs(Database &database) : database(database) {}
    void init(bool startUp = true);
    void checkBlinking(bool forceChange = false);
    void setAllOn();
    void setAllOff();
    void setColor(uint8_t ledID, ledColor_t color);
    ledColor_t getColor(uint8_t ledID);
    void setBlinkState(uint8_t ledID, blinkSpeed_t value);
    bool getBlinkState(uint8_t ledID);
    bool setFadeTime(uint8_t transitionSpeed);
    void midiToState(midiMessageType_t messageType, uint8_t data1, uint8_t data2, uint8_t channel, bool local = false);
    void setBlinkType(blinkType_t blinkType);
    blinkType_t getBlinkType();
    void resetBlinking();

    private:
    ledColor_t valueToColor(uint8_t receivedVelocity);
    blinkSpeed_t valueToBlinkSpeed(uint8_t value);
    uint8_t getState(uint8_t ledID);
    void handleLED(uint8_t ledID, bool state, bool rgbLED = false, rgbIndex_t index = rgb_R);
    void startUpAnimation();

    Database            &database;

    ///
    /// \brief Array holding time after which LEDs should blink.
    ///
    uint8_t             blinkTimer[MAX_NUMBER_OF_LEDS] = {};

    ///
    /// \brief Holds currently active LED blink type.
    ///
    blinkType_t         ledBlinkType = blinkType_timer;

    ///
    /// \brief Pointer to array used to check if blinking LEDs should toggle state.
    ///
    const uint8_t*      blinkResetArrayPtr = nullptr;

    ///
    /// \brief Array holding MIDI clock pulses after which LED state is toggled for all possible blink rates.
    ///
    const uint8_t       blinkReset_midiClock[BLINK_SPEEDS] =
    {
        255, //no blinking
        2,
        3,
        4,
        6,
        9,
        12,
        18,
        24,
        36,
        48
    };

    ///
    /// \brief Array holding time indexes (multipled by 100) after which LED state is toggled for all possible blink rates.
    ///
    const uint8_t       blinkReset_timer[BLINK_SPEEDS] =
    {
        0,
        1,
        2,
        3,
        4,
        5,
        6,
        7,
        8,
        9,
        10
    };

    ///
    /// \brief Array used to determine when the blink state for specific blink rate should be changed.
    ///
    uint8_t             blinkCounter[BLINK_SPEEDS] = {};

    ///
    /// \brief Holds last time in miliseconds when LED blinking has been updated.
    ///
    uint32_t            lastLEDblinkUpdateTime = 0;

    ///
    // \brief Holds blink state for each blink speed so that leds are in sync.
    ///
    bool                blinkState[BLINK_SPEEDS] = {};
};