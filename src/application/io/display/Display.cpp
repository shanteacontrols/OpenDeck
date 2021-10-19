/*

Copyright 2015-2021 Igor Petrovic

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
#include "midi/src/MIDI.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Helpers.h"

using namespace IO;

Display::Display(IO::U8X8&                u8x8,
                 Database&                database,
                 Util::MessageDispatcher& dispatcher)
    : _u8x8(u8x8)
    , _database(database)
{
    dispatcher.listen(Util::MessageDispatcher::messageSource_t::analog,
                      Util::MessageDispatcher::listenType_t::nonFwd,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          displayMIDIevent(Display::eventType_t::out, dispatchMessage);
                      });

    dispatcher.listen(Util::MessageDispatcher::messageSource_t::buttons,
                      Util::MessageDispatcher::listenType_t::nonFwd,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          displayMIDIevent(Display::eventType_t::out, dispatchMessage);
                      });

    dispatcher.listen(Util::MessageDispatcher::messageSource_t::encoders,
                      Util::MessageDispatcher::listenType_t::nonFwd,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          displayMIDIevent(Display::eventType_t::out, dispatchMessage);
                      });

    dispatcher.listen(Util::MessageDispatcher::messageSource_t::touchscreenButton,
                      Util::MessageDispatcher::listenType_t::nonFwd,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          displayMIDIevent(Display::eventType_t::out, dispatchMessage);
                      });

    dispatcher.listen(Util::MessageDispatcher::messageSource_t::touchscreenAnalog,
                      Util::MessageDispatcher::listenType_t::nonFwd,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          displayMIDIevent(Display::eventType_t::out, dispatchMessage);
                      });

    dispatcher.listen(Util::MessageDispatcher::messageSource_t::midiIn,
                      Util::MessageDispatcher::listenType_t::nonFwd,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          displayMIDIevent(Display::eventType_t::in, dispatchMessage);
                      });
}

/// Initialize display driver and variables.
bool Display::init(bool startupInfo)
{
    if (_database.read(Database::Section::display_t::features, feature_t::enable))
    {
        auto    controller = static_cast<U8X8::displayController_t>(_database.read(Database::Section::display_t::setting, setting_t::controller));
        auto    resolution = static_cast<U8X8::displayResolution_t>(_database.read(Database::Section::display_t::setting, setting_t::resolution));
        uint8_t address    = _database.read(Database::Section::display_t::setting, setting_t::i2cAddress);

        if (!startupInfo)
        {
            // avoid reinitializing display with the same settings in this case
            if (_initialized)
            {
                if (_lastController == controller)
                {
                    if (_lastResolution == resolution)
                    {
                        if (_lastAddress == address)
                        {
                            return true;
                        }
                    }
                }
            }
        }

        if (_u8x8.init(address, controller, resolution))
        {
            _u8x8.clearDisplay();
            _u8x8.setPowerSave(0);
            _u8x8.setFlipMode(0);
            _u8x8.setFont(u8x8_font_pxplustandynewtv_r);
            _u8x8.clearDisplay();

            _resolution = resolution;

            // init char arrays
            for (int i = 0; i < LCD_HEIGHT_MAX; i++)
            {
                for (int j = 0; j < LCD_STRING_BUFFER_SIZE - 2; j++)
                {
                    _lcdRowStillText[i][j] = ' ';
                    _lcdRowTempText[i][j]  = ' ';
                }

                _lcdRowStillText[i][LCD_STRING_BUFFER_SIZE - 1] = '\0';
                _lcdRowTempText[i][LCD_STRING_BUFFER_SIZE - 1]  = '\0';

                _scrollEvent[i].size         = 0;
                _scrollEvent[i].startIndex   = 0;
                _scrollEvent[i].currentIndex = 0;
                _scrollEvent[i].direction    = scrollDirection_t::leftToRight;
            }

            _initialized = true;

            if (startupInfo)
            {
                setDirectWriteState(true);

                if (_database.read(Database::Section::display_t::features, feature_t::welcomeMsg))
                    displayWelcomeMessage();

                if (_database.read(Database::Section::display_t::features, feature_t::vInfoMsg))
                    displayVinfo(false);

                setDirectWriteState(false);
            }

            setAlternateNoteDisplay(_database.read(Database::Section::display_t::features, feature_t::MIDInotesAlternate));
            setRetentionTime(_database.read(Database::Section::display_t::setting, setting_t::MIDIeventTime) * 1000);

            clearMIDIevent(eventType_t::in);
            clearMIDIevent(eventType_t::out);

            _lastController = controller;
            _lastResolution = resolution;

            return true;
        }
    }

    return false;
}

bool Display::deInit()
{
    if (!_initialized)
        return false;    // nothing to do

    if (_u8x8.deInit())
    {
        _initialized    = false;
        _lastController = U8X8::displayController_t::invalid;
        _lastResolution = U8X8::displayResolution_t::invalid;
        _lastAddress    = 0;
        return true;
    }

    return false;
}

/// Checks if LCD requires updating continuously.
bool Display::update()
{
    if (!_initialized)
        return false;

    if ((core::timing::currentRunTimeMs() - _lastLCDupdateTime) < LCD_REFRESH_TIME)
        return false;    // we don't need to update lcd in real time

    // use char pointer to point to line we're going to print
    char* charPointer;

    updateTempTextStatus();

    for (int i = 0; i < LCD_HEIGHT_MAX; i++)
    {
        if (_activeTextType == lcdTextType_t::still)
        {
            // scrolling is possible only with still text
            updateScrollStatus(i);
            charPointer = _lcdRowStillText[i];
        }
        else
        {
            charPointer = _lcdRowTempText[i];
        }

        if (!_charChange[i])
            continue;

        int8_t string_len = strlen(charPointer) > LCD_WIDTH_MAX ? LCD_WIDTH_MAX : strlen(charPointer);

        for (int j = 0; j < string_len; j++)
        {
            if (BIT_READ(_charChange[i], j))
                _u8x8.drawGlyph(j, _rowMap[_resolution][i], charPointer[j + _scrollEvent[i].currentIndex]);
        }

        // now fill remaining columns with spaces
        for (uint16_t j = string_len; j < LCD_WIDTH_MAX; j++)
            _u8x8.drawGlyph(j, _rowMap[_resolution][i], ' ');

        _charChange[i] = 0;
    }

    _lastLCDupdateTime = core::timing::currentRunTimeMs();

    // check if midi in/out messages need to be cleared
    if (_MIDImessageRetentionTime)
    {
        for (int i = 0; i < 2; i++)
        {
            // 0 = in, 1 = out
            if ((core::timing::currentRunTimeMs() - _lastMIDIMessageDisplayTime[i] > _MIDImessageRetentionTime) && _midiMessageDisplayed[i])
                clearMIDIevent(static_cast<eventType_t>(i));
        }
    }

    return true;
}

/// Updates text to be shown on display.
/// This function only updates internal buffers with received text, actual updating is done in update() function.
/// Text isn't passed directly, instead, value from string builder is used.
/// param [in]: row             Row which is being updated.
/// param [in]: textType        Type of text to be shown on display (enumerated type). See lcdTextType_t enumeration.
/// param [in]: startIndex      Index on which received text should on specified row.
void Display::updateText(uint8_t row, lcdTextType_t textType, uint8_t startIndex)
{
    if (!_initialized)
        return;

    const char* string     = _stringBuilder.string();
    uint8_t     size       = strlen(string);
    uint8_t     scrollSize = 0;

    if (size + startIndex >= LCD_STRING_BUFFER_SIZE - 2)
        size = LCD_STRING_BUFFER_SIZE - 2 - startIndex;    // trim string

    if (_directWriteState)
    {
        for (int j = 0; j < size; j++)
            _u8x8.drawGlyph(j + startIndex, _rowMap[_resolution][row], string[j]);
    }
    else
    {
        bool scrollingEnabled = false;

        switch (textType)
        {
        case lcdTextType_t::still:
        {
            for (int i = 0; i < size; i++)
            {
                _lcdRowStillText[row][startIndex + i] = string[i];
                BIT_WRITE(_charChange[row], startIndex + i, 1);
            }

            // scrolling is enabled only if some characters are found after LCD_WIDTH_MAX-1 index
            for (int i = LCD_WIDTH_MAX; i < LCD_STRING_BUFFER_SIZE - 1; i++)
            {
                if ((_lcdRowStillText[row][i] != ' ') && (_lcdRowStillText[row][i] != '\0'))
                {
                    scrollingEnabled = true;
                    scrollSize++;
                }
            }

            if (scrollingEnabled && !_scrollEvent[row].size)
            {
                // enable scrolling
                _scrollEvent[row].size         = scrollSize;
                _scrollEvent[row].startIndex   = startIndex;
                _scrollEvent[row].currentIndex = 0;
                _scrollEvent[row].direction    = scrollDirection_t::leftToRight;
                _lastScrollTime                = core::timing::currentRunTimeMs();
            }
            else if (!scrollingEnabled && _scrollEvent[row].size)
            {
                _scrollEvent[row].size         = 0;
                _scrollEvent[row].startIndex   = 0;
                _scrollEvent[row].currentIndex = 0;
                _scrollEvent[row].direction    = scrollDirection_t::leftToRight;
            }
        }
        break;

        case lcdTextType_t::temp:
        {
            // clear entire message first
            for (uint16_t j = 0; j < LCD_WIDTH_MAX - 2; j++)
                _lcdRowTempText[row][j] = ' ';

            _lcdRowTempText[row][LCD_WIDTH_MAX - 1] = '\0';

            for (int i = 0; i < size; i++)
                _lcdRowTempText[row][startIndex + i] = string[i];

            // make sure message is properly EOL'ed
            _lcdRowTempText[row][startIndex + size] = '\0';

            _activeTextType     = lcdTextType_t::temp;
            _messageDisplayTime = core::timing::currentRunTimeMs();

            // update all characters on display
            for (int i = 0; i < LCD_HEIGHT_MAX; i++)
                _charChange[i] = static_cast<uint32_t>(0xFFFFFFFF);
        }
        break;

        default:
            return;
        }
    }
}

/// Enables or disables direct writing to LCD.
/// When enabled, low-level APIs are used to write text to LCD directly.
/// Otherwise, update() function takes care of updating LCD.
/// param [in]: state   New direct write state.
void Display::setDirectWriteState(bool state)
{
    _directWriteState = state;
}

/// Calculates position on which text needs to be set on display to be in center of display row.
/// param [in]: textSize    Size of text for which center position on display is being calculated.
/// returns: Center position of text on display.
uint8_t Display::getTextCenter(uint8_t textSize)
{
    return _u8x8.getColumns() / 2 - (textSize / 2);
}

/// Updates status of temp text on display.
void Display::updateTempTextStatus()
{
    if (_activeTextType == lcdTextType_t::temp)
    {
        // temp text - check if temp text should be removed
        if ((core::timing::currentRunTimeMs() - _messageDisplayTime) > LCD_MESSAGE_DURATION)
        {
            _activeTextType = lcdTextType_t::still;

            // make sure all characters are updated once temp text is removed
            for (int j = 0; j < LCD_HEIGHT_MAX; j++)
                _charChange[j] = static_cast<uint32_t>(0xFFFFFFFF);
        }
    }
}

/// Updates status of scrolling text on display.
/// param [in]: row     Row which is being checked.
void Display::updateScrollStatus(uint8_t row)
{
    if (!_scrollEvent[row].size)
        return;

    if ((core::timing::currentRunTimeMs() - _lastScrollTime) < LCD_SCROLL_TIME)
        return;

    switch (_scrollEvent[row].direction)
    {
    case scrollDirection_t::leftToRight:
    {
        // left to right
        _scrollEvent[row].currentIndex++;

        if (_scrollEvent[row].currentIndex == _scrollEvent[row].size)
        {
            // switch direction
            _scrollEvent[row].direction = scrollDirection_t::rightToLeft;
        }
    }
    break;

    case scrollDirection_t::rightToLeft:
    {
        // right to left
        _scrollEvent[row].currentIndex--;

        if (_scrollEvent[row].currentIndex == 0)
        {
            // switch direction
            _scrollEvent[row].direction = scrollDirection_t::leftToRight;
        }
    }
    break;

    default:
        break;
    }

    for (uint16_t i = _scrollEvent[row].startIndex; i < LCD_WIDTH_MAX; i++)
        BIT_WRITE(_charChange[row], i, 1);

    _lastScrollTime = core::timing::currentRunTimeMs();
}

/// Checks for currently active text type on display.
/// returns: Active text type (enumerated type). See lcdTextType_t enumeration.
Display::lcdTextType_t Display::getActiveTextType()
{
    return _activeTextType;
}

/// Sets new message retention time.
/// param [in]: retentionTime New retention time in milliseconds.
void Display::setRetentionTime(uint32_t retentionTime)
{
    if (retentionTime < _MIDImessageRetentionTime)
    {
        for (int i = 0; i < 2; i++)
        {
            // 0 = in, 1 = out
            // make sure events are cleared immediately in next call of update()
            _lastMIDIMessageDisplayTime[i] = 0;
        }
    }

    _MIDImessageRetentionTime = retentionTime;

    // reset last update time
    _lastMIDIMessageDisplayTime[eventType_t::in]  = core::timing::currentRunTimeMs();
    _lastMIDIMessageDisplayTime[eventType_t::out] = core::timing::currentRunTimeMs();
}

/// Adds normalization to a given octave.
int8_t Display::normalizeOctave(uint8_t octave, int8_t normalization)
{
    return static_cast<int8_t>(octave) + normalization;
}

void Display::displayWelcomeMessage()
{
    if (!_initialized)
        return;

    uint8_t charIndex = 0;
    uint8_t location  = 0;
    uint8_t startRow;

    _u8x8.clearDisplay();

    switch (_u8x8.getRows())
    {
    case 4:
    {
        startRow = 1;
    }
    break;

    default:
    {
        startRow = 0;
    }
    break;
    }

    _stringBuilder.overwrite("OpenDeck");
    location           = getTextCenter(strlen(_stringBuilder.string()));
    charIndex          = 0;
    const char* string = _stringBuilder.string();

    while (string[charIndex] != '\0')
    {
        _u8x8.drawGlyph(location + charIndex, _rowMap[_resolution][startRow], string[charIndex]);
        charIndex++;
    }

    core::timing::waitMs(1000);

    _stringBuilder.overwrite("Welcome!");
    location  = getTextCenter(strlen(_stringBuilder.string()));
    charIndex = 0;

    while (string[charIndex] != '\0')
    {
        _u8x8.drawGlyph(location + charIndex, _rowMap[_resolution][startRow + 1], string[charIndex]);
        core::timing::waitMs(50);
        charIndex++;
    }

    core::timing::waitMs(2000);
}

void Display::displayVinfo(bool newFw)
{
    if (!_initialized)
        return;

    uint8_t startRow;

    _u8x8.clearDisplay();

    switch (_u8x8.getRows())
    {
    case 4:
    {
        startRow = 1;
    }
    break;

    default:
    {
        startRow = 0;
    }
    break;
    }

    _stringBuilder.overwrite("Version info:");
    updateText(startRow, lcdTextType_t::temp, getTextCenter(strlen(_stringBuilder.string())));

    _stringBuilder.overwrite("FW: v%d.%d.%d", SW_VERSION_MAJOR, SW_VERSION_MINOR, SW_VERSION_REVISION);
    updateText(startRow + 1, lcdTextType_t::temp, getTextCenter(strlen(_stringBuilder.string())));

    _stringBuilder.overwrite("HW: %s", Strings::board());

    updateText(startRow + 2, lcdTextType_t::temp, getTextCenter(strlen(_stringBuilder.string())));

    core::timing::waitMs(2000);
}

void Display::displayMIDIevent(eventType_t type, const Util::MessageDispatcher::message_t& dispatchMessage)
{
    if (!_initialized)
        return;

    uint8_t startRow    = (type == Display::eventType_t::in) ? ROW_START_MIDI_IN_MESSAGE : ROW_START_MIDI_OUT_MESSAGE;
    uint8_t startColumn = (type == Display::eventType_t::in) ? COLUMN_START_MIDI_IN_MESSAGE : COLUMN_START_MIDI_OUT_MESSAGE;

    _stringBuilder.overwrite("%s", Strings::midiMessage(dispatchMessage.message));
    _stringBuilder.fillUntil(_u8x8.getColumns() - startColumn - strlen(_stringBuilder.string()));
    updateText(startRow, lcdTextType_t::still, startColumn);

    switch (dispatchMessage.message)
    {
    case MIDI::messageType_t::noteOff:
    case MIDI::messageType_t::noteOn:
    {
        if (!_alternateNoteDisplay)
            _stringBuilder.overwrite("%d", dispatchMessage.midiIndex);
        else
            _stringBuilder.overwrite("%s%d", Strings::note(MIDI::getTonicFromNote(dispatchMessage.midiIndex)), normalizeOctave(MIDI::getOctaveFromNote(dispatchMessage.midiValue), _octaveNormalization));

        _stringBuilder.append(" v%d CH%d", dispatchMessage.midiValue, dispatchMessage.midiChannel + 1);
        _stringBuilder.fillUntil(_u8x8.getColumns() - strlen(_stringBuilder.string()));
        updateText(startRow + 1, lcdTextType_t::still, 0);
    }
    break;

    case MIDI::messageType_t::programChange:
    {
        _stringBuilder.overwrite("%d CH%d", dispatchMessage.midiIndex, dispatchMessage.midiChannel + 1);
        _stringBuilder.fillUntil(_u8x8.getColumns() - strlen(_stringBuilder.string()));
        updateText(startRow + 1, lcdTextType_t::still, 0);
    }
    break;

    case MIDI::messageType_t::controlChange:
    case MIDI::messageType_t::controlChange14bit:
    case MIDI::messageType_t::nrpn7bit:
    case MIDI::messageType_t::nrpn14bit:
    {
        _stringBuilder.overwrite("%d %d CH%d", dispatchMessage.midiIndex, dispatchMessage.midiValue, dispatchMessage.midiChannel + 1);
        _stringBuilder.fillUntil(_u8x8.getColumns() - strlen(_stringBuilder.string()));
        updateText(startRow + 1, lcdTextType_t::still, 0);
    }
    break;

    case MIDI::messageType_t::mmcPlay:
    case MIDI::messageType_t::mmcStop:
    case MIDI::messageType_t::mmcRecordStart:
    case MIDI::messageType_t::mmcRecordStop:
    case MIDI::messageType_t::mmcPause:
    {
        _stringBuilder.overwrite("CH%d", dispatchMessage.midiIndex);
        _stringBuilder.fillUntil(_u8x8.getColumns() - strlen(_stringBuilder.string()));
        updateText(startRow + 1, lcdTextType_t::still, 0);
    }
    break;

    case MIDI::messageType_t::sysRealTimeClock:
    case MIDI::messageType_t::sysRealTimeStart:
    case MIDI::messageType_t::sysRealTimeContinue:
    case MIDI::messageType_t::sysRealTimeStop:
    case MIDI::messageType_t::sysRealTimeActiveSensing:
    case MIDI::messageType_t::sysRealTimeSystemReset:
    case MIDI::messageType_t::systemExclusive:
    {
        _stringBuilder.overwrite("");
        _stringBuilder.fillUntil(_u8x8.getColumns());
        updateText(startRow + 1, lcdTextType_t::still, 0);
    }
    break;

    default:
        break;
    }

    _lastMIDIMessageDisplayTime[type] = core::timing::currentRunTimeMs();
    _midiMessageDisplayed[type]       = true;
}

void Display::clearMIDIevent(eventType_t type)
{
    if (!_initialized)
        return;

    switch (type)
    {
    case eventType_t::in:
    {
        // first row
        _stringBuilder.overwrite("In: ");
        _stringBuilder.fillUntil(_u8x8.getColumns() - strlen(_stringBuilder.string()));
        updateText(ROW_START_MIDI_IN_MESSAGE, lcdTextType_t::still, 0);
        // second row
        _stringBuilder.overwrite("");
        _stringBuilder.fillUntil(_u8x8.getColumns() - strlen(_stringBuilder.string()));
        updateText(ROW_START_MIDI_IN_MESSAGE + 1, lcdTextType_t::still, 0);
    }
    break;

    case eventType_t::out:
    {
        // first row
        _stringBuilder.overwrite("Out: ");
        _stringBuilder.fillUntil(_u8x8.getColumns() - strlen(_stringBuilder.string()));
        updateText(ROW_START_MIDI_OUT_MESSAGE, lcdTextType_t::still, 0);
        // second row
        _stringBuilder.overwrite("");
        _stringBuilder.fillUntil(_u8x8.getColumns() - strlen(_stringBuilder.string()));
        updateText(ROW_START_MIDI_OUT_MESSAGE + 1, lcdTextType_t::still, 0);
    }
    break;

    default:
        return;
    }

    _midiMessageDisplayed[type] = false;
}

void Display::setAlternateNoteDisplay(bool state)
{
    _alternateNoteDisplay = state;
}

void Display::setOctaveNormalization(int8_t value)
{
    _octaveNormalization = value;
}

void Display::setPreset(uint8_t preset)
{
    if (!_initialized)
        return;

    if (preset != _activePreset)
    {
        uint8_t startRow = ROW_START_MIDI_IN_MESSAGE;

        _stringBuilder.overwrite("%d", preset);
        _stringBuilder.fillUntil(_u8x8.getColumns() - strlen(_stringBuilder.string()));
        updateText(startRow + 1, lcdTextType_t::still, 0);

        _activePreset = preset;
    }
}