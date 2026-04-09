/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "protocol/midi/common.h"
#include "system/common.h"
#include "zlibs/utils/signaling/signaling.h"

#include <array>
#include <functional>
#include <vector>

namespace messaging
{
    /**
     * @brief Maximum number of UMP packets accumulated before a USB burst is flushed.
     */
    static constexpr size_t USB_UMP_BURST_PACKET_COUNT = 64;

    /**
     * @brief Identifies system-level events exchanged over the signaling bus.
     */
    enum class SystemMessage : uint8_t
    {
        BurstMidiStart,
        BurstMidiStop,
        MidiProgramOffsetChange,
        PresetChangeIncReq,
        PresetChangeDecReq,
        PresetChangeDirectReq,
        PresetChanged,
        BackupStart,
        BackupEnd,
        RestoreStart,
        RestoreEnd,
        FactoryResetStart,
        FactoryResetEnd,
        InitComplete,
        UsbMidiReady,
        MidiBpmChange,
        ConfigurationSessionOpened,
        ConfigurationSessionClosed,
    };

    /**
     * @brief Identifies the direction of MIDI traffic.
     */
    enum class MidiDirection : uint8_t
    {
        In,
        Out,
    };

    /**
     * @brief Identifies the physical MIDI transport.
     */
    enum class MidiTransport : uint8_t
    {
        Usb,
        Din,
        Ble,
    };

    /**
     * @brief Identifies which subsystem originated a MIDI event.
     */
    enum class MidiSource : uint8_t
    {
        Button,
        Analog,
        AnalogButton,
        Encoder,
        TouchscreenButton,
        Program,
    };

    /**
     * @brief Describes one logical MIDI event produced by a firmware component.
     */
    struct MidiSignal
    {
        MidiSource                  source          = MidiSource::Button;
        size_t                      component_index = 0;
        uint8_t                     channel         = 0;
        uint16_t                    index           = 0;
        uint16_t                    value           = 0;
        protocol::midi::MessageType message         = protocol::midi::MessageType::Invalid;
    };

    /**
     * @brief Carries one system event together with an optional numeric payload.
     */
    struct SystemSignal
    {
        SystemMessage system_message = {};
        uint16_t      value          = 0;
    };

    /**
     * @brief Requests a touchscreen screen change.
     */
    struct TouchscreenScreenSignal
    {
        size_t component_index = 0;
    };

    /**
     * @brief Requests a touchscreen LED/icon state update.
     */
    struct TouchscreenLedSignal
    {
        size_t   component_index = 0;
        uint16_t value           = 0;
    };

    /**
     * @brief Carries one raw UMP packet together with its traffic direction.
     */
    struct UmpSignal
    {
        MidiDirection direction = MidiDirection::Out;

        /**
         * @brief Selects which transport backends should receive the UMP packet.
         */
        enum class Route : uint8_t
        {
            All,
            Usb,
            Din,
            Ble,
        } route           = Route::All;
        ::midi_ump packet = {};
    };

    /**
     * @brief Carries one outgoing USB UMP batch event.
     */
    struct UsbUmpBurstSignal
    {
        ::midi_ump packet = {};
    };

    /**
     * @brief Reports MIDI traffic activity for transport indicators.
     */
    struct MidiTrafficSignal
    {
        MidiTransport transport = MidiTransport::Usb;
        MidiDirection direction = MidiDirection::Out;
    };

    /**
     * @brief Describes one internally generated program-like value change.
     */
    struct InternalProgram
    {
        uint8_t  channel = 0;
        uint16_t index   = 0;
        uint16_t value   = 0;
    };

    /**
     * @brief Announces that a staged forced-refresh cycle has started.
     */
    struct ForcedRefreshStart
    {
        sys::ForcedRefreshType type = {};
    };

    /**
     * @brief Announces that a staged forced-refresh cycle has finished.
     */
    struct ForcedRefreshStop
    {
        sys::ForcedRefreshType type = {};
    };

    /**
     * @brief Owns signal-bus subscriptions for one signal type.
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
         * @param replay Whether cached signal replay should be requested for
         *               this subscription.
         */
        template<typename Callback>
        void subscribe(Callback&& callback, bool replay = false)
        {
            _subscriptions.push_back(
                zlibs::utils::signaling::subscribe<Signal>(std::forward<Callback>(callback), replay));
        }

        /**
         * @brief Queues one signal instance through the asynchronous signaling backend.
         *
         * @param signal Signal payload to publish.
         *
         * @return `true` if the signal was queued successfully, otherwise `false`.
         */
        bool publish(const Signal& signal)
        {
            return zlibs::utils::signaling::publish(signal);
        }

        /**
         * @brief Publishes one signal instance synchronously to current subscribers.
         *
         * @param signal Signal payload to publish.
         *
         * @return Always `true`.
         */
        bool publish_immediate(const Signal& signal)
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
     * @brief Subscribes one callback to a signal type.
     *
     * @tparam Signal Signal payload type to subscribe to.
     * @tparam Callback Callable type that accepts a `const Signal&`.
     *
     * @param callback Callback to subscribe.
     * @param replay Whether cached signal replay should be requested for this
     *               subscription.
     */
    template<typename Signal, typename Callback>
    void subscribe(Callback&& callback, bool replay = false)
    {
        SignalRegistry<Signal>::instance().subscribe(std::forward<Callback>(callback), replay);
    }

    /**
     * @brief Publishes one signal instance through the corresponding registry.
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
        return SignalRegistry<Signal>::instance().publish_immediate(signal);
    }

    /**
     * @brief Clears the standard set of registries used by firmware tests.
     *
     * This removes owned subscriptions for the common signal types used across
     * the unit and integration test suite.
     */
    inline void clear_registry()
    {
        SignalRegistry<MidiSignal>::instance().clear();
        SignalRegistry<InternalProgram>::instance().clear();
        SignalRegistry<SystemSignal>::instance().clear();
        SignalRegistry<TouchscreenLedSignal>::instance().clear();
        SignalRegistry<TouchscreenScreenSignal>::instance().clear();
        SignalRegistry<UmpSignal>::instance().clear();
        SignalRegistry<UsbUmpBurstSignal>::instance().clear();
        SignalRegistry<MidiTrafficSignal>::instance().clear();
    }
}    // namespace messaging
