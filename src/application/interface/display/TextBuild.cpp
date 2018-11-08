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

#include "Display.h"
#include "strings/Strings.h"
#include "../../Version.h"
#include "Layout.h"
#include "midi/src/MIDI.h"
#include "core/src/general/Timing.h"

void Display::displayWelcomeMessage()
{
    uint8_t charIndex = 0;
    uint8_t location = 0;
    uint8_t startRow;

    U8X8::clearDisplay();

    switch(U8X8::getRows())
    {
        case 4:
        startRow = 1;
        break;

        default:
        startRow = 0;
        break;
    }

    char* string;

    stringBuffer.startLine();
    stringBuffer.appendText_P(deviceName_string);
    stringBuffer.endLine();
    location = getTextCenter(stringBuffer.getSize());
    charIndex = 0;
    string = stringBuffer.getString();

    while (string[charIndex] != '\0')
    {
        U8X8::drawGlyph(location+charIndex, rowMap[resolution][startRow], string[charIndex]);
        charIndex++;
    }

    wait_ms(1000);

    stringBuffer.startLine();
    stringBuffer.appendText_P(welcome_string);
    stringBuffer.endLine();
    location = getTextCenter(stringBuffer.getSize());
    charIndex = 0;
    string = stringBuffer.getString();

    while (string[charIndex] != '\0')
    {
        U8X8::drawGlyph(location+charIndex, rowMap[resolution][startRow+1], string[charIndex]);
        wait_ms(50);
        charIndex++;
    }

    wait_ms(2000);
}

void Display::displayVinfo(bool newFw)
{
    uint8_t startRow;

    U8X8::clearDisplay();

    switch(U8X8::getRows())
    {
        case 4:
        startRow = 1;
        break;

        default:
        startRow = 0;
        break;
    }

    stringBuffer.startLine();
    stringBuffer.appendText_P(versionInfo_string);
    stringBuffer.endLine();

    updateText(startRow, lcdText_temp, getTextCenter(stringBuffer.getSize()));

    stringBuffer.startLine();
    stringBuffer.appendText_P(fwVersion_string);
    stringBuffer.appendInt(SW_VERSION_MAJOR);
    stringBuffer.appendText(".");
    stringBuffer.appendInt(SW_VERSION_MINOR);
    stringBuffer.appendText(".");
    stringBuffer.appendInt(SW_VERSION_REVISION);
    stringBuffer.endLine();

    updateText(startRow+1, lcdText_temp, getTextCenter(stringBuffer.getSize()));

    stringBuffer.startLine();
    stringBuffer.appendText_P(hwVersion_string);

    #ifdef BOARD_OPEN_DECK
    stringBuffer.appendText("v:");
    stringBuffer.appendInt(HARDWARE_VERSION_MAJOR);
    stringBuffer.appendText(".");
    stringBuffer.appendInt(HARDWARE_VERSION_MINOR);
    stringBuffer.appendText(".x");
    stringBuffer.endLine();
    #else
    stringBuffer.appendText_P(reinterpret_cast<char*>(pgm_read_word(&(boardNameArray[BOARD_ID]))));
    stringBuffer.endLine();
    #endif

    updateText(startRow+2, lcdText_temp, getTextCenter(stringBuffer.getSize()));

    wait_ms(2000);
}

void Display::displayHome()
{
    U8X8::clearDisplay();

    uint8_t startRow;

    switch(U8X8::getRows())
    {
        case 4:
        startRow = 0;
        break;

        default:
        startRow = 0;
        break;
    }

    stringBuffer.startLine();
    stringBuffer.appendText_P(eventMIDIin_string);
    stringBuffer.endLine();

    updateText(startRow, lcdtext_still, 0);

    stringBuffer.startLine();
    stringBuffer.appendText_P(eventMIDIout_string);
    stringBuffer.endLine();

    updateText(startRow+2, lcdtext_still, 0);
}

