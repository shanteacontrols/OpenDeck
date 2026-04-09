/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_TOUCHSCREEN

#include "nextion.h"

#include "io/touchscreen/touchscreen.h"

#include <zephyr/kernel.h>

using namespace io::touchscreen;

Nextion::Nextion(Hwa& hwa)
    : _hwa(hwa)
{
    Touchscreen::register_model(ModelType::Nextion, this);
}

bool Nextion::init()
{
    Model::buffer_count = 0;

    if (_hwa.init())
    {
        // add slight delay to ensure display can receive commands after power on
        k_msleep(INIT_DELAY_MS);

        end_command();
        write_command("sendxy=1");

        return true;
    }

    return false;
}

bool Nextion::deinit()
{
    return _hwa.deinit();
}

bool Nextion::write_command(std::string_view line)
{
    if (line.size() >= Model::BUFFER_SIZE)
    {
        return false;
    }

    for (char value : line)
    {
        if (!_hwa.write(value))
        {
            return false;
        }
    }

    return end_command();
}

bool Nextion::set_screen(size_t index)
{
    return write_formatted_command("page %u", index);
}

TsEvent Nextion::update(Data& data)
{
    bool process = false;
    auto ret_val = TsEvent::None;

    while (true)
    {
        auto value = _hwa.read();

        if (!value.has_value())
        {
            break;
        }

        Model::rx_buffer[Model::buffer_count++] = value.value();

        if (value.value() == COMMAND_TERMINATOR)
        {
            _end_counter++;
        }
        else if (_end_counter)
        {
            _end_counter = 0;
        }

        if (_end_counter == COMMAND_TERMINATOR_SIZE)
        {
            // new message arrived
            _end_counter = 0;
            process      = true;
            break;
        }
    }

    if (process)
    {
        ret_val             = response(data);
        Model::buffer_count = 0;
    }

    return ret_val;
}

void Nextion::set_icon_state(Icon& icon, bool state)
{
    // ignore width/height zero - set either intentionally to avoid display or incorrectly
    if (!icon.width)
    {
        return;
    }

    if (!icon.height)
    {
        return;
    }

    write_formatted_command("picq %u,%u,%u,%u,%u", icon.x_pos, icon.y_pos, icon.width, icon.height, state ? icon.on_screen : icon.off_screen);
}

bool Nextion::end_command()
{
    for (int i = 0; i < COMMAND_TERMINATOR_SIZE; i++)
    {
        if (!_hwa.write(COMMAND_TERMINATOR))
        {
            return false;
        }
    }

    return true;
}

bool Nextion::set_brightness(Brightness brightness)
{
    return write_formatted_command("dims=%d", BRIGHTNESS_MAPPING[static_cast<uint8_t>(brightness)]);
}

TsEvent Nextion::response(Data& data)
{
    bool response_found = false;
    auto response       = ResponseId::Button;    // assumption for now

    for (size_t i = 0; i < static_cast<size_t>(ResponseId::Count); i++)
    {
        if (Model::buffer_count == RESPONSES[i].size)
        {
            if (Model::rx_buffer[0] == static_cast<uint8_t>(RESPONSES[i].response_id))
            {
                response       = static_cast<ResponseId>(i);
                response_found = true;
                break;
            }
        }
    }

    if (response_found)
    {
        switch (response)
        {
        case ResponseId::Button:
        {
            data.button_state = Model::rx_buffer[1];
            data.button_index = Model::rx_buffer[2];

            return TsEvent::Button;
        }
        break;

        default:
            return TsEvent::None;
        }
    }

    return TsEvent::None;
}

#endif
