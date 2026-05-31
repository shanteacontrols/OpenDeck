/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "zlibs/utils/misc/mutex.h"
#include "zlibs/utils/misc/ring_buffer.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace opendeck::common::protocols::websockets
{
    /**
     * @brief Fixed WebSockets RX/TX storage sized by the concrete endpoint.
     *
     * @tparam RxFrameSize Maximum accepted inbound frame payload size.
     * @tparam TxFrameSize Maximum queued outbound frame payload size.
     * @tparam TxQueueSize Number of outbound frames that can be queued.
     */
    template<size_t RxFrameSize, size_t TxFrameSize, size_t TxQueueSize>
    struct Buffers
    {
        struct TxFrame
        {
            std::array<uint8_t, TxFrameSize> data       = {};
            size_t                           size       = 0;
            int                              socket     = -1;
            uint32_t                         session_id = 0;
        };

        std::array<uint8_t, RxFrameSize> _rx_buffer = {};

        /**
         * @brief Inserts one outbound frame into the TX queue.
         *
         * @param frame Frame to queue.
         *
         * @return `true` when inserted, otherwise `false`.
         */
        bool insert(TxFrame frame)
        {
            const zlibs::utils::misc::LockGuard lock(_tx_queue_lock);

            return _tx_queue.insert(frame);
        }

        /**
         * @brief Removes one outbound frame from the TX queue.
         *
         * @return Queued frame when available, otherwise `std::nullopt`.
         */
        std::optional<TxFrame> remove()
        {
            const zlibs::utils::misc::LockGuard lock(_tx_queue_lock);

            return _tx_queue.remove();
        }

        /**
         * @brief Clears all queued outbound frames.
         */
        void reset()
        {
            const zlibs::utils::misc::LockGuard lock(_tx_queue_lock);

            _tx_queue.reset();
        }

        private:
        zlibs::utils::misc::RingBuffer<TxQueueSize, false, TxFrame> _tx_queue = {};
        zlibs::utils::misc::Mutex                                   _tx_queue_lock;
    };
}    // namespace opendeck::common::protocols::websockets
