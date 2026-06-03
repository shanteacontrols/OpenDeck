/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/osc/packet/internal.h"
#include "firmware/src/protocol/osc/shared/packet.h"
#include "firmware/src/protocol/osc/shared/paths.h"
#include "firmware/src/signaling/signaling.h"

#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <string_view>
#include <type_traits>
#include <variant>

namespace opendeck::firmware::protocol::osc
{
    /**
     * @brief Parsed view of one OSC message.
     */
    class OscMessageView
    {
        public:
        /**
         * @brief Returns the OSC address.
         *
         * @return Parsed OSC address.
         */
        std::string_view address() const;

        /**
         * @brief Returns the OSC type-tag string.
         *
         * @return Parsed OSC type-tag string.
         */
        std::string_view type_tags() const;

        /**
         * @brief Returns true when the message has no arguments.
         *
         * @return `true` when no OSC arguments are present.
         */
        bool empty() const;

        /**
         * @brief Reads one typed OSC argument by index.
         *
         * @tparam Arg OSC argument wrapper type.
         * @param index Argument index.
         *
         * @return Argument value, or empty if the index or type does not match.
         */
        template<typename Arg>
        std::optional<typename Arg::type> arg(size_t index) const
        {
            if constexpr (std::is_same_v<Arg, OscInt32>)
            {
                return read_arg(index, Arg{});
            }

            if constexpr (std::is_same_v<Arg, OscString>)
            {
                return read_arg(index, Arg{});
            }

            if constexpr (std::is_same_v<Arg, OscFloat32>)
            {
                return read_arg(index, Arg{});
            }

            static_assert(std::is_same_v<Arg, OscInt32> ||
                              std::is_same_v<Arg, OscString> ||
                              std::is_same_v<Arg, OscFloat32>,
                          "Unsupported OSC argument type");

            return {};
        }

        private:
        friend std::optional<OscMessageView> parse_message(std::span<const uint8_t> packet);

        /**
         * @brief Creates a message view from parsed packet fields.
         *
         * @param packet Raw packet bytes owned by the caller.
         * @param address Parsed OSC address.
         * @param type_tags Parsed OSC type-tag string.
         * @param arg_offsets Offsets of parsed argument payloads.
         */
        OscMessageView(std::span<const uint8_t> packet,
                       std::string_view         address,
                       std::string_view         type_tags,
                       std::span<const size_t>  arg_offsets);

        /**
         * @brief Checks whether an argument index exists and has the expected type tag.
         *
         * @param index Argument index to check.
         * @param type_tag Expected OSC type tag.
         *
         * @return `true` when the argument exists and the type tag matches.
         */
        bool arg_matches(size_t index, char type_tag) const;

        /**
         * @brief Reads one int32 argument by index.
         *
         * @param index Argument index to read.
         * @param arg Type selector.
         *
         * @return Argument value, or empty when the index or type does not match.
         */
        std::optional<int32_t> read_arg(size_t index, OscInt32 arg) const;

        /**
         * @brief Reads one string argument by index.
         *
         * @param index Argument index to read.
         * @param arg Type selector.
         *
         * @return Argument value, or empty when the index or type does not match.
         */
        std::optional<std::string_view> read_arg(size_t index, OscString arg) const;

        /**
         * @brief Reads one float32 argument by index.
         *
         * @param index Argument index to read.
         * @param arg Type selector.
         *
         * @return Argument value, or empty when the index or type does not match.
         */
        std::optional<float> read_arg(size_t index, OscFloat32 arg) const;

        std::span<const uint8_t>               _packet      = {};
        std::string_view                       _address     = {};
        std::string_view                       _type_tags   = {};
        std::array<size_t, MAX_ARGUMENT_COUNT> _arg_offsets = {};
        size_t                                 _arg_count   = 0;
    };

