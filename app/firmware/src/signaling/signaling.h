/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/signaling/shared/signaling.h"

#include "common/src/protocols/mdns/shared/common.h"
#include "firmware/src/protocol/midi/shared/common.h"
#include "firmware/src/system/shared/common.h"

#include "zlibs/utils/signaling/signaling.h"
#include "zlibs/utils/sysex_conf/sysex_conf_common.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <variant>

#include <zephyr/sys/__assert.h>

namespace opendeck::firmware::signaling
{
    using SignalingBackend = common::signaling::QueuedBackend;

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
        OscRefreshReq,
        BootloaderRebootReq,
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
        WebSockets,
    };

    /**
     * @brief Identifies which IO component originated an event.
     */
    enum class IoEventSource : uint8_t
    {
        Switch,
        Analog,
        AnalogSwitch,
        Encoder,
        TouchscreenSwitch,
        Output,
    };

    /**
     * @brief Requests OSC processing for one IO component event.
     *
     * Outbound events are produced by IO modules when a component changes and
     * consumed by the OSC protocol to build and send the configured OSC packet.
     * Inbound events are produced by the OSC protocol after parsing a received
     * OSC packet and consumed by IO modules that can apply the requested state.
     */
    struct OscIoSignal
    {
        IoEventSource          source          = IoEventSource::Switch;
        size_t                 component_index = 0;
        std::optional<int32_t> int32_value     = {};
        std::optional<float>   float_value     = {};
        SignalDirection        direction       = SignalDirection::Out;
    };

    /**
     * @brief Identifies a decoded APDS9960 gesture direction.
     */
    enum class OscSensorApds9960Gesture : uint8_t
    {
        None,
        Up,
        Down,
        Left,
        Right,
    };

    /**
     * @brief Carries one APDS9960 proximity value for OSC.
     */
    struct OscSensorApds9960ProximitySignal
    {
        int32_t value = 0;
    };

    /**
     * @brief Carries one APDS9960 ambient light value for OSC.
     */
    struct OscSensorApds9960AmbientLightSignal
    {
        float value = 0.0F;
    };

    /**
     * @brief Carries one raw VL53L4CX distance value in millimeters for OSC.
     */
    struct OscSensorVl53l4cxDistanceSignal
    {
        int32_t value = 0;
    };

    /**
     * @brief Carries one normalized calibrated VL53L4CX distance value for OSC.
     */
    struct OscSensorVl53l4cxDistanceNormSignal
    {
        float value = 0.0F;
    };

    /**
     * @brief Carries one CAP1188 capacitive touch state for OSC.
     */
    struct OscSensorCap1188TouchSignal
    {
        size_t  index = 0;
        int32_t value = 0;
    };

    /**
     * @brief Carries one APDS9960 RGB value tuple for OSC.
     */
    struct OscSensorApds9960RgbSignal
    {
        float red   = 0.0F;
        float green = 0.0F;
        float blue  = 0.0F;
    };

    /**
     * @brief Carries one decoded APDS9960 gesture value for OSC.
     */
    struct OscSensorApds9960GestureSignal
    {
        OscSensorApds9960Gesture gesture = OscSensorApds9960Gesture::None;
    };

    /**
     * @brief Carries one normalized BNO085 quaternion tuple for OSC.
     */
    struct OscSensorBno085QuaternionSignal
    {
        float real = 0.0F;
        float i    = 0.0F;
        float j    = 0.0F;
        float k    = 0.0F;
    };

    /**
     * @brief Carries one BNO085 Euler angle tuple in degrees for OSC.
     */
    struct OscSensorBno085EulerSignal
    {
        float yaw   = 0.0F;
        float pitch = 0.0F;
        float roll  = 0.0F;
    };

    /**
     * @brief Carries one BNO085 gyroscope tuple for OSC.
     */
    struct OscSensorBno085GyroscopeSignal
    {
        float x = 0.0F;
        float y = 0.0F;
        float z = 0.0F;
    };

    /**
     * @brief Carries one BNO085 linear acceleration tuple for OSC.
     */
    struct OscSensorBno085LinearAccelerationSignal
    {
        float x = 0.0F;
        float y = 0.0F;
        float z = 0.0F;
    };

    /**
     * @brief Carries one BNO085 gravity tuple for OSC.
     */
    struct OscSensorBno085GravitySignal
    {
        float x = 0.0F;
        float y = 0.0F;
        float z = 0.0F;
    };

    /**
     * @brief Carries one VL53L5CX distance row for OSC.
     */
    struct OscSensorVl53l5cxRowSignal
    {
        static constexpr size_t DISTANCE_COUNT = 8;

        size_t                              row         = 0;
        size_t                              width       = DISTANCE_COUNT;
        std::array<int32_t, DISTANCE_COUNT> distance_mm = {};
    };

    /**
     * @brief Carries the nearest VL53L5CX zone for OSC.
     */
    struct OscSensorVl53l5cxNearestSignal
    {
        int32_t distance_mm = 0;
        int32_t zone        = -1;
        int32_t x           = -1;
        int32_t y           = -1;
    };

    /**
     * @brief Carries the centroid of active VL53L5CX zones for OSC.
     */
    struct OscSensorVl53l5cxCentroidSignal
    {
        float   x            = -1.0F;
        float   y            = -1.0F;
        float   distance_mm  = 0.0F;
        int32_t active_zones = 0;
    };

    /**
     * @brief Carries compact VL53L5CX presence information for OSC.
     */
    struct OscSensorVl53l5cxPresenceSignal
    {
        int32_t present      = 0;
        int32_t active_zones = 0;
        int32_t nearest_mm   = 0;
        float   energy       = 0.0F;
    };

    using OscSensorSignalPayload = std::variant<OscSensorApds9960ProximitySignal,
                                                OscSensorApds9960AmbientLightSignal,
                                                OscSensorVl53l4cxDistanceSignal,
                                                OscSensorVl53l4cxDistanceNormSignal,
                                                OscSensorCap1188TouchSignal,
                                                OscSensorApds9960RgbSignal,
                                                OscSensorApds9960GestureSignal,
                                                OscSensorBno085QuaternionSignal,
                                                OscSensorBno085EulerSignal,
                                                OscSensorBno085GyroscopeSignal,
                                                OscSensorBno085LinearAccelerationSignal,
                                                OscSensorBno085GravitySignal,
                                                OscSensorVl53l5cxRowSignal,
                                                OscSensorVl53l5cxNearestSignal,
                                                OscSensorVl53l5cxCentroidSignal,
                                                OscSensorVl53l5cxPresenceSignal>;

    /**
     * @brief Requests OSC processing for one sensor event.
     */
    struct OscSensorSignal
    {
        OscSensorSignalPayload payload   = OscSensorApds9960ProximitySignal{};
        SignalDirection        direction = SignalDirection::Out;
    };

    /**
     * @brief Describes one IO event interpreted through MIDI configuration.
     */
    struct MidiIoSignal
    {
        IoEventSource               source          = IoEventSource::Switch;
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
     * @brief Carries one owned raw SysEx configuration frame from a config transport.
     *
     * `session_id` is a transport-local generation used to route responses and
     * ignore stale disconnects from earlier WebSockets clients.
     */
    struct ConfigRequestSignal
    {
        static constexpr size_t DATA_SIZE = zlibs::utils::sysex_conf::MAX_MESSAGE_SIZE;
        using Data                        = std::array<uint8_t, DATA_SIZE>;

        ConfigTransport transport  = ConfigTransport::WebSockets;
        uint32_t        session_id = 0;

        ConfigRequestSignal() = default;

        ConfigRequestSignal(ConfigTransport transport, std::span<const uint8_t> data, uint32_t session_id = 0)
            : transport(transport)
            , session_id(session_id)
        {
            set_data(data);
        }

        bool set_data(std::span<const uint8_t> data)
        {
            if (data.size() > _data.size())
            {
                _size = 0;
                return false;
            }

            std::copy(data.begin(), data.end(), _data.begin());
            _size = data.size();

            return true;
        }

        std::span<const uint8_t> data() const
        {
            return std::span<const uint8_t>(_data.data(), _size);
        }

        private:
        Data   _data = {};
        size_t _size = 0;
    };

    /**
     * @brief Carries one SysEx configuration response packet for a config session.
     */
    struct ConfigResponseSignal
    {
        ConfigTransport transport  = ConfigTransport::WebSockets;
        ::midi_ump      packet     = {};
        uint32_t        session_id = 0;
    };

    /**
     * @brief Reports that a SysEx configuration transport session disconnected.
     */
    struct ConfigDisconnectSignal
    {
        ConfigTransport transport  = ConfigTransport::WebSockets;
        uint32_t        session_id = 0;
    };

    /**
     * @brief Network name and address currently advertised by OpenDeck.
     */
    class NetworkIdentitySignal
    {
        public:
        static constexpr size_t NAME_SIZE         = opendeck::common::protocols::mdns::NETWORK_NAME_SIZE;
        static constexpr size_t IPV4_ADDRESS_SIZE = opendeck::common::protocols::mdns::IPV4_ADDRESS_SIZE;

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

    template<typename Signal>
    using SignalRegistry = common::signaling::SignalRegistry<Signal, SignalingBackend>;

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
     * @return `true` if the signal was queued successfully, otherwise `false`.
     */
    template<typename Signal>
    bool publish(const Signal& signal)
    {
        return SignalRegistry<Signal>::instance().publish(signal);
    }

    /**
     * @brief Clears the standard set of registries used by firmware tests.
     *
     * This removes owned subscriptions for the common signal types used across
     * the unit and integration test suite.
     */
    inline void clear_registry()
    {
        [[maybe_unused]] const auto cleared = zlibs::utils::signaling::dispatch_sync(
            []()
            {
                SignalRegistry<OscIoSignal>::instance().clear();
                SignalRegistry<OscSensorSignal>::instance().clear();
                SignalRegistry<MidiIoSignal>::instance().clear();
                SignalRegistry<InternalProgram>::instance().clear();
                SignalRegistry<SystemSignal>::instance().clear();
                SignalRegistry<TouchscreenScreenChangedSignal>::instance().clear();
                SignalRegistry<UmpSignal>::instance().clear();
                SignalRegistry<ConfigRequestSignal>::instance().clear();
                SignalRegistry<ConfigResponseSignal>::instance().clear();
                SignalRegistry<ConfigDisconnectSignal>::instance().clear();
                SignalRegistry<NetworkIdentitySignal>::instance().clear();
                SignalRegistry<ForcedRefreshStart>::instance().clear();
                SignalRegistry<ForcedRefreshStop>::instance().clear();
                SignalRegistry<UsbUmpBurstSignal>::instance().clear();
                SignalRegistry<TrafficSignal>::instance().clear();
            });

        __ASSERT(cleared, "Failed to clear signaling registries");
    }
}    // namespace opendeck::firmware::signaling
