/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inttypes.h>
#include <stddef.h>

#include <atomic>

#include <zephyr/kernel.h>

namespace io
{
    /**
     * @brief Identifies a top-level I/O subsystem.
     */
    enum class Io : uint8_t
    {
        Digital,
        Analog,
        Leds,
        I2c,
        Touchscreen,
        Indicators,
        Count
    };

    /**
     * @brief Common base for top-level I/O subsystems.
     */
    class Base
    {
        public:
        virtual ~Base() = default;

        /**
         * @brief Initializes the subsystem.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        virtual bool init() = 0;

        /**
         * @brief Deinitializes the subsystem.
         */
        virtual void deinit() = 0;

        /**
         * @brief Returns the number of staged-refreshable components exposed by the subsystem.
         *
         * @return Number of refreshable components, or `0` when staged refresh is unsupported.
         */
        virtual size_t refreshable_components() const
        {
            return 0;
        }

        /**
         * @brief Forces refresh of a staged subset of subsystem state.
         *
         * @param start_index First component index to refresh.
         * @param count Number of component indices to refresh.
         */
        virtual void force_refresh([[maybe_unused]] size_t start_index,
                                   [[maybe_unused]] size_t count)
        {
        }

        /**
         * @brief Freezes all I/O processing until `resume()` is called.
         */
        static void freeze()
        {
            ensure_runtime_init();
            frozen.store(true, std::memory_order_release);
        }

        /**
         * @brief Resumes I/O processing and wakes any blocked subsystem workers.
         */
        static void resume()
        {
            ensure_runtime_init();

            if (!frozen.exchange(false, std::memory_order_acq_rel))
            {
                return;
            }

            for (size_t i = 0; i < static_cast<size_t>(Io::Count); i++)
            {
                k_sem_give(&resume_semaphore);
            }
        }

        /**
         * @brief Returns whether I/O processing is currently frozen.
         *
         * @return `true` when I/O processing is frozen, otherwise `false`.
         */
        static bool is_frozen()
        {
            ensure_runtime_init();
            return frozen.load(std::memory_order_acquire);
        }

        /**
         * @brief Blocks the caller until I/O processing is allowed to continue.
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
        static inline std::atomic<bool> frozen           = true;
    };
}    // namespace io
