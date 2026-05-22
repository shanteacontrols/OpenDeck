/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "zlibs/utils/signaling/signaling.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>
#include <utility>
#include <vector>

namespace opendeck::bootloader::signaling
{
    /**
     * @brief Bootloader status text signal.
     */
    struct StatusSignal
    {
        static constexpr size_t MAX_SIZE = 128;

        StatusSignal() = default;

        /**
         * @brief Constructs the status signal from text.
         *
         * @param message Status text.
         */
        explicit StatusSignal(std::string_view message)
        {
            const size_t size = std::min(message.size(), _data.size());

            std::copy_n(message.begin(), size, _data.begin());
            _size = size;
        }

        /**
         * @brief Returns the status text.
         *
         * @return Status text.
         */
        std::string_view message() const
        {
            return std::string_view(_data.data(), _size);
        }

        private:
        std::array<char, MAX_SIZE> _data = {};
        size_t                     _size = 0;
    };

    /**
     * @brief Signal emitted when a firmware update starts writing payload data.
     */
    struct FirmwareUpdateStartedSignal
    {
    };

    /**
     * @brief Owns signal-bus subscriptions for one bootloader signal type.
     *
     * @tparam Signal Signal payload type managed by this registry.
     */
    template<typename Signal>
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
            _subscriptions.push_back(
                zlibs::utils::signaling::Channel<Signal>::subscribe(
                    typename zlibs::utils::signaling::Channel<Signal>::Callback{ std::forward<Callback>(callback) },
                    replay));
        }

        /**
         * @brief Publishes one signal synchronously.
         *
         * @param signal Signal payload to publish.
         *
         * @return Always `true`.
         */
        bool publish(const Signal& signal)
        {
            zlibs::utils::signaling::Channel<Signal>::publish(signal);
            return true;
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

    /**
     * @brief Subscribes one callback to a bootloader signal type.
     *
     * @tparam Signal Signal payload type to subscribe to.
     * @tparam Callback Callable type that accepts a `const Signal&`.
     *
     * @param callback Callback to subscribe.
     * @param replay Whether cached signal replay should be requested for this subscription.
     */
    template<typename Signal, typename Callback>
    void subscribe(Callback&& callback, bool replay = false)
    {
        SignalRegistry<Signal>::instance().subscribe(std::forward<Callback>(callback), replay);
    }

    /**
     * @brief Publishes one bootloader signal synchronously.
     *
     * @tparam Signal Signal payload type to publish.
     *
     * @param signal Signal payload to publish.
     *
     * @return Always `true`.
     */
    template<typename Signal>
    bool publish(const Signal& signal)
    {
        return SignalRegistry<Signal>::instance().publish(signal);
    }

    /**
     * @brief Clears the standard bootloader signal registries.
     */
    inline void clear_registry()
    {
        SignalRegistry<StatusSignal>::instance().clear();
        SignalRegistry<FirmwareUpdateStartedSignal>::instance().clear();
    }
}    // namespace opendeck::bootloader::signaling
