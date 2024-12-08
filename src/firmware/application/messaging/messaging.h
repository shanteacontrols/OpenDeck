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

#pragma once

#include "application/util/dispatcher/dispatcher.h"

#include "lib/midi/midi.h"

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
        RESTORE_END,
        FACTORY_RESET_START,
        FACTORY_RESET_END,
        MIDI_BPM_CHANGE,
    };

    struct Event
    {
        size_t                   componentIndex = 0;
        uint8_t                  channel        = 0;
        uint16_t                 index          = 0;
        uint16_t                 value          = 0;
        uint8_t*                 sysEx          = nullptr;
        size_t                   sysExLength    = 0;
        bool                     forcedRefresh  = false;
        lib::midi::messageType_t message        = lib::midi::messageType_t::INVALID;
        systemMessage_t          systemMessage  = systemMessage_t::FORCE_IO_REFRESH;
    };
}    // namespace messaging

#define MidiDispatcher util::Dispatcher<messaging::eventType_t, messaging::Event>::instance()