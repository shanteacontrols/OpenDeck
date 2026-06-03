/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/signaling/shared/signaling.h"

#include <utility>

namespace opendeck::bootloader::signaling
{
    using SignalingBackend = common::signaling::DirectBackend;

    template<typename Signal>
    using SignalRegistry = common::signaling::SignalRegistry<Signal, SignalingBackend>;

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
        SignalRegistry<common::signaling::DfuStatusSignal>::instance().clear();
        SignalRegistry<common::signaling::FirmwareUpdateStartedSignal>::instance().clear();
    }
}    // namespace opendeck::bootloader::signaling
