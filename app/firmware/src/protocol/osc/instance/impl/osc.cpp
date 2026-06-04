/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/protocol/osc/instance/impl/osc.h"
#include "firmware/src/protocol/osc/packet/packet.h"
#include "firmware/src/protocol/osc/shared/paths.h"
#include "firmware/src/util/configurable/configurable.h"
#include "firmware/src/util/conversion/conversion.h"

#include <zlibs/utils/misc/bit.h>
#include <zlibs/utils/misc/version.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <zephyr/sys/byteorder.h>

#include <array>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <optional>

using namespace opendeck::firmware::protocol::osc;
using namespace opendeck::firmware;

namespace zmisc = zlibs::utils::misc;

namespace
{
    LOG_MODULE_REGISTER(osc, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    constexpr size_t IPV4_OCTET_COUNT = 4;

    struct DestinationSettings
    {
        std::array<Setting, IPV4_OCTET_COUNT> octets = {};
        Setting                               port   = {};
    };

    constexpr std::array<DestinationSettings, DESTINATION_COUNT> DESTINATION_SETTINGS = {
        DestinationSettings{
            .octets = {
                Setting::Dest1Ipv4Octet0,
                Setting::Dest1Ipv4Octet1,
                Setting::Dest1Ipv4Octet2,
                Setting::Dest1Ipv4Octet3,
            },
            .port = Setting::Dest1Port,
        },
        DestinationSettings{
            .octets = {
                Setting::Dest2Ipv4Octet0,
                Setting::Dest2Ipv4Octet1,
                Setting::Dest2Ipv4Octet2,
                Setting::Dest2Ipv4Octet3,
            },
            .port = Setting::Dest2Port,
        },
        DestinationSettings{
            .octets = {
                Setting::Dest3Ipv4Octet0,
                Setting::Dest3Ipv4Octet1,
                Setting::Dest3Ipv4Octet2,
                Setting::Dest3Ipv4Octet3,
            },
            .port = Setting::Dest3Port,
        },
        DestinationSettings{
            .octets = {
                Setting::Dest4Ipv4Octet0,
                Setting::Dest4Ipv4Octet1,
                Setting::Dest4Ipv4Octet2,
                Setting::Dest4Ipv4Octet3,
            },
            .port = Setting::Dest4Port,
        },
    };

    constexpr uint32_t ipv4_octet_shift(size_t octet_index)
    {
        return static_cast<uint32_t>(((IPV4_OCTET_COUNT - 1) - octet_index) * zlibs::utils::misc::BYTE_BIT_COUNT);
    }

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

template<typename Signal>
bool Osc::enqueue(const Signal& event)
{
    if (!_initialized || protocol::Base::is_frozen() || (event.direction != signaling::SignalDirection::Out))
    {
        return true;
    }

    PacketBuffer packet = {};
    const auto   size   = make_packet(packet, event);

    if (!size)
    {
        return true;
    }

    TxEvent queued = {
        .packet   = packet,
        .size     = *size,
        .control  = false,
        .shutdown = false,
    };

    bool inserted = false;

    {
        const zmisc::LockGuard lock(_queue_lock);
        inserted = _queue.insert(queued);
    }

    if (!inserted)
    {
        LOG_WRN("OSC event queue full, dropping event");
        return false;
    }

    k_sem_give(&_send_wakeup);

    return true;
}

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
    _database.register_layout_init_provider(
        database::Config::Section::Global::OscSettings,
        [](size_t index) -> std::optional<uint32_t>
        {
            for (const auto& destination : DESTINATION_SETTINGS)
            {
                if (index == static_cast<size_t>(destination.port))
                {
                    return DEFAULT_DEST_PORT;
                }
            }

            if (index == static_cast<size_t>(Setting::ListenPort))
            {
                return DEFAULT_LISTEN_PORT;
            }

            return {};
        });

    k_sem_init(&_send_wakeup, 0, 1);
    k_sem_init(&_read_wakeup, 0, 1);

    signaling::subscribe<signaling::OscIoSignal>(
        [this](const signaling::OscIoSignal& event)
        {
            [[maybe_unused]] const auto ret = enqueue(event);
        });

    signaling::subscribe<signaling::OscSensorSignal>(
        [this](const signaling::OscSensorSignal& event)
        {
            [[maybe_unused]] const auto ret = enqueue(event);
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

bool Osc::has_network_identity() const
{
    return _network_identity_received.load() &&
           !_network_identity.name().empty() &&
           !_network_identity.ipv4_address().empty();
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
    if (event.size == 0)
    {
        LOG_ERR("Failed to send empty OSC packet");
        return false;
    }

    return send_packet(std::span<const uint8_t>(event.packet.data(), event.size), sock);
}

bool Osc::send_discovery_response(const sockaddr_in& sender, int sock)
{
    if (!has_network_identity())
    {
        return false;
    }

    const uint32_t listen_port = _database.read(database::Config::Section::Global::OscSettings, Setting::ListenPort);

    if ((listen_port == 0) || (listen_port > UDP_PORT_MAX))
    {
        LOG_ERR("Invalid OSC listen port configuration");
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

    bool sent_any = false;

    for (size_t i = 0; i < DESTINATION_COUNT; i++)
    {
        sockaddr_in destination_config = {};
        const auto  status             = destination(i, destination_config);

        if (status == DestinationStatus::Empty)
        {
            continue;
        }

        if (status == DestinationStatus::Invalid)
        {
            return false;
        }

        sockaddr_in dest = sender;
        dest.sin_port    = destination_config.sin_port;

        if (!send_packet_to(std::span<const uint8_t>(packet.data(), *size), dest, sock))
        {
            return false;
        }

        sent_any = true;
    }

    if (!sent_any)
    {
        LOG_WRN_ONCE("OSC destination is not configured");
        return false;
    }

    return true;
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
    bool sent_any = false;

    for (size_t i = 0; i < DESTINATION_COUNT; i++)
    {
        sockaddr_in dest   = {};
        const auto  status = destination(i, dest);

        if (status == DestinationStatus::Empty)
        {
            continue;
        }

        if (status == DestinationStatus::Invalid)
        {
            return false;
        }

        if (!send_packet_to(packet, dest, sock))
        {
            return false;
        }

        sent_any = true;
    }

    if (!sent_any)
    {
        LOG_WRN_ONCE("OSC destination is not configured");
        return false;
    }

    return true;
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

    signaling::publish(signaling::TrafficSignal{
        .transport = signaling::TrafficTransport::Network,
        .direction = signaling::SignalDirection::Out,
    });

    return true;
}

Osc::DestinationStatus Osc::destination(size_t index, sockaddr_in& dest)
{
    if (index >= DESTINATION_SETTINGS.size())
    {
        return DestinationStatus::Invalid;
    }

    const auto& destination_settings = DESTINATION_SETTINGS.at(index);
    const auto  port                 = _database.read(database::Config::Section::Global::OscSettings, destination_settings.port);

    std::array<uint32_t, IPV4_OCTET_COUNT> octets = {};

    for (size_t i = 0; i < octets.size(); i++)
    {
        octets.at(i) = _database.read(database::Config::Section::Global::OscSettings, destination_settings.octets.at(i));
    }

    if ((port == 0) ||
        (port > UDP_PORT_MAX))
    {
        LOG_ERR("Invalid OSC destination configuration");
        return DestinationStatus::Invalid;
    }

    uint32_t ipv4 = 0;

    for (size_t i = 0; i < octets.size(); i++)
    {
        if (octets.at(i) > IPV4_OCTET_MAX)
        {
            LOG_ERR("Invalid OSC destination configuration");
            return DestinationStatus::Invalid;
        }

        ipv4 |= octets.at(i) << ipv4_octet_shift(i);
    }

    if (ipv4 == 0)
    {
        return DestinationStatus::Empty;
    }

    dest                 = {};
    dest.sin_family      = AF_INET;
    dest.sin_port        = sys_cpu_to_be16(static_cast<uint16_t>(port));
    dest.sin_addr.s_addr = sys_cpu_to_be32(ipv4);

    return DestinationStatus::Ready;
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

    for (size_t i = 0; i < DESTINATION_COUNT; i++)
    {
        sockaddr_in dest   = {};
        const auto  status = destination(i, dest);

        if (status == DestinationStatus::Empty)
        {
            continue;
        }

        if (status == DestinationStatus::Invalid)
        {
            return false;
        }

        if (sender.sin_addr.s_addr == dest.sin_addr.s_addr)
        {
            return true;
        }
    }

    return false;
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
            .int32_value     = value,
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
    case Setting::RestrictIncomingToDestIp:
    {
        if (value > 1)
        {
            return sys::Config::Status::ErrorNewValue;
        }
    }
    break;

    case Setting::Dest1Ipv4Octet0:
    case Setting::Dest1Ipv4Octet1:
    case Setting::Dest1Ipv4Octet2:
    case Setting::Dest1Ipv4Octet3:
    case Setting::Dest2Ipv4Octet0:
    case Setting::Dest2Ipv4Octet1:
    case Setting::Dest2Ipv4Octet2:
    case Setting::Dest2Ipv4Octet3:
    case Setting::Dest3Ipv4Octet0:
    case Setting::Dest3Ipv4Octet1:
    case Setting::Dest3Ipv4Octet2:
    case Setting::Dest3Ipv4Octet3:
    case Setting::Dest4Ipv4Octet0:
    case Setting::Dest4Ipv4Octet1:
    case Setting::Dest4Ipv4Octet2:
    case Setting::Dest4Ipv4Octet3:
    {
        if (value > IPV4_OCTET_MAX)
        {
            return sys::Config::Status::ErrorNewValue;
        }
    }
    break;

    case Setting::Dest1Port:
    case Setting::Dest2Port:
    case Setting::Dest3Port:
    case Setting::Dest4Port:
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

    if ((setting == Setting::ListenPort) && _initialized)
    {
        close_listen_socket();
    }

    return result;
}
