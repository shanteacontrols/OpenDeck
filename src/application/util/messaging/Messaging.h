/*

Copyright 2015-2021 Igor Petrovic

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

#include <array>
#include <functional>
#include "midi/src/MIDI.h"

namespace IO
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
            midiIn
        };

        enum class listenType_t : uint8_t
        {
            nonFwd,
            forward,
            all
        };

        struct message_t
        {
            size_t              componentIndex = 0;
            uint8_t             midiChannel    = 0;
            uint16_t            midiIndex      = 0;
            uint16_t            midiValue      = 0;
            MIDI::messageType_t message        = MIDI::messageType_t::invalid;

            message_t() = default;
        };

        using messageCallback_t = std::function<void(const message_t& message)>;

        MessageDispatcher() = default;

        bool listen(messageSource_t source, listenType_t listenType, messageCallback_t callback);
        void notify(messageSource_t source, message_t const& message, listenType_t listenType);

        private:
        struct listener_t
        {
            messageSource_t   source;
            listenType_t      listenType = listenType_t::nonFwd;
            messageCallback_t callback   = nullptr;
        };

        static constexpr size_t               MAX_LISTENERS    = 20;
        std::array<listener_t, MAX_LISTENERS> _listener        = {};
        size_t                                _listenerCounter = 0;
    };
}    // namespace IO