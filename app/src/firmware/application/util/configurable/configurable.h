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

#include "application/system/config.h"

#include <functional>
#include <optional>

namespace util
{
    class Configurable
    {
        public:
        using getHandler_t = std::function<std::optional<uint8_t>(uint8_t section, size_t index, uint16_t& value)>;
        using setHandler_t = std::function<std::optional<uint8_t>(uint8_t section, size_t index, uint16_t value)>;

        static Configurable& instance()
        {
            static Configurable instance;
            return instance;
        }

        void    registerConfig(sys::Config::block_t block, getHandler_t&& getHandler, setHandler_t&& setHandler);
        uint8_t get(sys::Config::block_t block, uint8_t section, size_t index, uint16_t& value);
        uint8_t set(sys::Config::block_t block, uint8_t section, size_t index, uint16_t value);
        void    clear();

        private:
        Configurable() = default;

        struct GetHandlerInternal
        {
            sys::Config::block_t block   = sys::Config::block_t::GLOBAL;
            getHandler_t         handler = nullptr;
        };

        struct SetHandlerInternal
        {
            sys::Config::block_t block   = sys::Config::block_t::GLOBAL;
            setHandler_t         handler = nullptr;
        };

        std::vector<GetHandlerInternal> _getters = {};
        std::vector<SetHandlerInternal> _setters = {};
    };
}    // namespace util

#define ConfigHandler util::Configurable::instance()