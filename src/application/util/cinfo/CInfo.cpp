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

#include "CInfo.h"

using namespace Util;

ComponentInfo::ComponentInfo(Util::MessageDispatcher& dispatcher)
{
    dispatcher.listen(Util::MessageDispatcher::messageSource_t::analog,
                      Util::MessageDispatcher::listenType_t::all,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          send(Database::block_t::analog, dispatchMessage.componentIndex);
                      });

    dispatcher.listen(Util::MessageDispatcher::messageSource_t::buttons,
                      Util::MessageDispatcher::listenType_t::all,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          send(Database::block_t::buttons, dispatchMessage.componentIndex);
                      });

    dispatcher.listen(Util::MessageDispatcher::messageSource_t::encoders,
                      Util::MessageDispatcher::listenType_t::all,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          send(Database::block_t::encoders, dispatchMessage.componentIndex);
                      });

    dispatcher.listen(Util::MessageDispatcher::messageSource_t::touchscreenButton,
                      Util::MessageDispatcher::listenType_t::all,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          send(Database::block_t::touchscreen, dispatchMessage.componentIndex);
                      });

    dispatcher.listen(Util::MessageDispatcher::messageSource_t::touchscreenAnalog,
                      Util::MessageDispatcher::listenType_t::all,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          send(Database::block_t::touchscreen, dispatchMessage.componentIndex);
                      });
}

void ComponentInfo::registerHandler(cinfoHandler_t handler)
{
    _handler = std::move(handler);
}

void ComponentInfo::send(Database::block_t block, size_t index)
{
    if ((core::timing::currentRunTimeMs() - _lastCinfoMsgTime[static_cast<size_t>(block)]) > COMPONENT_INFO_TIMEOUT)
    {
        if (_handler != nullptr)
            _handler(static_cast<size_t>(block), index);

        _lastCinfoMsgTime[static_cast<size_t>(block)] = core::timing::currentRunTimeMs();
    }
}