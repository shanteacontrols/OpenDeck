/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/database/database.h"

#include <functional>

namespace opendeck::util
{
    /**
     * @brief Throttles and forwards component-info notifications.
     */
    class ComponentInfo
    {
        public:
        /**
         * @brief Callback type used to report a configuration block and component index pair.
         */
        using CinfoHandler = std::function<void(size_t, size_t)>;

        /**
         * @brief Constructs a component-info dispatcher.
         */
        ComponentInfo();

        /**
         * @brief Registers the callback used to emit component-info notifications.
         *
         * @param handler Callback invoked with the block and component indices.
         */
        void register_handler(CinfoHandler&& handler);

        private:
        static constexpr uint32_t COMPONENT_INFO_TIMEOUT = 500;

        CinfoHandler _handler                                                                   = nullptr;
        uint32_t     _last_cinfo_msg_time[static_cast<uint8_t>(database::Config::Block::Count)] = {};

        /**
         * @brief Emits a component-info notification when the throttle window allows it.
         *
         * @param block Configuration block associated with the component.
         * @param index Component index within the block.
         */
        void send(database::Config::Block block, size_t index);
    };
}    // namespace opendeck::util
