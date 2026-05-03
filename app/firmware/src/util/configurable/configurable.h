/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "system/config.h"

#include <functional>
#include <optional>

namespace opendeck::util
{
    class Configurable
    {
        public:
        using GetHandler = std::function<std::optional<uint8_t>(uint8_t section, size_t index, uint16_t& value)>;
        using SetHandler = std::function<std::optional<uint8_t>(uint8_t section, size_t index, uint16_t value)>;

        static Configurable& instance()
        {
            static Configurable instance;
            return instance;
        }

        void    register_config(sys::Config::Block block, GetHandler&& get_handler, SetHandler&& set_handler);
        uint8_t get(sys::Config::Block block, uint8_t section, size_t index, uint16_t& value);
        uint8_t set(sys::Config::Block block, uint8_t section, size_t index, uint16_t value);
        void    clear();

        private:
        Configurable() = default;

        /**
         * @brief Associates a configuration block with its read handler.
         */
        struct GetHandlerInternal
        {
            sys::Config::Block block   = sys::Config::Block::Global;
            GetHandler         handler = nullptr;
        };

        /**
         * @brief Associates a configuration block with its write handler.
         */
        struct SetHandlerInternal
        {
            sys::Config::Block block   = sys::Config::Block::Global;
            SetHandler         handler = nullptr;
        };

        std::vector<GetHandlerInternal> _getters = {};
        std::vector<SetHandlerInternal> _setters = {};
    };
}    // namespace opendeck::util

#define ConfigHandler util::Configurable::instance()