    /**
     * @brief Builds one OSC packet and derives type tags from argument types.
     *
     * @param packet Packet buffer to fill.
     * @param address OSC address pattern.
     * @param args OSC arguments to append.
     *
     * @return Number of bytes written, or empty if the packet does not fit.
     */
    template<typename... Args>
    std::optional<size_t> make_packet(PacketBuffer& packet, std::string_view address, const Args&... args)
    {
        return internal::make_packet_impl(packet, address, args...);
    }

    /**
     * @brief Builds one OSC packet for an indexed component address.
     *
     * @param packet Packet buffer to fill.
     * @param address OSC address prefix and component index.
     * @param args OSC arguments to append.
     *
     * @return Number of bytes written, or empty if the packet does not fit.
     */
    template<typename... Args>
    std::optional<size_t> make_packet(PacketBuffer& packet, OscIndexedAddress address, const Args&... args)
    {
        return internal::make_packet_impl(packet, address, args...);
    }

    /**
     * @brief Builds one OSC packet from a protocol-neutral IO signal.
     *
     * @param packet Packet buffer to fill.
     * @param signal IO event to encode as an OSC message.
     *
     * @return Number of bytes written, or empty if the IO source is not mapped
     *         to outbound OSC.
     */
    inline std::optional<size_t> make_packet(PacketBuffer&                                     packet,
                                             const opendeck::firmware::signaling::OscIoSignal& signal)
    {
        std::string_view path  = {};
        int32_t          value = signal.int32_value.value_or(0);

        switch (signal.source)
        {
        case opendeck::firmware::signaling::IoEventSource::Switch:
        case opendeck::firmware::signaling::IoEventSource::TouchscreenSwitch:
        {
            path  = paths::SWITCH.c_str();
            value = value != 0 ? 1 : 0;
        }
        break;

        case opendeck::firmware::signaling::IoEventSource::Encoder:
        {
            path = paths::ENCODER.c_str();
        }
        break;

        case opendeck::firmware::signaling::IoEventSource::Analog:
        {
            const auto address = OscIndexedAddress{
                .prefix = paths::ANALOG.c_str(),
                .index  = signal.component_index,
            };

            if (signal.float_value.has_value())
            {
                return make_packet(packet, address, OscFloat32{ signal.float_value.value() });
            }

            return make_packet(packet, address, OscInt32{ value });
        }

        default:
            return {};
        }

        return make_packet(packet,
                           OscIndexedAddress{
                               .prefix = path,
                               .index  = signal.component_index,
                           },
                           OscInt32{ value });
    }

