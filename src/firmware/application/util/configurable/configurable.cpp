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

#include "configurable.h"

using namespace util;

void Configurable::registerConfig(sys::Config::block_t block, getHandler_t&& getHandler, setHandler_t&& setHandler)
{
    _getters.push_back({ block, std::move(getHandler) });
    _setters.push_back({ block, std::move(setHandler) });
}

uint8_t Configurable::get(sys::Config::block_t block, uint8_t section, size_t index, uint16_t& value)
{
    for (size_t i = 0; i < _getters.size(); i++)
    {
        if (_getters.at(i).handler != nullptr)
        {
            if (_getters.at(i).block == block)
            {
                auto result = _getters.at(i).handler(section, index, value);

                if (result != std::nullopt)
                {
                    return *result;
                }
            }
        }
    }

    return sys::Config::Status::ERROR_NOT_SUPPORTED;
}

uint8_t Configurable::set(sys::Config::block_t block, uint8_t section, size_t index, uint16_t value)
{
    for (size_t i = 0; i < _setters.size(); i++)
    {
        if (_setters.at(i).handler != nullptr)
        {
            if (_setters.at(i).block == block)
            {
                auto result = _setters.at(i).handler(section, index, value);

                if (result != std::nullopt)
                {
                    return *result;
                }
            }
        }
    }

    return sys::Config::Status::ERROR_NOT_SUPPORTED;
}

void Configurable::clear()
{
    _getters.clear();
    _setters.clear();
}