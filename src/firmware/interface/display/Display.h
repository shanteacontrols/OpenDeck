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

#include "../../board/Board.h"
#include "../../board/avr/display/u8g2_wrapper.h"
#include "Config.h"
#include "DataTypes.h"
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
    bool init(displayController_t controller, displayResolution_t resolution);
    bool update();
    void displayWelcomeMessage();
    void displayVinfo(bool newFw);
    void displayHome();
    void displayMIDIevent(displayEventType_t type, midiMessageTypeDisplay_t message, uint16_t byte1, uint16_t byte2, uint8_t byte3);
    void setAlternateNoteDisplay(bool state);
    void setOctaveNormalization(int8_t value);
    void setDirectWriteState(bool state);
    void clearAll();
    void clearRow(uint8_t row);
    void updateText(uint8_t row, lcdTextType_t textType, uint8_t startIndex);
    uint8_t getTextCenter(uint8_t textSize);
    lcdTextType_t getActiveTextType();
    void setRetentionState(bool state);
    void setRetentionTime(uint32_t time);

    private:
    void updateScrollStatus(uint8_t row);
    void updateTempTextStatus();
    void clearMIDIevent(displayEventType_t type);

    ///
    /// \brief Holds last time index MIDI message was shown for specific event type (in or out).
    ///
    uint32_t        lastMIDIMessageDisplayTime[2];

    ///
    /// \brief Holds true if MIDI input or output message is shown on display.
    ///
    bool            midiMessageDisplayed[2];

    ///
    /// \brief Holds time after which MIDI message should be cleared on display if retention is disabled.
    ///
    uint32_t        MIDImessageRetentionTime;

    ///
    /// \brief Holds last time index info message was shown.
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
    uint32_t        charChange[LCD_HEIGHT_MAX];

    ///
    /// \brief Structure array holding scrolling information for all LCD rows.
    ///
    scrollEvent_t   scrollEvent[LCD_HEIGHT_MAX];

    ///
    /// \brief Holds last time text has been scrolled on display.
    ///
    uint32_t        lastScrollTime;

    ///
    /// \brief Holds value by which actual octave is being subtracted when showing octave on display.
    ///
    int8_t          octaveNormalization;

    ///
    /// \brief If set to true, note number will be shown on display (0-127), otherwise, note and octave
    /// will be displayed (i.e. C#4).
    bool            alternateNoteDisplay;

    ///
    /// \brief Holds true if direct LCD writing is enabled, false otherwise.
    ///
    bool            directWriteState;

    ///
    /// \brief Holds resolution of configured screen.
    ///
    displayResolution_t resolution;

    ///
    /// \brief Holds true if display has been initialized.
    ///
    bool            initDone;

    ///
    /// \brief Holds message retention state.
    /// If true, last MIDI message will be shown on display indefinately until it's
    /// overwritten by a new one.
    ///
    bool            retentionState;
};

extern Display display;
extern StringBuffer stringBuffer;

/// @}
