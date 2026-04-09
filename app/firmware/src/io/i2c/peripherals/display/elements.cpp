/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_DISPLAY

#include "display.h"

#include "zlibs/utils/misc/bit.h"

#include <zephyr/kernel.h>

using namespace io::i2c::display;
using namespace protocol;

void Display::Elements::update()
{
    if ((k_uptime_get_32() - _last_refresh_time) < REFRESH_TIME)
    {
        return;    // we don't need to update lcd in real time
    }

    for (size_t i = 0; i < _elements.size(); i++)
    {
        auto element = _elements.at(i);

        if (_message_retention_time)
        {
            if (element->use_retention())
            {
                if (strlen(element->text()))
                {
                    if (((k_uptime_get_32() - element->last_update_time()) > _message_retention_time))
                    {
                        element->set_text("");
                    }
                }
            }
        }

        auto change = element->change();

        if (change)
        {
            for (size_t index = 0; index < element->max_length(); index++)
            {
                if (zlibs::utils::misc::bit_read(change, index))
                {
                    u8x8_DrawGlyph(&_display._u8x8,
                                   element->column() + index,
                                   Display::ROW_MAP[_display._resolution][element->row()],
                                   element->text()[index]);
                }
            }

            element->clear_change();
        }
    }

    _last_refresh_time = k_uptime_get_32();
}

void Display::Elements::set_retention_time(uint32_t retention_time)
{
    _message_retention_time = retention_time;

    // clear out elements immediately on change
    for (size_t i = 0; i < _elements.size(); i++)
    {
        auto element = _elements.at(i);

        if (_message_retention_time)
        {
            if (element->use_retention())
            {
                if (strlen(element->text()))
                {
                    element->set_text("");
                }
            }
        }
    }
}

void Display::Elements::use_alternate_note(bool state)
{
    _midi_updater.use_alternate_note(state);
}

void Display::Elements::set_preset(uint8_t preset)
{
    _preset.set_preset(preset);
}

void Display::Elements::MIDIUpdater::use_alternate_note(bool state)
{
    _use_alternate_note = state;
}

void Display::Elements::MIDIUpdater::update_midi_value(DisplayTextControl& element,
                                                       uint8_t             channel,
                                                       uint16_t            index,
                                                       uint16_t            value,
                                                       midi::MessageType   message)
{
    switch (message)
    {
    case midi::MessageType::NoteOff:
    case midi::MessageType::NoteOn:
    {
        if (!_use_alternate_note)
        {
            element.set_text("CH%d %d v%d", channel, index, value);
        }
        else
        {
            element.set_text("CH%d %s%d v%d",
                             channel,
                             Strings::note(midi::note_to_tonic(static_cast<int8_t>(index))),
                             midi::note_to_octave(static_cast<int8_t>(index)),
                             value);
        }
    }
    break;

    case midi::MessageType::ProgramChange:
    {
        element.set_text("CH%d %d", channel, index);
    }
    break;

    case midi::MessageType::ControlChange:
    case midi::MessageType::ControlChange14Bit:
    case midi::MessageType::Nrpn7Bit:
    case midi::MessageType::Nrpn14Bit:
    {
        element.set_text("CH%d %d %d", channel, index, value);
    }
    break;

    case midi::MessageType::MmcPlay:
    case midi::MessageType::MmcStop:
    case midi::MessageType::MmcRecordStart:
    case midi::MessageType::MmcRecordStop:
    case midi::MessageType::MmcPause:
    {
        element.set_text("CH%d", index);
    }
    break;

    case midi::MessageType::SysRealTimeClock:
    case midi::MessageType::SysRealTimeStart:
    case midi::MessageType::SysRealTimeContinue:
    case midi::MessageType::SysRealTimeStop:
    case midi::MessageType::SysRealTimeActiveSensing:
    case midi::MessageType::SysRealTimeSystemReset:
    case midi::MessageType::SysEx:
    default:
        break;
    }
}

#endif
