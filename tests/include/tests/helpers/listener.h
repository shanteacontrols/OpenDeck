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

#include "application/messaging/messaging.h"

namespace test
{
    class Listener
    {
        public:
        Listener() = default;

        void messageListener(const messaging::Event& event)
        {
            _event.push_back(event);
        }

        std::vector<messaging::Event> _event = {};
    };
}    // namespace test