    /**
     * @brief Builds one OSC packet from a sensor signal.
     *
     * @param packet Packet buffer to fill.
     * @param signal Sensor event to encode as an OSC message.
     *
     * @return Number of bytes written, or empty if the sensor event is not mapped.
     */
    inline std::optional<size_t> make_packet(PacketBuffer&                                         packet,
                                             const opendeck::firmware::signaling::OscSensorSignal& signal)
    {
        return std::visit(
            [&packet](const auto& payload) -> std::optional<size_t>
            {
                using Payload = std::decay_t<decltype(payload)>;

                if constexpr (std::is_same_v<Payload, opendeck::firmware::signaling::OscSensorProximitySignal>)
                {
                    return make_packet(packet, paths::SENSOR_PROXIMITY.c_str(), OscInt32{ payload.value });
                }
                else if constexpr (std::is_same_v<Payload, opendeck::firmware::signaling::OscSensorAmbientLightSignal>)
                {
                    return make_packet(packet, paths::SENSOR_AMBIENT_LIGHT.c_str(), OscInt32{ payload.value });
                }
                else if constexpr (std::is_same_v<Payload, opendeck::firmware::signaling::OscSensorDistanceSignal>)
                {
                    return make_packet(packet, paths::SENSOR_DISTANCE.c_str(), OscInt32{ payload.value });
                }
                else if constexpr (std::is_same_v<Payload, opendeck::firmware::signaling::OscSensorTouchSignal>)
                {
                    return make_packet(packet,
                                       OscIndexedAddress{
                                           .prefix = paths::SENSOR_TOUCH.c_str(),
                                           .index  = payload.index,
                                       },
                                       OscInt32{ payload.value != 0 ? 1 : 0 });
                }
                else if constexpr (std::is_same_v<Payload, opendeck::firmware::signaling::OscSensorRgbSignal>)
                {
                    return make_packet(packet,
                                       paths::SENSOR_RGB.c_str(),
                                       OscInt32{ payload.red },
                                       OscInt32{ payload.green },
                                       OscInt32{ payload.blue });
                }
                else if constexpr (std::is_same_v<Payload, opendeck::firmware::signaling::OscSensorGestureSignal>)
                {
                    std::string_view gesture = {};

                    switch (payload.gesture)
                    {
                    case opendeck::firmware::signaling::OscSensorGesture::Up:
                    {
                        gesture = "up";
                    }
                    break;

                    case opendeck::firmware::signaling::OscSensorGesture::Down:
                    {
                        gesture = "down";
                    }
                    break;

                    case opendeck::firmware::signaling::OscSensorGesture::Left:
                    {
                        gesture = "left";
                    }
                    break;

                    case opendeck::firmware::signaling::OscSensorGesture::Right:
                    {
                        gesture = "right";
                    }
                    break;

                    default:
                        return {};
                    }

                    return make_packet(packet, paths::SENSOR_GESTURE.c_str(), OscString{ gesture });
                }
                else if constexpr (std::is_same_v<Payload, opendeck::firmware::signaling::OscSensorImuQuaternionSignal>)
                {
                    return make_packet(packet,
                                       paths::SENSOR_IMU_QUATERNION.c_str(),
                                       OscFloat32{ payload.real },
                                       OscFloat32{ payload.i },
                                       OscFloat32{ payload.j },
                                       OscFloat32{ payload.k });
                }
                else if constexpr (std::is_same_v<Payload, opendeck::firmware::signaling::OscSensorImuEulerSignal>)
                {
                    return make_packet(packet,
                                       paths::SENSOR_IMU_EULER.c_str(),
                                       OscFloat32{ payload.yaw },
                                       OscFloat32{ payload.pitch },
                                       OscFloat32{ payload.roll });
                }
                else if constexpr (std::is_same_v<Payload, opendeck::firmware::signaling::OscSensorImuGyroscopeSignal>)
                {
                    return make_packet(packet,
                                       paths::SENSOR_IMU_GYROSCOPE.c_str(),
                                       OscFloat32{ payload.x },
                                       OscFloat32{ payload.y },
                                       OscFloat32{ payload.z });
                }
                else if constexpr (std::is_same_v<Payload, opendeck::firmware::signaling::OscSensorImuLinearAccelerationSignal>)
                {
                    return make_packet(packet,
                                       paths::SENSOR_IMU_LINEAR_ACCEL.c_str(),
                                       OscFloat32{ payload.x },
                                       OscFloat32{ payload.y },
                                       OscFloat32{ payload.z });
                }
                else if constexpr (std::is_same_v<Payload, opendeck::firmware::signaling::OscSensorImuGravitySignal>)
                {
                    return make_packet(packet,
                                       paths::SENSOR_IMU_GRAVITY.c_str(),
                                       OscFloat32{ payload.x },
                                       OscFloat32{ payload.y },
                                       OscFloat32{ payload.z });
                }

                return {};
            },
            signal.payload);
    }

    /**
     * @brief Parses one OSC message packet.
     *
     * @param packet Raw OSC packet bytes.
     *
     * @return Parsed message view, or empty if the packet is malformed or unsupported.
     */
    std::optional<OscMessageView> parse_message(std::span<const uint8_t> packet);
}    // namespace opendeck::firmware::protocol::osc
