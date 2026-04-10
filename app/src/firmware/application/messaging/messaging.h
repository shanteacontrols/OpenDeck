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

#include "application/protocol/midi/common.h"
#include "zlibs/utils/signaling/signaling.h"

#include <functional>
#include <vector>

namespace messaging
{
    // enum indicating what types of system-level messages are possible.
    enum class systemMessage_t : uint8_t
    {
        FORCE_IO_REFRESH,
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
        INIT_COMPLETE,
        MIDI_BPM_CHANGE,
    };

    enum class MidiDirection : uint8_t
    {
        In,
        Out,
    };

    enum class MidiTransport : uint8_t
    {
        Usb,
        Din,
        Ble,
    };

    enum class MidiSource : uint8_t
    {
        Button,
        Analog,
        AnalogButton,
        Encoder,
        TouchscreenButton,
        Program,
    };

    struct MidiSignal
    {
        MidiSource                    source         = MidiSource::Button;
        size_t                        componentIndex = 0;
        uint8_t                       channel        = 0;
        uint16_t                      index          = 0;
        uint16_t                      value          = 0;
        protocol::midi::messageType_t message        = protocol::midi::messageType_t::INVALID;
    };

    struct SystemSignal
    {
        systemMessage_t systemMessage = systemMessage_t::FORCE_IO_REFRESH;
        uint16_t        value         = 0;
    };

    struct TouchscreenScreenSignal
    {
        size_t componentIndex = 0;
    };

    struct TouchscreenLedSignal
    {
        size_t   componentIndex = 0;
        uint16_t value          = 0;
    };

    struct UmpSignal
    {
        MidiDirection direction = MidiDirection::Out;
        ::midi_ump    packet    = {};
    };

    struct MidiTrafficSignal
    {
        MidiTransport transport = MidiTransport::Usb;
        MidiDirection direction = MidiDirection::Out;
    };

    template<typename Signal>
    class SignalRegistry
    {
        public:
        template<typename Callback>
        void subscribe(Callback&& callback)
        {
            _subscriptions.push_back(
                zlibs::utils::signaling::subscribe<Signal>(std::forward<Callback>(callback)));
        }

        bool publish(const Signal& signal)
        {
            return zlibs::utils::signaling::publish(signal);
        }

        void clear()
        {
            for (auto& subscription : _subscriptions)
            {
                subscription.reset();
            }

            _subscriptions.clear();
        }

        static SignalRegistry& instance()
        {
            static SignalRegistry instance;
            return instance;
        }

        private:
        SignalRegistry() = default;

        std::vector<zlibs::utils::signaling::Subscription> _subscriptions = {};
    };

    template<typename Signal, typename Callback>
    void subscribe(Callback&& callback)
    {
        SignalRegistry<Signal>::instance().subscribe(std::forward<Callback>(callback));
    }

    template<typename Signal>
    bool publish(const Signal& signal)
    {
        return SignalRegistry<Signal>::instance().publish(signal);
    }
}    // namespace messaging
