/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "zlibs/utils/misc/mutex.h"
#include "zlibs/utils/misc/ring_buffer.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace opendeck::common::protocols::websockets
{
    /**
     * @brief Runtime interface for fixed WebSockets RX/TX storage.
     */
    class BuffersBase
    {
        public:
        virtual ~BuffersBase() = default;

        /**
         * @brief Returns writable inbound frame storage.
         *
         * @return RX frame buffer.
         */
        virtual std::span<uint8_t> rx_buffer() = 0;

        /**
         * @brief Inserts one outbound frame into the TX queue.
         *
         * @param data Frame bytes to queue.
         *
         * @return `true` when inserted, otherwise `false`.
         */
        virtual bool insert_tx(std::span<const uint8_t> data) = 0;

        /**
         * @brief Removes one outbound frame from the TX queue.
         *
         * @return Queued frame bytes when available, otherwise `std::nullopt`.
         */
        virtual std::optional<std::span<const uint8_t>> remove_tx() = 0;

        /**
         * @brief Clears all queued outbound frames.
         */
        virtual void reset_tx() = 0;
    };

    /**
     * @brief Fixed WebSockets RX/TX storage sized by the concrete endpoint.
     *
     * @tparam RxFrameSize Maximum accepted inbound frame payload size.
     * @tparam TxFrameSize Maximum queued outbound frame payload size.
     * @tparam TxQueueSize Number of outbound frames that can be queued.
     */
    template<size_t RxFrameSize, size_t TxFrameSize, size_t TxQueueSize>
    class Buffers : public BuffersBase
    {
        static_assert(RxFrameSize > 0, "RX frame size must be larger than 0.");
        static_assert(TxFrameSize > 0, "TX frame size must be larger than 0.");
        static_assert(TxQueueSize > 0, "TX queue size must be larger than 0.");

        public:
        std::span<uint8_t> rx_buffer() override
        {
            return _rx_buffer;
        }

        bool insert_tx(std::span<const uint8_t> data) override
        {
            if (data.size() > TxFrameSize)
            {
                return false;
            }

            const zlibs::utils::misc::LockGuard lock(_tx_queue_lock);

            TxEntry frame = {
                .size = data.size(),
            };

            std::copy(data.begin(), data.end(), frame.data.begin());

            return _tx_queue.insert(frame);
        }

        std::optional<std::span<const uint8_t>> remove_tx() override
        {
            const zlibs::utils::misc::LockGuard lock(_tx_queue_lock);
            const auto                          frame = _tx_queue.remove();

            if (!frame)
            {
                return std::nullopt;
            }

            std::copy(frame->data.begin(), frame->data.begin() + frame->size, _tx_buffer.begin());

            return std::span<const uint8_t>(_tx_buffer.data(), frame->size);
        }

        void reset_tx() override
        {
            const zlibs::utils::misc::LockGuard lock(_tx_queue_lock);

            _tx_queue.reset();
        }

        private:
        struct TxEntry
        {
            std::array<uint8_t, TxFrameSize> data = {};
            size_t                           size = 0;
        };

        std::array<uint8_t, RxFrameSize>                            _rx_buffer = {};
        std::array<uint8_t, TxFrameSize>                            _tx_buffer = {};
        zlibs::utils::misc::RingBuffer<TxQueueSize, false, TxEntry> _tx_queue  = {};
        zlibs::utils::misc::Mutex                                   _tx_queue_lock;
    };
}    // namespace opendeck::common::protocols::websockets