void Display::displayMIDIevent(displayEventType_t type, midiMessageTypeDisplay_t message, uint16_t byte1, uint16_t byte2, uint8_t byte3)
{
    uint8_t startRow = (type == displayEventIn) ? ROW_START_MIDI_IN_MESSAGE : ROW_START_MIDI_OUT_MESSAGE;
    uint8_t startColumn = (type == displayEventIn) ? COLUMN_START_MIDI_IN_MESSAGE : COLUMN_START_MIDI_OUT_MESSAGE;

    stringBuffer.startLine();
    stringBuffer.appendText_P(reinterpret_cast<char*>(pgm_read_word(&(eventNameArray[static_cast<uint8_t>(message)]))));
    stringBuffer.appendChar(' ', U8X8::getColumns() - startColumn - stringBuffer.getSize());
    stringBuffer.endLine();
    updateText(startRow, lcdtext_still, startColumn);

    switch(message)
    {
        case midiMessageNoteOff_display:
        case midiMessageNoteOn_display:
        stringBuffer.startLine();

        if (!alternateNoteDisplay)
        {
            stringBuffer.appendInt(byte1);
        }
        else
        {
            stringBuffer.appendText_P(reinterpret_cast<char*>(pgm_read_word(&(noteNameArray[static_cast<uint8_t>(MIDI::getTonicFromNote(byte1))]))));
            stringBuffer.appendInt(normalizeOctave(MIDI::getOctaveFromNote(byte1), octaveNormalization));
        }

        stringBuffer.appendText(" v");
        stringBuffer.appendInt(byte2);
        stringBuffer.appendText(" CH");
        stringBuffer.appendInt(byte3);
        stringBuffer.appendChar(' ', U8X8::getColumns() - stringBuffer.getSize());
        stringBuffer.endLine();
        updateText(startRow+1, lcdtext_still, 0);
        break;

        case midiMessageProgramChange_display:
        stringBuffer.startLine();
        stringBuffer.appendInt(byte1);
        stringBuffer.appendText(" CH");
        stringBuffer.appendInt(byte3);
        stringBuffer.appendChar(' ', U8X8::getColumns() - stringBuffer.getSize());
        stringBuffer.endLine();
        updateText(startRow+1, lcdtext_still, 0);
        break;

        case midiMessageControlChange_display:
        case midiMessageNRPN_display:
        stringBuffer.startLine();
        stringBuffer.appendInt(byte1);
        stringBuffer.appendText(" ");
        stringBuffer.appendInt(byte2);
        stringBuffer.appendText(" CH");
        stringBuffer.appendInt(byte3);
        stringBuffer.appendChar(' ', U8X8::getColumns() - stringBuffer.getSize());
        stringBuffer.endLine();
        updateText(startRow+1, lcdtext_still, 0);
        break;

        case midiMessageMMCplay_display:
        case midiMessageMMCstop_display:
        case midiMessageMMCrecordOn_display:
        case midiMessageMMCrecordOff_display:
        case midiMessageMMCpause_display:
        stringBuffer.startLine();
        stringBuffer.appendText("CH");
        stringBuffer.appendInt(byte1);
        stringBuffer.appendChar(' ', U8X8::getColumns() - stringBuffer.getSize());
        stringBuffer.endLine();
        updateText(startRow+1, lcdtext_still, 0);
        break;

        case midiMessageClock_display:
        case midiMessageStart_display:
        case midiMessageContinue_display:
        case midiMessageStop_display:
        case midiMessageActiveSensing_display:
        case midiMessageSystemReset_display:
        case midiMessageSystemExclusive_display:
        stringBuffer.startLine();
        stringBuffer.appendChar(' ', U8X8::getColumns());
        stringBuffer.endLine();
        updateText(startRow+1, lcdtext_still, 0);
        break;

        default:
        break;
    }

    lastMIDIMessageDisplayTime[type] = rTimeMs();
    midiMessageDisplayed[type] = true;
}

void Display::clearMIDIevent(displayEventType_t type)
{
    switch(type)
    {
        case displayEventIn:
        //first row
        stringBuffer.startLine();
        stringBuffer.appendChar(' ', U8X8::getColumns()-COLUMN_START_MIDI_IN_MESSAGE);
        stringBuffer.endLine();
        updateText(ROW_START_MIDI_IN_MESSAGE, lcdtext_still, COLUMN_START_MIDI_IN_MESSAGE);
        //second row
        stringBuffer.startLine();
        stringBuffer.appendChar(' ', U8X8::getColumns());
        stringBuffer.endLine();
        updateText(ROW_START_MIDI_IN_MESSAGE+1, lcdtext_still, 0);
        break;

        case displayEventOut:
        //first row
        stringBuffer.startLine();
        stringBuffer.appendChar(' ', U8X8::getColumns()-COLUMN_START_MIDI_OUT_MESSAGE);
        stringBuffer.endLine();
        updateText(ROW_START_MIDI_OUT_MESSAGE, lcdtext_still, COLUMN_START_MIDI_OUT_MESSAGE);
        //second row
        stringBuffer.startLine();
        stringBuffer.appendChar(' ', U8X8::getColumns());
        stringBuffer.endLine();
        updateText(ROW_START_MIDI_OUT_MESSAGE+1, lcdtext_still, 0);
        break;

        default:
        return;
        break;
    }

    midiMessageDisplayed[type] = false;
}

void Display::setAlternateNoteDisplay(bool state)
{
    alternateNoteDisplay = state;
}

void Display::setOctaveNormalization(int8_t value)
{
    octaveNormalization = value;
}