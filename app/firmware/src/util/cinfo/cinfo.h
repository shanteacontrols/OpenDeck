/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/database/instance/impl/database.h"

#include "zlibs/utils/misc/kwork_delayable.h"
#include "zlibs/utils/misc/mutex.h"

#include <array>
#include <cstddef>
#include <functional>
#include <optional>

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
        static constexpr uint32_t SAME_COMPONENT_INFO_TIMEOUT = 500;

        struct PendingInfo
        {
            bool                    pending = false;
            size_t                  index   = 0;
            std::optional<uint32_t> last_ms = {};
        };

        CinfoHandler                                                                  _handler = nullptr;
        std::array<PendingInfo, static_cast<uint8_t>(database::Config::Block::Count)> _pending = {};
        zlibs::utils::misc::Mutex                                                     _lock;
        zlibs::utils::misc::KworkDelayable                                            _work;

        /**
         * @brief Queues a component-info notification when the throttle window allows it.
         *
         * @param block Configuration block associated with the component.
         * @param index Component index within the block.
         */
        void queue(database::Config::Block block, size_t index);

        /**
         * @brief Sends any pending component-info notifications from worker context.
         */
        void process_pending();
    };
}    // namespace opendeck::util
