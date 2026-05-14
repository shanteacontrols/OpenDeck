/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_OSC

#include "osc.h"
#include "packet/packet.h"
#include "paths.h"
#include "util/configurable/configurable.h"
#include "util/conversion/conversion.h"

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <optional>

#include <zlibs/utils/misc/version.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <zephyr/sys/byteorder.h>

using namespace opendeck::protocol::osc;
using namespace opendeck;

namespace zmisc = zlibs::utils::misc;

namespace
{
    LOG_MODULE_REGISTER(osc, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    /**
     * @brief Builds the OSC device information packet used by discovery.
     *
     * @param packet Packet buffer to fill.
     * @param listen_port OSC listen port reported by the device.
     * @param network_name mDNS name advertised for this device.
     * @param ip_address Current IPv4 address text.
     *
     * @return Number of encoded bytes, or empty if the packet does not fit.
     */
    std::optional<size_t> make_device_info_packet(PacketBuffer&    packet,
                                                  uint16_t         listen_port,
                                                  std::string_view network_name,
                                                  std::string_view ip_address)
    {
        static constexpr auto VERSION = zmisc::Version<OPENDECK_SW_VERSION_MAJOR,
                                                       OPENDECK_SW_VERSION_MINOR,
                                                       OPENDECK_SW_VERSION_REVISION>::STRING;

        return make_packet(packet,
                           paths::DEVICE_INFO.c_str(),
                           OscString{ "opendeck" },
                           OscString{ OPENDECK_TARGET },
                           OscString{ static_cast<std::string_view>(VERSION) },
                           OscInt32{ static_cast<int32_t>(listen_port) },
                           OscString{ network_name },
                           OscString{ ip_address });
    }

}    // namespace

Osc::Osc(Hwa& hwa, Database& database)
    : _hwa(hwa)
    , _database(database)
    , _read_thread(
          [this]()
          {
              read_loop();
          },
          [this]()
          {
              _shutdown    = true;
              _initialized = false;
              close_listen_socket();
              k_sem_give(&_read_wakeup);
          })
    , _send_thread(
          [this]()
          {
              send_loop();
          },
          [this]()
          {
              TxEvent event  = {};
              event.control  = true;
              event.shutdown = true;

              {
                  const zmisc::LockGuard lock(_queue_lock);
                  _queue.reset();
                  _queue.insert(event);
              }

              k_sem_give(&_send_wakeup);
          })
{
    k_sem_init(&_send_wakeup, 0, 1);
    k_sem_init(&_read_wakeup, 0, 1);

    signaling::subscribe<signaling::OscIoSignal>(
        [this](const signaling::OscIoSignal& event)
        {
            [[maybe_unused]] const auto ret = enqueue_input(event);
        });

    signaling::subscribe<signaling::NetworkIdentitySignal>(
        [this](const signaling::NetworkIdentitySignal& identity)
        {
            _network_identity          = identity;
            _network_identity_received = true;

            const auto network_name = _network_identity.name();
            const auto ip_address   = _network_identity.ipv4_address();

            if (ip_address.empty())
            {
                LOG_WRN("OSC network identity has no address: %.*s",
                        static_cast<int>(network_name.size()),
                        network_name.data());
            }
            else
            {
                LOG_INF("OSC network identity: %.*s %.*s",
                        static_cast<int>(network_name.size()),
                        network_name.data(),
                        static_cast<int>(ip_address.size()),
                        ip_address.data());
            }

            if (_initialized && has_network_identity())
            {
                [[maybe_unused]] const auto ret = send_discovery_announcement();
            }
        });

    ConfigHandler.register_config(
        sys::Config::Block::Global,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sys_config_get(static_cast<sys::Config::Section::Global>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sys_config_set(static_cast<sys::Config::Section::Global>(section), index, value);
        });
}

bool Osc::init()
{
    if (_initialized)
    {
        return true;
    }

    if (!enabled())
    {
        LOG_INF("OSC disabled");
        return true;
    }

    LOG_INF("Init OSC");

    _shutdown    = false;
    _initialized = true;
    k_sem_give(&_read_wakeup);
    _read_thread.run();
    _send_thread.run();

    if (has_network_identity())
    {
        send_discovery_announcement();
    }

    return true;
}

bool Osc::deinit()
{
    _initialized = false;
    close_listen_socket();

    TxEvent event = {};
    event.control = true;

    {
        const zmisc::LockGuard lock(_queue_lock);
        _queue.reset();
        _queue.insert(event);
    }

    k_sem_give(&_send_wakeup);

    return true;
}

bool Osc::enabled()
{
    return _database.read(database::Config::Section::Global::OscSettings, Setting::Enable) != 0;
}

bool Osc::has_network_identity() const
{
    return _network_identity_received.load() &&
           !_network_identity.name().empty() &&
           !_network_identity.ipv4_address().empty();
}

bool Osc::enqueue_input(const signaling::OscIoSignal& event)
{
    if (!_initialized || protocol::Base::is_frozen() || (event.direction != signaling::SignalDirection::Out))
    {
        return true;
    }

    TxEvent queued = {
        .path             = {},
        .component_index  = event.component_index,
        .value            = event.int32_value.value_or(0),
        .float_value      = false,
        .normalized_value = 0.0F,
        .control          = false,
        .shutdown         = false,
    };

    switch (event.source)
    {
    case signaling::IoEventSource::Switch:
    case signaling::IoEventSource::TouchscreenSwitch:
    {
        queued.path  = paths::SWITCH.c_str();
        queued.value = event.int32_value.value_or(0) != 0 ? 1 : 0;
    }
    break;

    case signaling::IoEventSource::Encoder:
    {
        queued.path = paths::ENCODER.c_str();
    }
    break;

    case signaling::IoEventSource::Analog:
    {
        queued.path             = paths::ANALOG.c_str();
        queued.float_value      = event.float_value.has_value();
        queued.normalized_value = event.float_value.value_or(0.0F);
    }
    break;

    default:
        return true;
    }

    bool inserted = false;

    {
        const zmisc::LockGuard lock(_queue_lock);
        inserted = _queue.insert(queued);
    }

    if (!inserted)
    {
        LOG_WRN("OSC event queue full, dropping input %zu", event.component_index);
        return false;
    }

    k_sem_give(&_send_wakeup);

    return true;
}

void Osc::read_loop()
{
    while (true)
    {
        if (_shutdown)
        {
            return;
        }

        if (!_initialized)
        {
            k_sem_take(&_read_wakeup, K_FOREVER);
            continue;
        }

        int listen_sock = _listen_sock.load();

        if (listen_sock < 0)
        {
            listen_sock = open_listen_socket();

            if (listen_sock < 0)
            {
                k_sem_take(&_read_wakeup, K_MSEC(LISTEN_SOCKET_RETRY_DELAY_MS));
                continue;
            }

            _listen_sock = listen_sock;
        }

        const bool received = receive_packet(listen_sock);

        if (!received && _shutdown)
        {
            return;
        }

        if (!received && _initialized)
        {
            close_listen_socket();
        }
    }
}

void Osc::send_loop()
{
    int send_sock = -1;

    while (true)
    {
        k_sem_take(&_send_wakeup, K_FOREVER);

        while (true)
        {
            std::optional<TxEvent> queued = {};

            {
                const zmisc::LockGuard lock(_queue_lock);
                queued = _queue.remove();
            }

            if (!queued.has_value())
            {
                break;
            }

            const auto event = queued.value();

            if (event.control)
            {
                if (send_sock >= 0)
                {
                    _hwa.close(send_sock);
                    send_sock = -1;
                }

                if (event.shutdown)
                {
                    return;
                }

                continue;
            }

            if (!_initialized)
            {
                continue;
            }

            if (!has_network_identity())
            {
                LOG_WRN_ONCE("Dropping outbound OSC packets until mDNS publishes network identity");
                continue;
            }

            if (send_sock < 0)
            {
                send_sock = open_send_socket();
            }

            if ((send_sock >= 0) && !send_event(event, send_sock))
            {
                _hwa.close(send_sock);
                send_sock = -1;
            }
        }
    }
}

bool Osc::send_event(const TxEvent& event, int sock)
{
    if (event.float_value)
    {
        return send_normalized_float(OscIndexedAddress{
                                         .prefix = event.path,
                                         .index  = event.component_index,
                                     },
                                     event.normalized_value,
                                     sock);
    }

    return send_int(OscIndexedAddress{
                        .prefix = event.path,
                        .index  = event.component_index,
                    },
                    event.value,
                    sock);
}

bool Osc::send_int(OscIndexedAddress address, int32_t value, int sock)
{
    PacketBuffer packet = {};
    const auto   size   = make_packet(packet, address, OscInt32{ value });

    if (!size)
    {
        LOG_ERR("Failed to build OSC int packet");
        return false;
    }

    return send_packet(std::span<const uint8_t>(packet.data(), *size), sock);
}

bool Osc::send_normalized_float(OscIndexedAddress address, float value, int sock)
{
    PacketBuffer packet = {};
    const auto   size   = make_packet(packet,
                                      address,
                                      OscFloat32{ value });

    if (!size)
    {
        LOG_ERR("Failed to build OSC float packet");
        return false;
    }

    return send_packet(std::span<const uint8_t>(packet.data(), *size), sock);
}

bool Osc::send_discovery_response(const sockaddr_in& sender, int sock)
{
    if (!has_network_identity())
    {
        return false;
    }

    const uint32_t listen_port = _database.read(database::Config::Section::Global::OscSettings, Setting::ListenPort);
    const uint32_t dest_port   = _database.read(database::Config::Section::Global::OscSettings, Setting::DestPort);

    if ((listen_port == 0) || (listen_port > UDP_PORT_MAX))
    {
        LOG_ERR("Invalid OSC listen port configuration");
        return false;
    }

    if ((dest_port == 0) || (dest_port > UDP_PORT_MAX))
    {
        LOG_ERR("Invalid OSC destination port configuration");
        return false;
    }

    PacketBuffer packet = {};
    const auto   size   = make_device_info_packet(packet,
                                                  static_cast<uint16_t>(listen_port),
                                                  _network_identity.name(),
                                                  _network_identity.ipv4_address());

    if (!size)
    {
        LOG_ERR("Failed to build OSC discovery response");
        return false;
    }

    sockaddr_in dest = sender;
    dest.sin_port    = sys_cpu_to_be16(static_cast<uint16_t>(dest_port));

    return send_packet_to(std::span<const uint8_t>(packet.data(), *size), dest, sock);
}

bool Osc::send_discovery_announcement()
{
    const uint32_t listen_port = _database.read(database::Config::Section::Global::OscSettings, Setting::ListenPort);

    if ((listen_port == 0) || (listen_port > UDP_PORT_MAX))
    {
        LOG_ERR("Invalid OSC listen port configuration");
        return false;
    }

    if (!has_network_identity())
    {
        return false;
    }

    const int sock = open_send_socket();

    if (sock < 0)
    {
        return false;
    }

    PacketBuffer packet = {};
    const auto   size   = make_device_info_packet(packet,
                                                  static_cast<uint16_t>(listen_port),
                                                  _network_identity.name(),
                                                  _network_identity.ipv4_address());

    if (!size)
    {
        LOG_ERR("Failed to build OSC discovery announcement");
        _hwa.close(sock);
        return false;
    }

    const bool sent = send_packet(std::span<const uint8_t>(packet.data(), *size), sock);

    _hwa.close(sock);

    return sent;
}

bool Osc::send_packet(std::span<const uint8_t> packet, int sock)
{
    sockaddr_in dest = {};

    if (!destination(dest))
    {
        return false;
    }

    return send_packet_to(packet, dest, sock);
}

bool Osc::send_packet_to(std::span<const uint8_t> packet, const sockaddr_in& dest, int sock)
{
    const auto                 sent       = _hwa.send(sock,
                                                      packet.data(),
                                                      packet.size(),
                                                      0,
                                                      reinterpret_cast<const sockaddr*>(&dest),
                                                      sizeof(dest));
    [[maybe_unused]] const int send_errno = errno;

    if (sent < 0)
    {
        LOG_ERR("Failed to send OSC packet: %d",
                send_errno);
        return false;
    }

    signaling::publish(signaling::OscSignal{
        .direction = signaling::SignalDirection::Out,
        .packet    = packet,
    });

    signaling::publish(signaling::TrafficSignal{
        .transport = signaling::TrafficTransport::Network,
        .direction = signaling::SignalDirection::Out,
    });

    return true;
}

bool Osc::destination(sockaddr_in& dest)
{
    const uint32_t octet0 = _database.read(database::Config::Section::Global::OscSettings, Setting::DestIpv4Octet0);
    const uint32_t octet1 = _database.read(database::Config::Section::Global::OscSettings, Setting::DestIpv4Octet1);
    const uint32_t octet2 = _database.read(database::Config::Section::Global::OscSettings, Setting::DestIpv4Octet2);
    const uint32_t octet3 = _database.read(database::Config::Section::Global::OscSettings, Setting::DestIpv4Octet3);
    const uint32_t port   = _database.read(database::Config::Section::Global::OscSettings, Setting::DestPort);

    if ((octet0 > IPV4_OCTET_MAX) ||
        (octet1 > IPV4_OCTET_MAX) ||
        (octet2 > IPV4_OCTET_MAX) ||
        (octet3 > IPV4_OCTET_MAX) ||
        (port == 0) ||
        (port > UDP_PORT_MAX))
    {
        LOG_ERR("Invalid OSC destination configuration");
        return false;
    }

    const uint32_t ipv4 = (octet0 << 24) |
                          (octet1 << 16) |
                          (octet2 << 8) |
                          octet3;

    if (ipv4 == 0)
    {
        LOG_WRN("OSC destination is not configured");
        return false;
    }

    dest                 = {};
    dest.sin_family      = AF_INET;
    dest.sin_port        = sys_cpu_to_be16(static_cast<uint16_t>(port));
    dest.sin_addr.s_addr = sys_cpu_to_be32(ipv4);

    return true;
}

int Osc::open_send_socket()
{
    const int sock = _hwa.open_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock < 0)
    {
        LOG_ERR("Failed to create OSC send socket: %d", errno);
        return -1;
    }

