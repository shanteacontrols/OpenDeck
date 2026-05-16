/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/analog/shared/deps.h"

#include "zlibs/utils/misc/mutex.h"

#include <atomic>
#include <deque>

namespace opendeck::io::analog
{
    /**
     * @brief Test analog hardware adapter backed by an in-memory frame queue.
     */
    class HwaTest : public Hwa
    {
        public:
        HwaTest() = default;

        /**
         * @brief Initializes the test backend.
         *
         * @return Always `true`.
         */
        bool init() override
        {
            return true;
        }

        /**
         * @brief Deinitializes the test backend.
         */
        void deinit() override
        {
        }

        /**
         * @brief Returns and removes the next queued frame.
         *
         * @return Queued frame, or `std::nullopt` when the queue is empty.
         */
        std::optional<Frame> read() override
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);

            if (_frames.empty())
            {
                return {};
            }

            auto frame = _frames.front();
            _frames.pop_front();
            _read_count.fetch_add(1, std::memory_order_relaxed);

            return frame;
        }

        /**
         * @brief Stores the most recently requested scan mask.
         *
         * @param mask Physical-channel scan mask.
         */
        void set_scan_mask(const ScanMask& mask) override
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _scan_mask = mask;
        }

        /**
         * @brief Appends one frame to the test input queue.
         *
         * @param frame Frame to queue.
         */
        void push_frame(const Frame& frame)
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _frames.push_back(frame);
        }

        /**
         * @brief Returns how many frames have been read so far.
         *
         * @return Number of successful `read()` operations.
         */
        size_t read_count() const
        {
            return _read_count.load(std::memory_order_relaxed);
        }

        /**
         * @brief Clears the read counter maintained by the test backend.
         */
        void clear_read_count()
        {
            _read_count.store(0, std::memory_order_relaxed);
        }

        /**
         * @brief Returns the most recently requested scan mask.
         *
         * @return Stored physical-channel scan mask.
         */
        ScanMask scan_mask() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _scan_mask;
        }

        private:
        mutable zlibs::utils::misc::Mutex _mutex;
        std::deque<Frame>                 _frames     = {};
        std::atomic<size_t>               _read_count = { 0 };
        ScanMask                          _scan_mask  = {};
    };
}    // namespace opendeck::io::analog
