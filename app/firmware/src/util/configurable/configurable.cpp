/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "configurable.h"

using namespace util;

void Configurable::register_config(sys::Config::Block block, GetHandler&& get_handler, SetHandler&& set_handler)
{
    _getters.push_back({ block, std::move(get_handler) });
    _setters.push_back({ block, std::move(set_handler) });
}

uint8_t Configurable::get(sys::Config::Block block, uint8_t section, size_t index, uint16_t& value)
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

    return sys::Config::Status::ErrorNotSupported;
}

uint8_t Configurable::set(sys::Config::Block block, uint8_t section, size_t index, uint16_t value)
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

    return sys::Config::Status::ErrorNotSupported;
}

void Configurable::clear()
{
    _getters.clear();
    _setters.clear();
}