    return sock;
}

int Osc::open_listen_socket()
{
    const uint32_t port = _database.read(database::Config::Section::Global::OscSettings, Setting::ListenPort);

    if ((port == 0) || (port > UDP_PORT_MAX))
    {
        LOG_ERR("Invalid OSC listen port configuration");
        return -1;
    }

    sockaddr_in local = {};

    local.sin_family      = AF_INET;
    local.sin_port        = sys_cpu_to_be16(static_cast<uint16_t>(port));
    local.sin_addr.s_addr = 0;

    const int sock = _hwa.open_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock < 0)
    {
        LOG_ERR("Failed to create OSC listen socket: %d", errno);
        return -1;
    }

    if (_hwa.bind(sock, reinterpret_cast<const sockaddr*>(&local), sizeof(local)) < 0)
    {
        LOG_ERR("Failed to bind OSC listen socket on port %d: %d",
                port,
                errno);
        _hwa.close(sock);
        return -1;
    }

    LOG_INF("Listening for OSC UDP on port %d", port);

    return sock;
}

bool Osc::receive_packet(int listen_sock)
{
    PacketBuffer packet     = {};
    sockaddr_in  sender     = {};
    socklen_t    sender_len = sizeof(sender);

    const auto received = _hwa.receive(listen_sock,
                                       packet.data(),
                                       packet.size(),
                                       0,
                                       reinterpret_cast<sockaddr*>(&sender),
                                       &sender_len);

    if (received < 0)
    {
        if (!_initialized || (_listen_sock.load() != listen_sock))
        {
            return false;
        }

        LOG_ERR("Failed to receive OSC packet: %d", errno);
        return false;
    }

    if (!sender_allowed(sender))
    {
        LOG_WRN_ONCE("Ignoring OSC packet from sender outside configured destination IP");
        return true;
    }

    const auto payload = std::span<const uint8_t>(packet.data(), static_cast<size_t>(received));

    signaling::publish(signaling::OscSignal{
        .direction = signaling::SignalDirection::In,
        .packet    = payload,
    });

    const auto message = parse_message(payload);

    if (!message)
    {
        LOG_WRN_ONCE("Ignoring malformed OSC packet");
        return true;
    }

    if (message->address() == paths::DISCOVERY.c_str())
    {
        if (!message->empty())
        {
            LOG_WRN_ONCE("Ignoring malformed OSC discovery packet");
            return true;
        }

        signaling::publish(signaling::TrafficSignal{
            .transport = signaling::TrafficTransport::Network,
            .direction = signaling::SignalDirection::In,
        });

        send_discovery_response(sender, listen_sock);
        return true;
    }

    if (message->address() == paths::REFRESH_REQ.c_str())
    {
        if (!message->empty())
        {
            LOG_WRN_ONCE("Ignoring malformed OSC refresh packet");
            return true;
        }

        signaling::publish(signaling::SystemSignal{
            .system_event = signaling::SystemEvent::OscRefreshReq,
        });

        signaling::publish(signaling::TrafficSignal{
            .transport = signaling::TrafficTransport::Network,
            .direction = signaling::SignalDirection::In,
        });

        return true;
    }

    const auto value = message->arg<OscInt32>(0);

    if (!value)
    {
        LOG_WRN_ONCE("Ignoring malformed OSC packet");
        return true;
    }

    if (handle_message(message->address(), *value))
    {
        signaling::publish(signaling::TrafficSignal{
            .transport = signaling::TrafficTransport::Network,
            .direction = signaling::SignalDirection::In,
        });
    }

    return true;
}

