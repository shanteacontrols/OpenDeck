/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "protocol/mdns/common.h"
#include "protocol/midi/common.h"
#include "system/common.h"
#include "zlibs/utils/signaling/signaling.h"

#include <algorithm>
#include <array>
#include <functional>
#include <span>
#include <string_view>
#include <vector>

namespace opendeck::signaling
{
    /**
     * @brief Identifies system-level events exchanged over the signaling bus.
     */
    enum class SystemEvent : uint8_t
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
     * @brief Identifies signal direction from OpenDeck's perspective.
     *
     * `Out` means OpenDeck emitted state toward protocols, transports, or observers.
     * `In` means an external protocol or transport is driving state into OpenDeck.
     */
    enum class SignalDirection : uint8_t
    {
        In,
        Out,
    };

    /**
     * @brief Identifies the transport that carried traffic.
     */
    enum class TrafficTransport : uint8_t
    {
        Usb,
        Din,
        Ble,
        Network,
    };

    /**
     * @brief Identifies the endpoint that carries SysEx configuration data.
     */
    enum class ConfigTransport : uint8_t
    {
        Usb,
        WebConfig,
    };

    /**
     * @brief Identifies which IO component originated an event.
     */
    enum class IoEventSource : uint8_t
    {
        Button,
        Analog,
        AnalogButton,
        Encoder,
        TouchscreenButton,
        Led,
    };

    /**
     * @brief Describes one protocol-neutral IO event.
     */
    struct OscIoSignal
    {
        IoEventSource          source          = IoEventSource::Button;
        size_t                 component_index = 0;
        std::optional<int32_t> int32_value     = {};
        std::optional<float>   float_value     = {};
        SignalDirection        direction       = SignalDirection::Out;
    };

    /**
     * @brief Describes one IO event interpreted through MIDI configuration.
     */
    struct MidiIoSignal
    {
        IoEventSource               source          = IoEventSource::Button;
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
        SystemEvent system_event = {};
        uint16_t    value        = 0;
    };

    /**
     * @brief Reports that the active touchscreen screen changed.
     */
    struct TouchscreenScreenChangedSignal
    {
        size_t screen_index = 0;
    };

    /**
     * @brief Carries one raw UMP packet together with its traffic direction.
     */
    struct UmpSignal
    {
        SignalDirection direction = SignalDirection::Out;

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
     * @brief Carries one raw SysEx configuration frame.
     */
    struct ConfigRequestSignal
    {
        ConfigTransport          transport = ConfigTransport::WebConfig;
        std::span<const uint8_t> data      = {};
    };

    /**
     * @brief Carries one SysEx configuration response packet.
     */
    struct ConfigResponseSignal
    {
        ConfigTransport transport = ConfigTransport::WebConfig;
        ::midi_ump      packet    = {};
    };

    /**
     * @brief Carries one raw OSC packet observed by the network protocol.
     */
    struct OscSignal
    {
        SignalDirection          direction = SignalDirection::Out;
        std::span<const uint8_t> packet    = {};
    };

    /**
     * @brief Network name and address currently advertised by OpenDeck.
     */
    class NetworkIdentitySignal
    {
        public:
        static constexpr size_t NAME_SIZE         = opendeck::protocol::mdns::NETWORK_NAME_SIZE;
        static constexpr size_t IPV4_ADDRESS_SIZE = opendeck::protocol::mdns::IPV4_ADDRESS_SIZE;

        NetworkIdentitySignal() = default;

        NetworkIdentitySignal(std::string_view network_name, std::string_view address)
        {
            copy_to_buffer(network_name, _name);
            copy_to_buffer(address, _ipv4_address);
        }

        /**
         * @brief Returns the advertised mDNS name.
         *
         * @return Network name text.
         */
        std::string_view name() const
        {
            return std::string_view(_name.data());
        }

        /**
         * @brief Returns the advertised IPv4 address.
         *
         * @return IPv4 address text.
         */
        std::string_view ipv4_address() const
        {
            return std::string_view(_ipv4_address.data());
        }

        private:
        std::array<char, NAME_SIZE>         _name         = {};
        std::array<char, IPV4_ADDRESS_SIZE> _ipv4_address = {};

        template<size_t Size>
        static void copy_to_buffer(std::string_view value, std::array<char, Size>& buffer)
        {
            buffer.fill('\0');

            if (value.size() >= buffer.size())
            {
                return;
            }

            std::copy(value.begin(), value.end(), buffer.begin());
        }
    };

    /**
     * @brief Carries one outgoing USB UMP batch event.
     */
    struct UsbUmpBurstSignal
    {
        ::midi_ump packet = {};
    };

    /**
     * @brief Reports transport traffic activity for indicators.
     */
    struct TrafficSignal
    {
        TrafficTransport transport = TrafficTransport::Usb;
        SignalDirection  direction = SignalDirection::Out;
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
         * @brief Queues one signal instance through the owned signaling backend.
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
        SignalRegistry<OscIoSignal>::instance().clear();
        SignalRegistry<MidiIoSignal>::instance().clear();
        SignalRegistry<InternalProgram>::instance().clear();
        SignalRegistry<SystemSignal>::instance().clear();
        SignalRegistry<TouchscreenScreenChangedSignal>::instance().clear();
        SignalRegistry<UmpSignal>::instance().clear();
        SignalRegistry<ConfigRequestSignal>::instance().clear();
        SignalRegistry<ConfigResponseSignal>::instance().clear();
        SignalRegistry<OscSignal>::instance().clear();
        SignalRegistry<NetworkIdentitySignal>::instance().clear();
        SignalRegistry<UsbUmpBurstSignal>::instance().clear();
        SignalRegistry<TrafficSignal>::instance().clear();
    }
}    // namespace opendeck::signaling
