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

#ifdef DISPLAY_SUPPORTED

#include "../../board/Board.h"
#include "../../board/avr/display/u8g2_wrapper.h"
#include "Config.h"
#include "DataTypes.h"
#include "strings/Strings.h"
#include "Macros.h"

///
/// \brief LCD control.
/// \defgroup interfaceLCD LCD
/// \ingroup interface
/// @{

class Display
{
    public:
    Display();
    void init(displayController controller, displayResolution resolution);
    bool update();
    void displayWelcomeMessage();
    void setDirectWriteState(bool state);
    void setupHomeScreen();
    void displayFirmwareUpdated();
    void clearAll();
    void clearRow(uint8_t row);
    void updateText(uint8_t row, lcdTextType_t textType, uint8_t startIndex);
    uint8_t getTextCenter(uint8_t textSize);

    lcdTextType_t getActiveTextType();

    private:
    void updateScrollStatus(uint8_t row);
    void updateTempTextStatus();

    ///
    /// \brief Holds time index LCD message was shown.
    ///
    uint32_t        messageDisplayTime;

    ///
    /// \brief Holds last time LCD was updated.
    /// LCD isn't updated in real-time but after defined amount of time (see LCD_REFRESH_TIME).
    ///
    uint32_t        lastLCDupdateTime;

    ///
    /// \brief Holds active text type on display.
    /// Enumerated type (see lcdTextType_t enumeration).
    ///
    lcdTextType_t   activeTextType;

    ///
    /// \brief Array holding still LCD text for each LCD row.
    ///
    char            lcdRowStillText[LCD_HEIGHT_MAX][STRING_BUFFER_SIZE];

    ///
    /// \brief Array holding temp LCD text for each LCD row.
    ///
    char            lcdRowTempText[LCD_HEIGHT_MAX][STRING_BUFFER_SIZE];

    ///
    /// \brief Array holding true of false value representing the change of character at specific location on LCD row.
    /// \warning This variables assume there can be no more than 32 characters per LCD row.
    ///
    uint32_t        charChange[LCD_WIDTH_MAX];

    ///
    /// \brief Structure array holding scrolling information for all LCD rows.
    ///
    scrollEvent_t   scrollEvent[LCD_HEIGHT_MAX];

    ///
    /// \brief Holds last time text has been scrolled on display.
    ///
    uint32_t        lastScrollTime;

    ///
    /// \brief Holds state of direct LCD writing (true/enabled, false/disabled).
    ///
    bool            directWriteState;
};

extern Display display;

/// @}

#endif