bool Osc::sender_allowed(const sockaddr_in& sender)
{
    const bool restricted = _database.read(database::Config::Section::Global::OscSettings,
                                           Setting::RestrictIncomingToDestIp) != 0;

    if (!restricted)
    {
        return true;
    }

    sockaddr_in dest = {};

    if (!destination(dest))
    {
        return false;
    }

    return sender.sin_addr.s_addr == dest.sin_addr.s_addr;
}

void Osc::close_listen_socket()
{
    const int listen_sock = _listen_sock.exchange(-1);

    if (listen_sock >= 0)
    {
        _hwa.close(listen_sock);
    }
}

bool Osc::handle_message(std::string_view address, int32_t value)
{
    const auto index = paths::parse_indexed(address, paths::OUTPUT.c_str());

    if (index)
    {
        signaling::publish(signaling::OscIoSignal{
            .source          = signaling::IoEventSource::Output,
            .component_index = *index,
            .int32_value     = static_cast<int32_t>(value != 0 ? 1 : 0),
            .direction       = signaling::SignalDirection::In,
        });

        return true;
    }

    LOG_WRN_ONCE("Ignoring unsupported OSC address");

    return false;
}

std::optional<uint8_t> Osc::sys_config_get(sys::Config::Section::Global section, size_t index, uint16_t& value)
{
    if (section != sys::Config::Section::Global::OscSettings)
    {
        return {};
    }

    uint32_t read_value = 0;

    auto result = _database.read(util::Conversion::sys_2_db_section(section), index, read_value)
                      ? sys::Config::Status::Ack
                      : sys::Config::Status::ErrorRead;

    value = read_value;
    return result;
}

