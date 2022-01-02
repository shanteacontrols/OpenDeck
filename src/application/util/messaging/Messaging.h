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

#include <vector>
#include <functional>
#include "midi/src/MIDI.h"

namespace Util
{
    class MessageDispatcher
    {
        public:
        enum class messageSource_t : uint8_t
        {
            analog,
            buttons,
            encoders,
            touchscreenButton,
            touchscreenAnalog,
            touchscreenScreen,
            midiIn,
            preset,
            system,
            leds
        };

        enum class listenType_t : uint8_t
        {
            nonFwd,
            fwd,
            all
        };

        // enum indicating what types of system-level messages are possible.
        // When system message is used, its type should be stored into componentIndex
        // in message_t structure.
        enum class systemMessages_t : uint8_t
        {
            forceIOrefresh,
            sysExResponse,
            midiProgramIndication
        };

        struct message_t
        {
            size_t              componentIndex = 0;
            uint8_t             midiChannel    = 0;
            uint16_t            midiIndex      = 0;
            uint16_t            midiValue      = 0;
            uint8_t*            sysEx          = nullptr;
            size_t              sysExLength    = 0;
            MIDI::messageType_t message        = MIDI::messageType_t::invalid;

            message_t() = default;
        };

        using messageCallback_t = std::function<void(const message_t& message)>;

        static MessageDispatcher& instance()
        {
            static MessageDispatcher _instance;
            return _instance;
        }

        void listen(messageSource_t source, listenType_t listenType, messageCallback_t&& callback);
        void notify(messageSource_t source, message_t const& message, listenType_t listenType);

        private:
        MessageDispatcher() = default;

        struct listener_t
        {
            messageSource_t   source;
            listenType_t      listenType = listenType_t::nonFwd;
            messageCallback_t callback   = nullptr;
        };

        std::vector<listener_t> _listener = {};
    };
}    // namespace Util

#define Dispatcher Util::MessageDispatcher::instance()