/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "zlibs/utils/signaling/signaling.h"

#include <utility>
#include <vector>

namespace opendeck::common::signaling
{
    /**
     * @brief Signal backend that queues publishes through the zlibs dispatcher.
     */
    struct QueuedBackend
    {
        /**
         * @brief Subscribes a callback through the zlibs dispatcher.
         *
         * @tparam Signal Signal payload type to subscribe to.
         * @tparam Callback Callable type that accepts a `const Signal&`.
         *
         * @param callback Callback to subscribe.
         * @param replay Whether cached signal replay should be requested for this subscription.
         *
         * @return Subscription handle.
         */
        template<typename Signal, typename Callback>
        static zlibs::utils::signaling::Subscription subscribe(Callback&& callback, bool replay)
        {
            return zlibs::utils::signaling::subscribe<Signal>(std::forward<Callback>(callback), replay);
        }

        /**
         * @brief Queues one signal through the zlibs dispatcher.
         *
         * @tparam Signal Signal payload type to publish.
         *
         * @param signal Signal payload to publish.
         *
         * @return `true` if the signal was queued successfully, otherwise `false`.
         */
        template<typename Signal>
        static bool publish(const Signal& signal)
        {
            return zlibs::utils::signaling::publish(signal);
        }
    };

    /**
     * @brief Signal backend that publishes directly on the zlibs channel.
     */
    struct DirectBackend
    {
        /**
         * @brief Subscribes a callback directly to the zlibs channel.
         *
         * @tparam Signal Signal payload type to subscribe to.
         * @tparam Callback Callable type that accepts a `const Signal&`.
         *
         * @param callback Callback to subscribe.
         * @param replay Whether cached signal replay should be requested for this subscription.
         *
         * @return Subscription handle.
         */
        template<typename Signal, typename Callback>
        static zlibs::utils::signaling::Subscription subscribe(Callback&& callback, bool replay)
        {
            return zlibs::utils::signaling::Channel<Signal>::subscribe(
                typename zlibs::utils::signaling::Channel<Signal>::Callback{ std::forward<Callback>(callback) },
                replay);
        }

        /**
         * @brief Publishes one signal synchronously on the zlibs channel.
         *
         * @tparam Signal Signal payload type to publish.
         *
         * @param signal Signal payload to publish.
         *
         * @return Always `true`.
         */
        template<typename Signal>
        static bool publish(const Signal& signal)
        {
            zlibs::utils::signaling::Channel<Signal>::publish(signal);
            return true;
        }
    };

    /**
     * @brief Owns signal-bus subscriptions for one signal type.
     *
     * @tparam Signal Signal payload type managed by this registry.
     * @tparam Backend Signal backend used for subscribe and publish.
     */
    template<typename Signal, typename Backend>
    class SignalRegistry
    {
        public:
        /**
         * @brief Registers one callback for the signal type handled by this registry.
         *
         * @tparam Callback Callable type that accepts a `const Signal&`.
         *
         * @param callback Callback to subscribe.
         * @param replay Whether cached signal replay should be requested for this subscription.
         */
        template<typename Callback>
        void subscribe(Callback&& callback, bool replay = false)
        {
            _subscriptions.push_back(Backend::template subscribe<Signal>(std::forward<Callback>(callback), replay));
        }

        /**
         * @brief Publishes one signal through the configured backend.
         *
         * @param signal Signal payload to publish.
         *
         * @return Backend publish result.
         */
        bool publish(const Signal& signal)
        {
            return Backend::publish(signal);
        }

        /**
         * @brief Removes every subscription owned by this registry instance.
         */
        void clear()
        {
            for (auto& subscription : _subscriptions)
            {
                subscription.reset();
            }

            _subscriptions.clear();
        }

        /**
         * @brief Returns the singleton registry instance for the signal type.
         *
         * @return Registry singleton for `Signal`.
         */
        static SignalRegistry& instance()
        {
            static SignalRegistry instance;
            return instance;
        }

        private:
        SignalRegistry() = default;

        std::vector<zlibs::utils::signaling::Subscription> _subscriptions = {};
    };
}    // namespace opendeck::common::signaling
