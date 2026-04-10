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

#include <array>
#include <atomic>
#include <cstddef>
#include <optional>

namespace io::digital::drivers
{
    template<typename T>
    class Buffer
    {
        public:
        static constexpr size_t CAPACITY     = 8;
        static constexpr size_t STORAGE_SIZE = CAPACITY + 1;

        void push(const T& value)
        {
            const size_t head = _head.load(std::memory_order_relaxed);
            size_t       next = head + 1;

            if (next >= STORAGE_SIZE)
            {
                next = 0;
            }

            if (next == _tail.load(std::memory_order_acquire))
            {
                size_t tail = _tail.load(std::memory_order_relaxed) + 1;

                if (tail >= STORAGE_SIZE)
                {
                    tail = 0;
                }

                _tail.store(tail, std::memory_order_release);
            }

            _buffer[head] = value;
            _head.store(next, std::memory_order_release);
        }

        std::optional<T> pop()
        {
            const size_t tail = _tail.load(std::memory_order_relaxed);
            const size_t head = _head.load(std::memory_order_acquire);

            if (tail == head)
            {
                return {};
            }

            T      value = _buffer[tail];
            size_t next  = tail + 1;

            if (next >= STORAGE_SIZE)
            {
                next = 0;
            }

            _tail.store(next, std::memory_order_release);

            return value;
        }

        bool empty() const
        {
            return _head.load(std::memory_order_acquire) == _tail.load(std::memory_order_acquire);
        }

        void clear()
        {
            _tail.store(_head.load(std::memory_order_acquire), std::memory_order_release);
        }

        private:
        std::array<T, STORAGE_SIZE> _buffer = {};
        std::atomic<size_t>         _head   = 0;
        std::atomic<size_t>         _tail   = 0;
    };
}    // namespace io::digital::drivers