std::optional<uint8_t> Osc::sys_config_set(sys::Config::Section::Global section, size_t index, uint16_t value)
{
    if (section != sys::Config::Section::Global::OscSettings)
    {
        return {};
    }

    auto setting = static_cast<Setting>(index);

    switch (setting)
    {
    case Setting::Enable:
    case Setting::RestrictIncomingToDestIp:
    {
        if (value > 1)
        {
            return sys::Config::Status::ErrorNewValue;
        }
    }
    break;

    case Setting::DestIpv4Octet0:
    case Setting::DestIpv4Octet1:
    case Setting::DestIpv4Octet2:
    case Setting::DestIpv4Octet3:
    {
        if (value > IPV4_OCTET_MAX)
        {
            return sys::Config::Status::ErrorNewValue;
        }
    }
    break;

    case Setting::DestPort:
    case Setting::ListenPort:
    {
        if (value == 0)
        {
            return sys::Config::Status::ErrorNewValue;
        }
    }
    break;

    default:
        return sys::Config::Status::ErrorIndex;
    }

    const auto result = _database.update(util::Conversion::sys_2_db_section(section), index, value)
                            ? sys::Config::Status::Ack
                            : sys::Config::Status::ErrorWrite;

    if (result != sys::Config::Status::Ack)
    {
        return result;
    }

    if (setting == Setting::Enable)
    {
        if (value)
        {
            init();
        }
        else
        {
            deinit();
        }
    }
    else if ((setting == Setting::ListenPort) && _initialized)
    {
        close_listen_socket();
    }

    return result;
}

#endif
