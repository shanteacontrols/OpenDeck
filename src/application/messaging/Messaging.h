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

namespace Messaging
{
    enum class eventSource_t : uint8_t
    {
        analog,
        buttons,
        encoders,
        touchscreenButton,
        touchscreenScreen,
        midiIn,
        preset,
        program,
        system,
        leds
    };

    // enum indicating what types of system-level messages are possible.
    // When system message is used, its type should be stored into componentIndex
    // in event_t structure.
    enum class systemMessage_t : uint8_t
    {
        forceIOrefresh,
        sysExResponse
    };

    struct event_t
    {
        size_t                       componentIndex = 0;
        uint8_t                      midiChannel    = 0;
        uint16_t                     midiIndex      = 0;
        uint16_t                     midiValue      = 0;
        uint8_t*                     sysEx          = nullptr;
        size_t                       sysExLength    = 0;
        MIDIlib::Base::messageType_t message        = MIDIlib::Base::messageType_t::invalid;

        event_t() = default;
    };

    using listenType_t = Util::Dispatcher<eventSource_t, event_t>::listenType_t;
}    // namespace Messaging

#define MIDIDispatcher Util::Dispatcher<Messaging::eventSource_t, Messaging::event_t>::instance()