/*

Copyright Igor Petrovic

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

#ifdef PROJECT_TARGET_SUPPORT_DISPLAY

#include "display.h"

#include "application/io/i2c/i2c.h"
#include "application/protocol/midi/midi.h"
#include "application/io/common/common.h"
#include "application/util/conversion/conversion.h"
#include "application/util/configurable/configurable.h"

#include "core/mcu.h"

using namespace io::i2c::display;
using namespace protocol;

Display::Display(Hwa&      hwa,
                 Database& database)
    : _hwa(hwa)
    , _database(database)
{
    ConfigHandler.registerConfig(
        sys::Config::block_t::I2C,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sysConfigGet(static_cast<sys::Config::Section::i2c_t>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sysConfigSet(static_cast<sys::Config::Section::i2c_t>(section), index, value);
        });

    I2c::registerPeripheral(this);
}

bool Display::init()
{
    if (!_hwa.init())
    {
        return false;
    }

    bool addressFound = false;

    for (size_t address = 0; address < I2C_ADDRESS.size(); address++)
    {
        if (_hwa.deviceAvailable(I2C_ADDRESS[address]))
        {
            addressFound        = true;
            _selectedI2Caddress = I2C_ADDRESS[address];
            break;
        }
    }

    if (!addressFound)
    {
        return false;
    }

    if (_database.read(database::Config::Section::i2c_t::DISPLAY, setting_t::ENABLE))
    {
        auto controller = static_cast<displayController_t>(_database.read(database::Config::Section::i2c_t::DISPLAY, setting_t::CONTROLLER));
        auto resolution = static_cast<displayResolution_t>(_database.read(database::Config::Section::i2c_t::DISPLAY, setting_t::RESOLUTION));

        if (initU8X8(_selectedI2Caddress, controller, resolution))
        {
            _resolution  = resolution;
            _initialized = true;

            if (!_startupInfoShown)
            {
                if (_database.read(database::Config::Section::i2c_t::DISPLAY, setting_t::DEVICE_INFO_MSG) && !_startupInfoShown)
                {
                    displayWelcomeMessage();
                    u8x8_ClearDisplay(&_u8x8);
                }
            }

            _elements._midiUpdater.useAlternateNote(_database.read(database::Config::Section::i2c_t::DISPLAY,
                                                                   setting_t::MIDI_NOTES_ALTERNATE));
            _elements._preset.setPreset(_database.getPreset());
            _elements.setRetentionTime(_database.read(database::Config::Section::i2c_t::DISPLAY, setting_t::EVENT_TIME) * 1000);
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
    _u8x8.user_ptr = this;

    // i2c hw access
    auto gpioDelay = [](u8x8_t* u8x8, uint8_t msg, uint8_t argInt, U8X8_UNUSED void* argPtr) -> uint8_t
    {
        return 0;
    };

    auto i2cHWA = [](u8x8_t* u8x8, uint8_t msg, uint8_t argInt, void* argPtr) -> uint8_t
    {
        auto instance = static_cast<Display*>(u8x8->user_ptr);
        auto data     = static_cast<uint8_t*>(argPtr);

        switch (msg)
        {
        case U8X8_MSG_BYTE_SEND:
        {
            memcpy(&instance->_u8x8Buffer[instance->_u8x8Counter], data, argInt);
            instance->_u8x8Counter += argInt;
        }
        break;

        case U8X8_MSG_BYTE_INIT:
            break;

        case U8X8_MSG_BYTE_START_TRANSFER:
        {
            instance->_u8x8Counter = 0;
        }
        break;

        case U8X8_MSG_BYTE_END_TRANSFER:
            return instance->_hwa.write(u8x8_GetI2CAddress(u8x8), instance->_u8x8Buffer, instance->_u8x8Counter);

        default:
            return 0;
        }

        return 1;
    };

    // setup specific callbacks depending on controller/resolution
    if ((resolution == displayResolution_t::R128X64) && (controller == displayController_t::SSD1306))
    {
        _u8x8.display_cb        = u8x8_d_ssd1306_128x64_noname;
        _u8x8.cad_cb            = u8x8_cad_ssd13xx_i2c;
        _u8x8.byte_cb           = i2cHWA;
        _u8x8.gpio_and_delay_cb = gpioDelay;
        _rows                   = 4;
        success                 = true;
    }
    else if ((resolution == displayResolution_t::R128X32) && (controller == displayController_t::SSD1306))
    {
        _u8x8.display_cb        = u8x8_d_ssd1306_128x32_univision;
        _u8x8.cad_cb            = u8x8_cad_ssd13xx_i2c;
        _u8x8.byte_cb           = i2cHWA;
        _u8x8.gpio_and_delay_cb = gpioDelay;
        _rows                   = 2;
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
    _initialized = false;

    return true;
}

/// Checks if LCD requires updating continuously.
void Display::update()
{
    if (!_initialized)
    {
        return;
    }

    _elements.update();
}

/// Calculates position on which text needs to be set on display to be in center of display row.
/// param [in]: textSize    Size of text for which center position on display is being calculated.
/// returns: Center position of text on display.
uint8_t Display::getTextCenter(uint8_t textSize)
{
    return MAX_COLUMNS / 2 - (textSize / 2);
}

void Display::displayWelcomeMessage()
{
    if (!_initialized)
    {
        return;
    }

    uint8_t startRow;
    bool    showBoard = _rows >= 4;

    u8x8_ClearDisplay(&_u8x8);

    switch (_rows)
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

    auto writeString = [&](uint8_t row, const char* text, ...)
    {
        char tempBuff[MAX_COLUMNS + 1] = {};

        va_list args;
        va_start(args, text);
        vsnprintf(tempBuff, sizeof(tempBuff), text, args);
        va_end(args);

        uint8_t charIndex = 0;
        uint8_t location  = getTextCenter(strlen(tempBuff));

        while (tempBuff[charIndex] != '\0')
        {
            u8x8_DrawGlyph(&_u8x8, location + charIndex, ROW_MAP[_resolution][startRow], tempBuff[charIndex]);
            charIndex++;
        }
    };

    writeString(startRow, "OpenDeck");

    startRow++;
    writeString(startRow, "FW: v%d.%d.%d", SW_VERSION_MAJOR, SW_VERSION_MINOR, SW_VERSION_REVISION);

    if (showBoard)
    {
        startRow++;
        writeString(startRow, "HW: %s", Strings::TARGET_NAME_STRING);
    }

    core::mcu::timing::waitMs(2000);
}

std::optional<uint8_t> Display::sysConfigGet(sys::Config::Section::i2c_t section, size_t index, uint16_t& value)
{
    if (section != sys::Config::Section::i2c_t::DISPLAY)
    {
        return std::nullopt;
    }

    uint32_t readValue;

    auto result = _database.read(util::Conversion::SYS_2_DB_SECTION(section), index, readValue)
                      ? sys::Config::Status::ACK
                      : sys::Config::Status::ERROR_READ;

    value = readValue;

    return result;
}

std::optional<uint8_t> Display::sysConfigSet(sys::Config::Section::i2c_t section, size_t index, uint16_t value)
{
    if (section != sys::Config::Section::i2c_t::DISPLAY)
    {
        return std::nullopt;
    }

    auto initAction = common::initAction_t::AS_IS;

    switch (section)
    {
    case sys::Config::Section::i2c_t::DISPLAY:
    {
        auto setting = static_cast<setting_t>(index);

        switch (setting)
        {
        case setting_t::ENABLE:
        {
            initAction = value ? common::initAction_t::INIT : common::initAction_t::DE_INIT;
        }
        break;

        case setting_t::CONTROLLER:
        {
            if ((value <= static_cast<uint8_t>(displayController_t::AMOUNT)) && (value >= 0))
            {
                initAction = common::initAction_t::INIT;
            }
        }
        break;

        case setting_t::RESOLUTION:
        {
            if ((value <= static_cast<uint8_t>(displayResolution_t::AMOUNT)) && (value >= 0))
            {
                initAction = common::initAction_t::INIT;
            }
        }
        break;

        case setting_t::EVENT_TIME:
        {
            _elements.setRetentionTime(value * 1000);
        }
        break;

        case setting_t::MIDI_NOTES_ALTERNATE:
        {
            _elements._midiUpdater.useAlternateNote(value);
        };

        default:
            break;
        }
    }
    break;

    default:
        break;
    }

    auto result = _database.update(util::Conversion::SYS_2_DB_SECTION(section), index, value)
                      ? sys::Config::Status::ACK
                      : sys::Config::Status::ERROR_WRITE;

    if (result == sys::Config::Status::ACK)
    {
        if (initAction == common::initAction_t::INIT)
        {
            init();
        }
        else if (initAction == common::initAction_t::DE_INIT)
        {
            deInit();
        }
    }

    return result;
}

#endif