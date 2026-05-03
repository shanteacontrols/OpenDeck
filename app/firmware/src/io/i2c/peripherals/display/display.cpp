/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_DISPLAY

#include "display.h"

#include "io/i2c/i2c.h"
#include "protocol/midi/midi.h"
#include "io/common/common.h"
#include "util/conversion/conversion.h"
#include "util/configurable/configurable.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

using namespace opendeck::io::i2c::display;
using namespace opendeck::protocol;

namespace
{
    LOG_MODULE_REGISTER(display, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    constexpr uint16_t EVENT_RETENTION_TIME_SCALE_MS = 1000;
    constexpr uint16_t WELCOME_MESSAGE_DELAY_MS      = 2000;
}    // namespace

Display::Display(Hwa&      hwa,
                 Database& database)
    : _hwa(hwa)
    , _database(database)
{
    ConfigHandler.register_config(
        sys::Config::Block::I2c,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sys_config_get(static_cast<sys::Config::Section::I2c>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sys_config_set(static_cast<sys::Config::Section::I2c>(section), index, value);
        });

    I2c::register_peripheral(this);
}

bool Display::init()
{
    if (!_hwa.init())
    {
        return false;
    }

    bool address_found = false;

    for (size_t address = 0; address < I2C_ADDRESS.size(); address++)
    {
        if (_hwa.device_available(I2C_ADDRESS[address]))
        {
            address_found         = true;
            _selected_i2c_address = I2C_ADDRESS[address];
            break;
        }
    }

    if (!address_found)
    {
        return false;
    }

    if (_database.read(database::Config::Section::I2c::Display, Setting::Enable))
    {
        auto controller = static_cast<DisplayController>(_database.read(database::Config::Section::I2c::Display, Setting::Controller));
        auto resolution = static_cast<DisplayResolution>(_database.read(database::Config::Section::I2c::Display, Setting::Resolution));

        if (init_u8x8(_selected_i2c_address, controller, resolution))
        {
            _resolution  = resolution;
            _initialized = true;

            if (!_startup_info_shown)
            {
                if (_database.read(database::Config::Section::I2c::Display, Setting::DeviceInfoMsg) && !_startup_info_shown)
                {
                    display_welcome_message();
                    u8x8_ClearDisplay(&_u8x8);
                }
            }

            _elements.use_alternate_note(_database.read(database::Config::Section::I2c::Display,
                                                        Setting::MidiNotesAlternate));
            _elements.set_preset(_database.current_preset());
            _elements.set_retention_time(_database.read(database::Config::Section::I2c::Display, Setting::EventTime) *
                                         EVENT_RETENTION_TIME_SCALE_MS);
        }
        else
        {
            _initialized = false;
            return false;
        }
    }

    // make sure welcome message on startup isn't shown anymore when init is called
    _startup_info_shown = true;

    return _initialized;
}

bool Display::init_u8x8(uint8_t i2c_address, DisplayController controller, DisplayResolution resolution)
{
    bool success = false;

    // setup defaults
    u8x8_SetupDefaults(&_u8x8);
    _u8x8.user_ptr = this;

    // i2c hw access
    auto gpio_delay = []([[maybe_unused]] u8x8_t* u8x8,
                         [[maybe_unused]] uint8_t msg,
                         [[maybe_unused]] uint8_t arg_int,
                         [[maybe_unused]] void*   arg_ptr) -> uint8_t
    {
        return 0;
    };

    auto i2c_hwa = [](u8x8_t* u8x8,
                      uint8_t msg,
                      uint8_t arg_int,
                      void*   arg_ptr) -> uint8_t
    {
        auto self = static_cast<Display*>(u8x8->user_ptr);
        auto data = static_cast<uint8_t*>(arg_ptr);

        switch (msg)
        {
        case U8X8_MSG_BYTE_SEND:
        {
            memcpy(&self->_u8x8_buffer[self->_u8x8_counter], data, arg_int);
            self->_u8x8_counter += arg_int;
        }
        break;

        case U8X8_MSG_BYTE_INIT:
            break;

        case U8X8_MSG_BYTE_START_TRANSFER:
        {
            self->_u8x8_counter = 0;
        }
        break;

        case U8X8_MSG_BYTE_END_TRANSFER:
            return self->_hwa.write(u8x8_GetI2CAddress(u8x8), self->_u8x8_buffer, self->_u8x8_counter);

        default:
            return 0;
        }

        return 1;
    };

    // setup specific callbacks depending on controller/resolution
    if ((resolution == DisplayResolution::Res128x64) && (controller == DisplayController::Ssd1306))
    {
        _u8x8.display_cb        = u8x8_d_ssd1306_128x64_noname;
        _u8x8.cad_cb            = u8x8_cad_ssd13xx_i2c;
        _u8x8.byte_cb           = i2c_hwa;
        _u8x8.gpio_and_delay_cb = gpio_delay;
        _rows                   = 4;
        success                 = true;
    }
    else if ((resolution == DisplayResolution::Res128x32) && (controller == DisplayController::Ssd1306))
    {
        _u8x8.display_cb        = u8x8_d_ssd1306_128x32_univision;
        _u8x8.cad_cb            = u8x8_cad_ssd13xx_i2c;
        _u8x8.byte_cb           = i2c_hwa;
        _u8x8.gpio_and_delay_cb = gpio_delay;
        _rows                   = 2;
        success                 = true;
    }

    if (success)
    {
        _u8x8.i2c_address = i2c_address;
        u8x8_SetupMemory(&_u8x8);
        u8x8_InitDisplay(&_u8x8);
        u8x8_SetFont(&_u8x8, u8x8_font_pxplustandynewtv_r);
        u8x8_ClearDisplay(&_u8x8);
        u8x8_SetPowerSave(&_u8x8, false);

        return true;
    }

    return false;
}

bool Display::deinit()
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

void Display::update()
{
    if (!_initialized)
    {
        return;
    }

    _elements.update();
}

uint8_t Display::get_text_center(uint8_t text_size)
{
    return MAX_COLUMNS / 2 - (text_size / 2);
}

void Display::display_welcome_message()
{
    if (!_initialized)
    {
        return;
    }

    uint8_t start_row;
    bool    show_board = _rows >= 4;

    u8x8_ClearDisplay(&_u8x8);

    switch (_rows)
    {
    case 4:
    {
        start_row = 1;
    }
    break;

    default:
    {
        start_row = 0;
    }
    break;
    }

    auto write_string = [&](const char* text, auto... args)
    {
        static constexpr size_t FORMAT_BUFFER_SIZE            = 64;
        char                    temp_buff[FORMAT_BUFFER_SIZE] = {};

        if constexpr (sizeof...(args) == 0)
        {
            [[maybe_unused]] auto ret = snprintf(temp_buff, sizeof(temp_buff), "%s", text);
        }
        else
        {
            [[maybe_unused]] auto ret = snprintf(temp_buff, sizeof(temp_buff), text, args...);
        }

        uint8_t char_index = 0;
        uint8_t location   = get_text_center(strlen(temp_buff));

        while (temp_buff[char_index] != '\0')
        {
            u8x8_DrawGlyph(&_u8x8, location + char_index, ROW_MAP[_resolution][start_row], temp_buff[char_index]);
            char_index++;
        }
    };

    write_string("OpenDeck");

    start_row++;
    write_string("FW: v%d.%d.%d",
                 OPENDECK_SW_VERSION_MAJOR,
                 OPENDECK_SW_VERSION_MINOR,
                 OPENDECK_SW_VERSION_REVISION);

    if (show_board)
    {
        start_row++;
        write_string("HW: %s", Strings::TARGET_NAME_STRING);
    }

    k_msleep(WELCOME_MESSAGE_DELAY_MS);
}

std::optional<uint8_t> Display::sys_config_get(sys::Config::Section::I2c section, size_t index, uint16_t& value)
{
    if (section != sys::Config::Section::I2c::Display)
    {
        return {};
    }

    uint32_t read_value = 0;

    auto result = _database.read(util::Conversion::sys_2_db_section(section), index, read_value)
                      ? sys::Config::Status::Ack
                      : sys::Config::Status::ErrorRead;

    value = read_value;

    return result;
}

std::optional<uint8_t> Display::sys_config_set(sys::Config::Section::I2c section, size_t index, uint16_t value)
{
    if (section != sys::Config::Section::I2c::Display)
    {
        return {};
    }

    auto init_action = common::InitAction::AsIs;

    switch (section)
    {
    case sys::Config::Section::I2c::Display:
    {
        auto setting = static_cast<Setting>(index);

        switch (setting)
        {
        case Setting::Enable:
        {
            init_action = value ? common::InitAction::Init : common::InitAction::DeInit;
        }
        break;

        case Setting::Controller:
        {
            if ((value <= static_cast<uint8_t>(DisplayController::Count)) && (value >= 0))
            {
                init_action = common::InitAction::Init;
            }
        }
        break;

        case Setting::Resolution:
        {
            if ((value <= static_cast<uint8_t>(DisplayResolution::Count)) && (value >= 0))
            {
                init_action = common::InitAction::Init;
            }
        }
        break;

        case Setting::EventTime:
        {
            _elements.set_retention_time(value * EVENT_RETENTION_TIME_SCALE_MS);
        }
        break;

        case Setting::MidiNotesAlternate:
        {
            _elements.use_alternate_note(value);
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

    auto result = _database.update(util::Conversion::sys_2_db_section(section), index, value)
                      ? sys::Config::Status::Ack
                      : sys::Config::Status::ErrorWrite;

    if (result == sys::Config::Status::Ack)
    {
        if (init_action == common::InitAction::Init)
        {
            init();
        }
        else if (init_action == common::InitAction::DeInit)
        {
            deinit();
        }
    }

    return result;
}

#endif
