/*

Copyright 2015-2020 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include <string.h>
#include "Display.h"
#include "strings/Strings.h"
#include "../../Version.h"
#include "midi/src/MIDI.h"
#include "core/src/general/Timing.h"

using namespace IO;

void Display::displayWelcomeMessage()
{
    if (!initDone)
        return;

    uint8_t charIndex = 0;
    uint8_t location  = 0;
    uint8_t startRow;

    u8x8.clearDisplay();

    switch (u8x8.getRows())
    {
    case 4:
        startRow = 1;
        break;

    default:
        startRow = 0;
        break;
    }

    stringBuilder.overwrite("OpenDeck");
    location           = getTextCenter(strlen(stringBuilder.string()));
    charIndex          = 0;
    const char* string = stringBuilder.string();

    while (string[charIndex] != '\0')
    {
        u8x8.drawGlyph(location + charIndex, rowMap[resolution][startRow], string[charIndex]);
        charIndex++;
    }

    core::timing::waitMs(1000);

    stringBuilder.overwrite("Welcome!");
    location  = getTextCenter(strlen(stringBuilder.string()));
    charIndex = 0;

    while (string[charIndex] != '\0')
    {
        u8x8.drawGlyph(location + charIndex, rowMap[resolution][startRow + 1], string[charIndex]);
        core::timing::waitMs(50);
        charIndex++;
    }

    core::timing::waitMs(2000);
}

void Display::displayVinfo(bool newFw)
{
    if (!initDone)
        return;

    uint8_t startRow;

    u8x8.clearDisplay();

    switch (u8x8.getRows())
    {
    case 4:
        startRow = 1;
        break;

    default:
        startRow = 0;
        break;
    }

    stringBuilder.overwrite("Version info:");
    updateText(startRow, lcdTextType_t::temp, getTextCenter(strlen(stringBuilder.string())));

    stringBuilder.overwrite("FW: v%d.%d.%d", SW_VERSION_MAJOR, SW_VERSION_MINOR, SW_VERSION_REVISION);
    updateText(startRow + 1, lcdTextType_t::temp, getTextCenter(strlen(stringBuilder.string())));

    stringBuilder.overwrite("HW: %s", Strings::board());

    updateText(startRow + 2, lcdTextType_t::temp, getTextCenter(strlen(stringBuilder.string())));

    core::timing::waitMs(2000);
}

void Display::displayMIDIevent(eventType_t type, event_t event, uint16_t byte1, uint16_t byte2, uint8_t byte3)
{
    if (!initDone)
        return;

    uint8_t startRow    = (type == Display::eventType_t::in) ? ROW_START_MIDI_IN_MESSAGE : ROW_START_MIDI_OUT_MESSAGE;
    uint8_t startColumn = (type == Display::eventType_t::in) ? COLUMN_START_MIDI_IN_MESSAGE : COLUMN_START_MIDI_OUT_MESSAGE;

    stringBuilder.overwrite("%s", Strings::midiMessage(event));
    stringBuilder.fillUntil(u8x8.getColumns() - startColumn - strlen(stringBuilder.string()));
    updateText(startRow, lcdTextType_t::still, startColumn);

    switch (event)
    {
    case event_t::noteOff:
    case event_t::noteOn:

        if (!alternateNoteDisplay)
            stringBuilder.overwrite("%d", byte1);
        else
            stringBuilder.overwrite("%s%d", Strings::note(MIDI::getTonicFromNote(byte1)), normalizeOctave(MIDI::getOctaveFromNote(byte1), octaveNormalization));

        stringBuilder.append(" v%d CH%d", byte2, byte3);
        stringBuilder.fillUntil(u8x8.getColumns() - strlen(stringBuilder.string()));
        updateText(startRow + 1, lcdTextType_t::still, 0);
        break;

    case event_t::programChange:
        stringBuilder.overwrite("%d CH%d", byte1, byte3);
        stringBuilder.fillUntil(u8x8.getColumns() - strlen(stringBuilder.string()));
        updateText(startRow + 1, lcdTextType_t::still, 0);
        break;

    case event_t::controlChange:
    case event_t::nrpn:
        stringBuilder.overwrite("%d %d CH%d", byte1, byte2, byte3);
        stringBuilder.fillUntil(u8x8.getColumns() - strlen(stringBuilder.string()));
        updateText(startRow + 1, lcdTextType_t::still, 0);
        break;

    case event_t::mmcPlay:
    case event_t::mmcStop:
    case event_t::mmcRecordOn:
    case event_t::mmcRecordOff:
    case event_t::mmcPause:
        stringBuilder.overwrite("CH%d", byte1);
        stringBuilder.fillUntil(u8x8.getColumns() - strlen(stringBuilder.string()));
        updateText(startRow + 1, lcdTextType_t::still, 0);
        break;

    case event_t::sysRealTimeClock:
    case event_t::sysRealTimeStart:
    case event_t::sysRealTimeContinue:
    case event_t::sysRealTimeStop:
    case event_t::sysRealTimeActiveSensing:
    case event_t::sysRealTimeSystemReset:
    case event_t::systemExclusive:
        stringBuilder.overwrite("");
        stringBuilder.fillUntil(u8x8.getColumns());
        updateText(startRow + 1, lcdTextType_t::still, 0);
        break;

    case event_t::presetChange:
        stringBuilder.overwrite("%d", byte1);
        stringBuilder.fillUntil(u8x8.getColumns() - strlen(stringBuilder.string()));
        updateText(startRow + 1, lcdTextType_t::still, 0);
        break;

    default:
        break;
    }

    lastMIDIMessageDisplayTime[type] = core::timing::currentRunTimeMs();
    midiMessageDisplayed[type]       = true;
}

void Display::clearMIDIevent(eventType_t type)
{
    if (!initDone)
        return;

    switch (type)
    {
    case eventType_t::in:
        //first row
        stringBuilder.overwrite("In: ");
        stringBuilder.fillUntil(u8x8.getColumns() - strlen(stringBuilder.string()));
        updateText(ROW_START_MIDI_IN_MESSAGE, lcdTextType_t::still, 0);
        //second row
        stringBuilder.overwrite("");
        stringBuilder.fillUntil(u8x8.getColumns() - strlen(stringBuilder.string()));
        updateText(ROW_START_MIDI_IN_MESSAGE + 1, lcdTextType_t::still, 0);
        break;

    case eventType_t::out:
        //first row
        stringBuilder.overwrite("Out: ");
        stringBuilder.fillUntil(u8x8.getColumns() - strlen(stringBuilder.string()));
        updateText(ROW_START_MIDI_OUT_MESSAGE, lcdTextType_t::still, 0);
        //second row
        stringBuilder.overwrite("");
        stringBuilder.fillUntil(u8x8.getColumns() - strlen(stringBuilder.string()));
        updateText(ROW_START_MIDI_OUT_MESSAGE + 1, lcdTextType_t::still, 0);
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