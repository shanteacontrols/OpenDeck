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

#include "Messaging.h"

using namespace Util;

void MessageDispatcher::listen(messageSource_t source, listenType_t listenType, messageCallback_t&& callback)
{
    _listener.push_back({ source, listenType, std::move(callback) });
}

void MessageDispatcher::notify(messageSource_t source, message_t const& message, listenType_t listenType)
{
    for (size_t i = 0; i < _listener.size(); i++)
    {
        if (_listener[i].source == source)
        {
            if (_listener[i].callback != nullptr)
            {
                if ((_listener[i].listenType == listenType) || (_listener[i].listenType == listenType_t::all))
                {
                    _listener[i].callback(message);
                }
            }
        }
    }
}