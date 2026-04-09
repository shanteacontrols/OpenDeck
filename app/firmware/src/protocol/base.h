/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <atomic>
#include <cinttypes>

#include <zephyr/kernel.h>

namespace protocol
{
    /**
     * @brief Identifies a supported top-level communication protocol.
     */
    enum class Protocol : uint8_t
    {
        Midi,
        Count
    };

    /**
     * @brief Common base for protocol backends.
     */
    class Base
    {
        public:
        virtual ~Base() = default;

        /**
         * @brief Initializes the protocol backend.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        virtual bool init() = 0;

        /**
         * @brief Deinitializes the protocol backend.
         *
         * @return `true` if deinitialization succeeded, otherwise `false`.
         */
        virtual bool deinit() = 0;

        /**
         * @brief Freezes all protocol processing until `resume()` is called.
         */
        static void freeze()
        {
            ensure_runtime_init();
            frozen.store(true, std::memory_order_release);
        }

        /**
         * @brief Resumes protocol processing and wakes any blocked protocol workers.
         */
        static void resume()
        {
            ensure_runtime_init();

            if (!frozen.exchange(false, std::memory_order_acq_rel))
            {
                return;
            }

            for (size_t i = 0; i < static_cast<size_t>(Protocol::Count); i++)
            {
                k_sem_give(&resume_semaphore);
            }
        }

        /**
         * @brief Returns whether protocol processing is currently frozen.
         *
         * @return `true` when protocol processing is frozen, otherwise `false`.
         */
        static bool is_frozen()
        {
            ensure_runtime_init();
            return frozen.load(std::memory_order_acquire);
        }

        /**
         * @brief Blocks the caller until protocol processing is allowed to continue.
         */
        static void wait_until_running()
        {
            ensure_runtime_init();

            while (is_frozen())
            {
                [[maybe_unused]] auto ret = k_sem_take(&resume_semaphore, K_FOREVER);
            }
        }

        private:
        /**
         * @brief Initializes shared runtime primitives the first time they are needed.
         */
        static void ensure_runtime_init()
        {
            [[maybe_unused]] static const bool initialized = []()
            {
                k_sem_init(&resume_semaphore, 0, K_SEM_MAX_LIMIT);
                return true;
            }();
        }

        static inline struct k_sem      resume_semaphore = {};
        static inline std::atomic<bool> frozen           = false;
    };
}    // namespace protocol
