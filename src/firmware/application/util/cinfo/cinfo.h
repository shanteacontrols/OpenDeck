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

#include "application/database/database.h"

#include <functional>

namespace util
{
    class ComponentInfo
    {
        public:
        using cinfoHandler_t = std::function<void(size_t, size_t)>;

        ComponentInfo();

        void registerHandler(cinfoHandler_t&& handler);

        private:
        /// Minimum time difference in milliseconds between sending two component info messages.
        static constexpr uint32_t COMPONENT_INFO_TIMEOUT = 500;

        cinfoHandler_t _handler                                                                   = nullptr;
        uint32_t       _lastCinfoMsgTime[static_cast<uint8_t>(database::Config::block_t::AMOUNT)] = {};

        void send(database::Config::block_t block, size_t index);
    };
}    // namespace util
