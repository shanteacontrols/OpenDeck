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

#include "cinfo.h"
#include "application/messaging/messaging.h"

#include <zephyr/kernel.h>

using namespace util;

ComponentInfo::ComponentInfo()
{
    messaging::subscribe<messaging::MidiSignal>(
        [this](const messaging::MidiSignal& signal)
        {
            switch (signal.source)
            {
            case messaging::MidiSource::Analog:
            {
                send(database::Config::block_t::ANALOG, signal.componentIndex);
            }
            break;

            case messaging::MidiSource::AnalogButton:
            {
                send(database::Config::block_t::ANALOG, signal.componentIndex);
            }
            break;

            case messaging::MidiSource::Button:
            {
                send(database::Config::block_t::BUTTONS, signal.componentIndex);
            }
            break;

            case messaging::MidiSource::Encoder:
            {
                send(database::Config::block_t::ENCODERS, signal.componentIndex);
            }
            break;

            case messaging::MidiSource::TouchscreenButton:
            {
                send(database::Config::block_t::TOUCHSCREEN, signal.componentIndex);
            }
            break;

            default:
                break;
            }
        });
}

void ComponentInfo::registerHandler(cinfoHandler_t&& handler)
{
    _handler = std::move(handler);
}

void ComponentInfo::send(database::Config::block_t block, size_t index)
{
    if ((k_uptime_get_32() - _lastCinfoMsgTime[static_cast<size_t>(block)]) > COMPONENT_INFO_TIMEOUT)
    {
        if (_handler != nullptr)
        {
            _handler(static_cast<size_t>(block), index);
        }

        _lastCinfoMsgTime[static_cast<size_t>(block)] = k_uptime_get_32();
    }
}
