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

#pragma once

#include "util/dispatcher/Dispatcher.h"
#include "midi/src/MIDI.h"

namespace messaging
{
    enum class eventType_t : uint8_t
    {
        ANALOG,
        ANALOG_BUTTON,
        BUTTON,
        ENCODER,
        TOUCHSCREEN_BUTTON,
        TOUCHSCREEN_SCREEN,
        TOUCHSCREEN_LED,
        MIDI_IN,
        PROGRAM,
        SYSTEM,
    };

    // enum indicating what types of system-level messages are possible.
    enum class systemMessage_t : uint8_t
    {
        FORCE_IO_REFRESH,
        SYS_EX_RESPONSE,
        MIDI_PROGRAM_OFFSET_CHANGE,
        PRESET_CHANGE_INC_REQ,
        PRESET_CHANGE_DEC_REQ,
        PRESET_CHANGE_DIRECT_REQ,
        PRESET_CHANGED,
        BACKUP,
        RESTORE_START,
        RESTORE_END
    };

    struct event_t
    {
        size_t                       componentIndex = 0;
        uint8_t                      channel        = 0;
        uint16_t                     index          = 0;
        uint16_t                     value          = 0;
        uint8_t*                     sysEx          = nullptr;
        size_t                       sysExLength    = 0;
        MIDIlib::Base::messageType_t message        = MIDIlib::Base::messageType_t::INVALID;
        systemMessage_t              systemMessage  = systemMessage_t::FORCE_IO_REFRESH;

        event_t(size_t                       componentIndex,
                uint8_t                      channel,
                uint16_t                     index,
                uint16_t                     value,
                uint8_t*                     sysEx,
                size_t                       sysExLength,
                MIDIlib::Base::messageType_t message)
            : componentIndex(componentIndex)
            , channel(channel)
            , index(index)
            , value(value)
            , sysEx(sysEx)
            , sysExLength(sysExLength)
            , message(message)
        {}

        event_t() = default;
    };
}    // namespace messaging

#define MIDIDispatcher util::Dispatcher<messaging::eventType_t, messaging::event_t>::instance()