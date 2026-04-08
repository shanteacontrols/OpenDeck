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

#include "core/mcu.h"

using namespace io::i2c::display;
using namespace protocol;

void Display::Elements::update()
{
    if ((core::mcu::timing::ms() - _lastRefreshTime) < REFRESH_TIME)
    {
        return;    // we don't need to update lcd in real time
    }

    for (size_t i = 0; i < _elements.size(); i++)
    {
        auto element = _elements.at(i);

        if (_messageRetentionTime)
        {
            if (element->USE_RETENTION())
            {
                if (strlen(element->text()))
                {
                    if (((core::mcu::timing::ms() - element->lastUpdateTime()) > _messageRetentionTime))
                    {
                        element->setText("");
                    }
                }
            }
        }

        auto change = element->change();

        if (change)
        {
            for (size_t index = 0; index < element->MAX_LENGTH(); index++)
            {
                if (core::util::BIT_READ(change, index))
                {
                    u8x8_DrawGlyph(&_display._u8x8,
                                   element->COLUMN() + index,
                                   Display::ROW_MAP[_display._resolution][element->ROW()],
                                   element->text()[index]);
                }
            }

            element->clearChange();
        }
    }

    _lastRefreshTime = core::mcu::timing::ms();
}

/// Sets new message retention time.
/// param [in]: retentionTime New retention time in milliseconds.
void Display::Elements::setRetentionTime(uint32_t retentionTime)
{
    _messageRetentionTime = retentionTime;

    // clear out elements immediately on change
    for (size_t i = 0; i < _elements.size(); i++)
    {
        auto element = _elements.at(i);

        if (_messageRetentionTime)
        {
            if (element->USE_RETENTION())
            {
                if (strlen(element->text()))
                {
                    element->setText("");
                }
            }
        }
    }
}

void Display::Elements::MIDIUpdater::useAlternateNote(bool state)
{
    _useAlternateNote = state;
}

void Display::Elements::MIDIUpdater::updateMIDIValue(DisplayTextControl& element, const messaging::Event& event)
{
    switch (event.message)
    {
    case midi::messageType_t::NOTE_OFF:
    case midi::messageType_t::NOTE_ON:
    {
        if (!_useAlternateNote)
        {
            element.setText("CH%d %d v%d", event.channel, event.index, event.value);
        }
        else
        {
            element.setText("CH%d %s%d v%d",
                            event.channel,
                            Strings::NOTE(midi::NOTE_TO_TONIC(event.index)),
                            midi::NOTE_TO_OCTAVE(event.value),
                            event.value);
        }
    }
    break;

    case midi::messageType_t::PROGRAM_CHANGE:
    {
        element.setText("CH%d %d", event.channel, event.index);
    }
    break;

    case midi::messageType_t::CONTROL_CHANGE:
    case midi::messageType_t::CONTROL_CHANGE_14BIT:
    case midi::messageType_t::NRPN_7BIT:
    case midi::messageType_t::NRPN_14BIT:
    {
        element.setText("CH%d %d %d", event.channel, event.index, event.value);
    }
    break;

    case midi::messageType_t::MMC_PLAY:
    case midi::messageType_t::MMC_STOP:
    case midi::messageType_t::MMC_RECORD_START:
    case midi::messageType_t::MMC_RECORD_STOP:
    case midi::messageType_t::MMC_PAUSE:
    {
        element.setText("CH%d", event.index);
    }
    break;

    case midi::messageType_t::SYS_REAL_TIME_CLOCK:
    case midi::messageType_t::SYS_REAL_TIME_START:
    case midi::messageType_t::SYS_REAL_TIME_CONTINUE:
    case midi::messageType_t::SYS_REAL_TIME_STOP:
    case midi::messageType_t::SYS_REAL_TIME_ACTIVE_SENSING:
    case midi::messageType_t::SYS_REAL_TIME_SYSTEM_RESET:
    case midi::messageType_t::SYS_EX:
    default:
        break;
    }
}

#endif