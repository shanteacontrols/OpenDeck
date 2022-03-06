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

#include "CInfo.h"
#include "messaging/Messaging.h"

using namespace Util;

ComponentInfo::ComponentInfo()
{
    MIDIDispatcher.listen(Messaging::eventSource_t::analog,
                          Messaging::listenType_t::all,
                          [this](const Messaging::event_t& event) {
                              send(Database::Config::block_t::analog, event.componentIndex);
                          });

    MIDIDispatcher.listen(Messaging::eventSource_t::buttons,
                          Messaging::listenType_t::all,
                          [this](const Messaging::event_t& event) {
                              send(Database::Config::block_t::buttons, event.componentIndex);
                          });

    MIDIDispatcher.listen(Messaging::eventSource_t::encoders,
                          Messaging::listenType_t::all,
                          [this](const Messaging::event_t& event) {
                              send(Database::Config::block_t::encoders, event.componentIndex);
                          });

    MIDIDispatcher.listen(Messaging::eventSource_t::touchscreenButton,
                          Messaging::listenType_t::all,
                          [this](const Messaging::event_t& event) {
                              send(Database::Config::block_t::touchscreen, event.componentIndex);
                          });

    MIDIDispatcher.listen(Messaging::eventSource_t::touchscreenAnalog,
                          Messaging::listenType_t::all,
                          [this](const Messaging::event_t& event) {
                              send(Database::Config::block_t::touchscreen, event.componentIndex);
                          });
}

void ComponentInfo::registerHandler(cinfoHandler_t&& handler)
{
    _handler = std::move(handler);
}

void ComponentInfo::send(Database::Config::block_t block, size_t index)
{
    if ((core::timing::currentRunTimeMs() - _lastCinfoMsgTime[static_cast<size_t>(block)]) > COMPONENT_INFO_TIMEOUT)
    {
        if (_handler != nullptr)
        {
            _handler(static_cast<size_t>(block), index);
        }

        _lastCinfoMsgTime[static_cast<size_t>(block)] = core::timing::currentRunTimeMs();
    }
}