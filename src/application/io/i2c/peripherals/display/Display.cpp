/*

Copyright 2015-2022 Igor Petrovic

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
#include "protocol/midi/MIDI.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Helpers.h"
#include "io/common/Common.h"
#include "util/conversion/Conversion.h"
#include "util/configurable/Configurable.h"

using namespace IO;
using namespace Protocol;

// u8x8 lib doesn't send packets larger than 32 bytes
#define U8X8_BUFFER_SIZE 32

namespace
{
    uint8_t u8x8Buffer[U8X8_BUFFER_SIZE];
    size_t  u8x8Counter;
}    // namespace

// hwa needs to be static since it is used in callback for C library
I2C::HWA* Display::_hwa;

Display::Display(I2C::HWA&           hwa,
                 Database::Instance& database)
    : _database(database)
{
    _hwa = &hwa;

    MIDIDispatcher.listen(Messaging::eventType_t::analog,
                          [this](const Messaging::event_t& event) {
                              displayMIDIevent(Display::eventType_t::out, event);
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::button,
                          [this](const Messaging::event_t& event) {
                              displayMIDIevent(Display::eventType_t::out, event);
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::encoder,
                          [this](const Messaging::event_t& event) {
                              displayMIDIevent(Display::eventType_t::out, event);
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::midiIn,
                          [this](const Messaging::event_t& event) {
                              if (event.message != MIDI::messageType_t::systemExclusive)
                              {
                                  displayMIDIevent(Display::eventType_t::in, event);
                              }
                          });

    ConfigHandler.registerConfig(
        System::Config::block_t::i2c,
        // read
        [this](uint8_t section, size_t index, uint16_t& value) {
            return sysConfigGet(static_cast<System::Config::Section::i2c_t>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value) {
            return sysConfigSet(static_cast<System::Config::Section::i2c_t>(section), index, value);
        });

    I2C::registerPeripheral(this);
}

bool Display::init(uint8_t address)
{
    _selectedI2Caddress = address;

    if (_database.read(Database::Config::Section::i2c_t::display, setting_t::enable))
    {
        auto controller = static_cast<displayController_t>(_database.read(Database::Config::Section::i2c_t::display, setting_t::controller));
        auto resolution = static_cast<displayResolution_t>(_database.read(Database::Config::Section::i2c_t::display, setting_t::resolution));

        if (initU8X8(address, controller, resolution))
        {
            _resolution = resolution;

            // init char arrays
            for (int i = 0; i < LCD_HEIGHT_MAX; i++)
            {
                for (int j = 0; j < LCD_STRING_BUFFER_SIZE - 2; j++)
                {
                    _lcdRowText[i][j] = ' ';
                }

                _lcdRowText[i][LCD_STRING_BUFFER_SIZE - 1] = '\0';
            }

            _initialized = true;

            if (!_startupInfoShown)
            {
                if (_database.read(Database::Config::Section::i2c_t::display, setting_t::deviceInfoMsg) && !_startupInfoShown)
                {
                    displayWelcomeMessage();
                }
            }

            setRetentionTime(_database.read(Database::Config::Section::i2c_t::display, setting_t::MIDIeventTime) * 1000);

            clearMIDIevent(eventType_t::in);
            clearMIDIevent(eventType_t::out);
        }
        else
        {
            _initialized = false;
            return false;
        }
    }

    // make sure welcome message on startup isn't shown anymore when init is called
    _startupInfoShown = true;

    return _initialized;
}

bool Display::initU8X8(uint8_t i2cAddress, displayController_t controller, displayResolution_t resolution)
{
    bool success = false;

    // setup defaults
    u8x8_SetupDefaults(&_u8x8);

    // i2c hw access
    auto gpioDelay = [](u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, U8X8_UNUSED void* arg_ptr) -> uint8_t {
        return 0;
    };

    auto i2cHWA = [](u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr) -> uint8_t {
        auto* array = (uint8_t*)arg_ptr;

        switch (msg)
        {
        case U8X8_MSG_BYTE_SEND:
        {
            memcpy(&u8x8Buffer[u8x8Counter], array, arg_int);
            u8x8Counter += arg_int;
        }
        break;

        case U8X8_MSG_BYTE_INIT:
            break;

        case U8X8_MSG_BYTE_START_TRANSFER:
        {
            u8x8Counter = 0;
        }
        break;

        case U8X8_MSG_BYTE_END_TRANSFER:
            return _hwa->write(u8x8_GetI2CAddress(u8x8), u8x8Buffer, u8x8Counter);

        default:
            return 0;
        }

        return 1;
    };

    // setup specific callbacks depending on controller/resolution
    if ((resolution == displayResolution_t::_128x64) && (controller == displayController_t::ssd1306))
    {
        _u8x8.display_cb        = u8x8_d_ssd1306_128x64_noname;
        _u8x8.cad_cb            = u8x8_cad_ssd13xx_i2c;
        _u8x8.byte_cb           = i2cHWA;
        _u8x8.gpio_and_delay_cb = gpioDelay;
        _rows                   = 4;
        _columns                = 16;
        success                 = true;
    }
    else if ((resolution == displayResolution_t::_128x32) && (controller == displayController_t::ssd1306))
    {
        _u8x8.display_cb        = u8x8_d_ssd1306_128x32_univision;
        _u8x8.cad_cb            = u8x8_cad_ssd13xx_i2c;
        _u8x8.byte_cb           = i2cHWA;
        _u8x8.gpio_and_delay_cb = gpioDelay;
        _rows                   = 2;
        _columns                = 16;
        success                 = true;
    }

    if (success)
    {
        _u8x8.i2c_address = i2cAddress;
        u8x8_SetupMemory(&_u8x8);
        u8x8_InitDisplay(&_u8x8);
        u8x8_SetFont(&_u8x8, u8x8_font_pxplustandynewtv_r);
        u8x8_ClearDisplay(&_u8x8);
        u8x8_SetPowerSave(&_u8x8, false);

        return true;
    }

    return false;
}

bool Display::deInit()
{
    if (!_initialized)
    {
        return false;    // nothing to do
    }

    u8x8_SetupDefaults(&_u8x8);

    _rows        = 0;
    _columns     = 0;
    _initialized = false;

    return true;
}

const uint8_t* Display::addresses(size_t& amount)
{
    amount = sizeof(_i2cAddress) / sizeof(uint8_t);
    return _i2cAddress;
}

/// Checks if LCD requires updating continuously.
void Display::update()
{
    if (!_initialized)
    {
        return;
    }

    if ((core::timing::currentRunTimeMs() - _lastLCDupdateTime) < LCD_REFRESH_TIME)
    {
        return;    // we don't need to update lcd in real time
    }

    // use char pointer to point to line we're going to print
    char* charPointer;

    for (int i = 0; i < LCD_HEIGHT_MAX; i++)
    {
        charPointer = _lcdRowText[i];

        if (!_charChange[i])
        {
            continue;
        }

        int8_t string_len = strlen(charPointer) > LCD_WIDTH_MAX ? LCD_WIDTH_MAX : strlen(charPointer);

        for (int j = 0; j < string_len; j++)
        {
            if (BIT_READ(_charChange[i], j))
            {
                u8x8_DrawGlyph(&_u8x8, j, _rowMap[_resolution][i], charPointer[j]);
            }
        }

        // now fill remaining columns with spaces
        for (uint16_t j = string_len; j < LCD_WIDTH_MAX; j++)
        {
            u8x8_DrawGlyph(&_u8x8, j, _rowMap[_resolution][i], ' ');
        }

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
            {
                clearMIDIevent(static_cast<eventType_t>(i));
            }
        }
    }
}

/// Updates text to be shown on display.
/// This function only updates internal buffers with received text, actual updating is done in update() function.
/// Text isn't passed directly, instead, value from string builder is used.
/// param [in]: row             Row which is being updated.
/// param [in]: startIndex      Index on which received text should on specified row.
void Display::updateText(uint8_t row, uint8_t startIndex)
{
    if (!_initialized)
    {
        return;
    }

    auto string = _stringBuilder.string();
    auto size   = strlen(string);

    if ((size + startIndex) >= (LCD_STRING_BUFFER_SIZE - 2))
    {
        size = LCD_STRING_BUFFER_SIZE - 2 - startIndex;    // trim string
    }

    for (size_t i = 0; i < size; i++)
    {
        if (_lcdRowText[row][startIndex + i] != string[i])
        {
            BIT_SET(_charChange[row], startIndex + i);
        }

        _lcdRowText[row][startIndex + i] = string[i];
    }
}

/// Calculates position on which text needs to be set on display to be in center of display row.
/// param [in]: textSize    Size of text for which center position on display is being calculated.
/// returns: Center position of text on display.
uint8_t Display::getTextCenter(uint8_t textSize)
{
    return _columns / 2 - (textSize / 2);
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
    {
        return;
    }

    uint8_t startRow;
    bool    showBoard = false;

    u8x8_ClearDisplay(&_u8x8);

    switch (_rows)
    {
    case 4:
    {
        startRow  = 1;
        showBoard = true;
    }
    break;

    default:
    {
        startRow = 0;
    }
    break;
    }

    auto writeString = [&](const char* string, uint8_t row) {
        uint8_t charIndex = 0;
        uint8_t location  = getTextCenter(strlen(string));

        while (string[charIndex] != '\0')
        {
            u8x8_DrawGlyph(&_u8x8, location + charIndex, _rowMap[_resolution][startRow], string[charIndex]);
            charIndex++;
        }
    };

    _stringBuilder.overwrite("OpenDeck");
    writeString(_stringBuilder.string(), startRow);

    startRow++;
    _stringBuilder.overwrite("FW: v%d.%d.%d", SW_VERSION_MAJOR, SW_VERSION_MINOR, SW_VERSION_REVISION);
    writeString(_stringBuilder.string(), startRow);

    if (showBoard)
    {
        startRow++;
        _stringBuilder.overwrite("HW: %s", Strings::board());
        writeString(_stringBuilder.string(), startRow);
    }

    core::timing::waitMs(2000);
}

void Display::displayMIDIevent(eventType_t type, const Messaging::event_t& event)
{
    if (!_initialized)
    {
        return;
    }

    uint8_t startRow    = (type == Display::eventType_t::in) ? ROW_START_MIDI_IN_MESSAGE : ROW_START_MIDI_OUT_MESSAGE;
    uint8_t startColumn = (type == Display::eventType_t::in) ? COLUMN_START_MIDI_IN_MESSAGE : COLUMN_START_MIDI_OUT_MESSAGE;

    _stringBuilder.overwrite("%s", Strings::midiMessage(event.message));
    _stringBuilder.fillUntil(_columns - startColumn - strlen(_stringBuilder.string()));
    updateText(startRow, startColumn);

    switch (event.message)
    {
    case MIDI::messageType_t::noteOff:
    case MIDI::messageType_t::noteOn:
    {
        if (!_database.read(Database::Config::Section::i2c_t::display, setting_t::MIDInotesAlternate))
        {
            _stringBuilder.overwrite("%d", event.midiIndex);
        }
        else
        {
            _stringBuilder.overwrite("%s%d", Strings::note(MIDI::noteToTonic(event.midiIndex)), normalizeOctave(MIDI::noteToOctave(event.midiValue), _octaveNormalization));
        }

        _stringBuilder.append(" v%d CH%d", event.midiValue, event.midiChannel);
        _stringBuilder.fillUntil(_columns - strlen(_stringBuilder.string()));
        updateText(startRow + 1, 0);
    }
    break;

    case MIDI::messageType_t::programChange:
    {
        _stringBuilder.overwrite("%d CH%d", event.midiIndex, event.midiChannel);
        _stringBuilder.fillUntil(_columns - strlen(_stringBuilder.string()));
        updateText(startRow + 1, 0);
    }
    break;

    case MIDI::messageType_t::controlChange:
    case MIDI::messageType_t::controlChange14bit:
    case MIDI::messageType_t::nrpn7bit:
    case MIDI::messageType_t::nrpn14bit:
    {
        _stringBuilder.overwrite("%d %d CH%d", event.midiIndex, event.midiValue, event.midiChannel);
        _stringBuilder.fillUntil(_columns - strlen(_stringBuilder.string()));
        updateText(startRow + 1, 0);
    }
    break;

    case MIDI::messageType_t::mmcPlay:
    case MIDI::messageType_t::mmcStop:
    case MIDI::messageType_t::mmcRecordStart:
    case MIDI::messageType_t::mmcRecordStop:
    case MIDI::messageType_t::mmcPause:
    {
        _stringBuilder.overwrite("CH%d", event.midiIndex);
        _stringBuilder.fillUntil(_columns - strlen(_stringBuilder.string()));
        updateText(startRow + 1, 0);
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
        _stringBuilder.fillUntil(_columns);
        updateText(startRow + 1, 0);
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
    {
        return;
    }

    switch (type)
    {
    case eventType_t::in:
    {
        // first row
        _stringBuilder.overwrite("In: ");
        _stringBuilder.fillUntil(_columns - strlen(_stringBuilder.string()));
        updateText(ROW_START_MIDI_IN_MESSAGE, 0);
        // second row
        _stringBuilder.overwrite("");
        _stringBuilder.fillUntil(_columns - strlen(_stringBuilder.string()));
        updateText(ROW_START_MIDI_IN_MESSAGE + 1, 0);
    }
    break;

    case eventType_t::out:
    {
        // first row
        _stringBuilder.overwrite("Out: ");
        _stringBuilder.fillUntil(_columns - strlen(_stringBuilder.string()));
        updateText(ROW_START_MIDI_OUT_MESSAGE, 0);
        // second row
        _stringBuilder.overwrite("");
        _stringBuilder.fillUntil(_columns - strlen(_stringBuilder.string()));
        updateText(ROW_START_MIDI_OUT_MESSAGE + 1, 0);
    }
    break;

    default:
        return;
    }

    _midiMessageDisplayed[type] = false;
}

std::optional<uint8_t> Display::sysConfigGet(System::Config::Section::i2c_t section, size_t index, uint16_t& value)
{
    if (section != System::Config::Section::i2c_t::display)
    {
        return std::nullopt;
    }

    int32_t readValue;
    auto    result = _database.read(Util::Conversion::sys2DBsection(section), index, readValue) ? System::Config::status_t::ack : System::Config::status_t::errorRead;

    value = readValue;

    return result;
}

std::optional<uint8_t> Display::sysConfigSet(System::Config::Section::i2c_t section, size_t index, uint16_t value)
{
    if (section != System::Config::Section::i2c_t::display)
    {
        return std::nullopt;
    }

    auto initAction = Common::initAction_t::asIs;

    switch (section)
    {
    case System::Config::Section::i2c_t::display:
    {
        auto setting = static_cast<setting_t>(index);

        switch (setting)
        {
        case setting_t::enable:
        {
            initAction = value ? Common::initAction_t::init : Common::initAction_t::deInit;
        }
        break;

        case setting_t::controller:
        {
            if ((value <= static_cast<uint8_t>(displayController_t::AMOUNT)) && (value >= 0))
            {
                initAction = Common::initAction_t::init;
            }
        }
        break;

        case setting_t::resolution:
        {
            if ((value <= static_cast<uint8_t>(displayResolution_t::AMOUNT)) && (value >= 0))
            {
                initAction = Common::initAction_t::init;
            }
        }
        break;

        case setting_t::MIDIeventTime:
        {
            setRetentionTime(value * 1000);
        }
        break;

        default:
            break;
        }
    }
    break;

    default:
        break;
    }

    auto result = _database.update(Util::Conversion::sys2DBsection(section), index, value) ? System::Config::status_t::ack : System::Config::status_t::errorWrite;

    if (result == System::Config::status_t::ack)
    {
        if (initAction == Common::initAction_t::init)
        {
            init(_selectedI2Caddress);
        }
        else if (initAction == Common::initAction_t::deInit)
        {
            deInit();
        }
    }

    return result;
}
