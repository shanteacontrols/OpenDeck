/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"
#include "tests/helpers/database.h"

#include "database/builder.h"
#include "io/analog/deps.h"
#include "protocol/osc/hwa_test.h"
#include "protocol/osc/osc.h"
#include "protocol/osc/packet/packet.h"
#include "protocol/osc/paths.h"
#include "signaling/signaling.h"
#include "util/configurable/configurable.h"
#include "zlibs/utils/misc/mutex.h"

#include <cstdint>
#include <span>
#include <vector>

#include <zephyr/kernel.h>
#include <zephyr/sys/byteorder.h>

using namespace opendeck;
using namespace opendeck::protocol;

namespace
{
    constexpr uint8_t  DEST_OCTET_0 = 192;
    constexpr uint8_t  DEST_OCTET_1 = 168;
    constexpr uint8_t  DEST_OCTET_2 = 1;
    constexpr uint8_t  DEST_OCTET_3 = 126;
    constexpr uint16_t DEST_PORT    = 9000;
    constexpr uint16_t LISTEN_PORT  = 9001;

    sockaddr_in endpoint(uint8_t octet0, uint8_t octet1, uint8_t octet2, uint8_t octet3, uint16_t port)
    {
        const uint32_t ipv4 = (static_cast<uint32_t>(octet0) << 24) |
                              (static_cast<uint32_t>(octet1) << 16) |
                              (static_cast<uint32_t>(octet2) << 8) |
                              octet3;

        sockaddr_in address     = {};
        address.sin_family      = AF_INET;
        address.sin_port        = sys_cpu_to_be16(port);
        address.sin_addr.s_addr = sys_cpu_to_be32(ipv4);

        return address;
    }

    std::span<const uint8_t> packet_span(const osc::PacketBuffer& packet, size_t size)
    {
        return std::span<const uint8_t>(packet.data(), size);
    }

    class RawIoCollector
    {
        public:
        RawIoCollector()
        {
            signaling::subscribe<signaling::OscIoSignal>(
                [this](const signaling::OscIoSignal& signal)
                {
                    const zlibs::utils::misc::LockGuard lock(_mutex);
                    _signals.push_back(signal);
                });
        }

        std::vector<signaling::OscIoSignal> signals() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _signals;
        }

        size_t count() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _signals.size();
        }

        private:
        mutable zlibs::utils::misc::Mutex   _mutex;
        std::vector<signaling::OscIoSignal> _signals = {};
    };

    class OscProtocolTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_database_admin.init(_handlers));
            ASSERT_TRUE(_database_admin.factory_reset());
            configure_osc();
        }

        void TearDown() override
        {
            _osc.deinit();
            k_msleep(10);
            ConfigHandler.clear();
            signaling::clear_registry();
        }

        void configure_osc()
        {
            ASSERT_TRUE(set_osc_setting(osc::Setting::Enable, 1));
            ASSERT_TRUE(set_osc_setting(osc::Setting::DestIpv4Octet0, DEST_OCTET_0));
            ASSERT_TRUE(set_osc_setting(osc::Setting::DestIpv4Octet1, DEST_OCTET_1));
            ASSERT_TRUE(set_osc_setting(osc::Setting::DestIpv4Octet2, DEST_OCTET_2));
            ASSERT_TRUE(set_osc_setting(osc::Setting::DestIpv4Octet3, DEST_OCTET_3));
            ASSERT_TRUE(set_osc_setting(osc::Setting::DestPort, DEST_PORT));
            ASSERT_TRUE(set_osc_setting(osc::Setting::ListenPort, LISTEN_PORT));
            ASSERT_TRUE(set_osc_setting(osc::Setting::RestrictIncomingToDestIp, 0));
        }

        bool set_osc_setting(osc::Setting setting, uint16_t value)
        {
            return _database_admin.update(database::Config::Section::Global::OscSettings, setting, value);
        }

        bool wait_for_sent_packets(size_t count) const
        {
            for (size_t i = 0; i < 200; i++)
            {
                if (_hwa.sent_packet_count() >= count)
                {
                    return true;
                }

                k_msleep(1);
            }

            return false;
        }

        bool wait_for_raw_io(const RawIoCollector& collector, size_t count) const
        {
            for (size_t i = 0; i < 200; i++)
            {
                if (collector.count() >= count)
                {
                    return true;
                }

                k_msleep(1);
            }

            return false;
        }

        void publish_network_identity()
        {
            signaling::publish(signaling::NetworkIdentitySignal("opendeck-test.local", "192.168.1.112"));
        }

        void publish_network_identity_without_ip()
        {
            signaling::publish(signaling::NetworkIdentitySignal("opendeck-test.local", {}));
        }

        tests::NoOpDatabaseHandlers _handlers;
        database::Builder           _database_builder;
        database::Admin&            _database_admin = _database_builder.instance();
        osc::HwaTest                _hwa;
        osc::Database               _database = osc::Database(_database_admin);
        osc::Osc                    _osc      = osc::Osc(_hwa, _database);
    };
}    // namespace

TEST_F(OscProtocolTest, SendsDiscoveryAnnouncementOnInit)
{
    publish_network_identity();

    ASSERT_TRUE(_osc.init());

    ASSERT_TRUE(wait_for_sent_packets(1));
    const auto sent = _hwa.sent_packets();
    ASSERT_GE(sent.size(), 1U);

    const auto message = osc::parse_message(sent.front().data);
    ASSERT_TRUE(message);

    EXPECT_EQ(message->address(), osc::paths::DEVICE_INFO.c_str());
    EXPECT_EQ(message->type_tags(), ",sssiss");
    EXPECT_EQ(*message->arg<osc::OscString>(0), "opendeck");
    EXPECT_EQ(*message->arg<osc::OscString>(1), OPENDECK_TARGET);
    EXPECT_EQ(*message->arg<osc::OscInt32>(3), LISTEN_PORT);
    EXPECT_EQ(*message->arg<osc::OscString>(4), "opendeck-test.local");
    EXPECT_EQ(*message->arg<osc::OscString>(5), "192.168.1.112");

    EXPECT_EQ(sys_be16_to_cpu(sent.front().dest.sin_port), DEST_PORT);
}

TEST_F(OscProtocolTest, SkipsDiscoveryAnnouncementWithoutNetworkIdentity)
{
    ASSERT_TRUE(_osc.init());
    k_msleep(10);

    EXPECT_EQ(_hwa.sent_packet_count(), 0U);
}

TEST_F(OscProtocolTest, SendsDiscoveryAnnouncementWhenNetworkIdentityArrives)
{
    ASSERT_TRUE(_osc.init());
    k_msleep(10);
    ASSERT_EQ(_hwa.sent_packet_count(), 0U);

    publish_network_identity();

    ASSERT_TRUE(wait_for_sent_packets(1));
    const auto sent = _hwa.sent_packets();
    ASSERT_EQ(sent.size(), 1U);

    const auto message = osc::parse_message(sent.front().data);
    ASSERT_TRUE(message);

    EXPECT_EQ(message->address(), osc::paths::DEVICE_INFO.c_str());
    EXPECT_EQ(*message->arg<osc::OscString>(4), "opendeck-test.local");
    EXPECT_EQ(*message->arg<osc::OscString>(5), "192.168.1.112");
}

TEST_F(OscProtocolTest, NetworkIdentityTopicEnablesOscEvenWithoutIpText)
{
    publish_network_identity_without_ip();

    ASSERT_TRUE(_osc.init());
    ASSERT_TRUE(wait_for_sent_packets(1));

    const auto sent = _hwa.sent_packets();
    ASSERT_EQ(sent.size(), 1U);

    const auto message = osc::parse_message(sent.front().data);
    ASSERT_TRUE(message);

    EXPECT_EQ(message->address(), osc::paths::DEVICE_INFO.c_str());
    EXPECT_EQ(*message->arg<osc::OscString>(4), "opendeck-test.local");
    EXPECT_EQ(*message->arg<osc::OscString>(5), "");
}

TEST_F(OscProtocolTest, SendsInputAfterLateNetworkIdentity)
{
    ASSERT_TRUE(_osc.init());
    k_msleep(10);
    ASSERT_EQ(_hwa.sent_packet_count(), 0U);

    publish_network_identity();
    ASSERT_TRUE(wait_for_sent_packets(1));
    _hwa.clear_sent_packets();

    signaling::publish(signaling::OscIoSignal{
        .source          = signaling::IoEventSource::Analog,
        .component_index = 3,
        .int32_value     = static_cast<int32_t>(io::analog::Filter::POSITION_MAX_VALUE / 2U),
        .float_value     = 0.5F,
        .direction       = signaling::SignalDirection::Out,
    });

    ASSERT_TRUE(wait_for_sent_packets(1));
    const auto sent = _hwa.sent_packets();
    ASSERT_EQ(sent.size(), 1U);

    const auto message = osc::parse_message(sent.front().data);
    ASSERT_TRUE(message);

    EXPECT_EQ(message->address(), "/opendeck/input/analog/3");
    EXPECT_EQ(message->type_tags(), ",f");
    EXPECT_FLOAT_EQ(*message->arg<osc::OscFloat32>(0), 0.5F);
}

TEST_F(OscProtocolTest, SendsRawAnalogInputAsNormalizedFloatOscPacket)
{
    publish_network_identity();

    ASSERT_TRUE(_osc.init());
    ASSERT_TRUE(wait_for_sent_packets(1));
    _hwa.clear_sent_packets();

    signaling::publish(signaling::OscIoSignal{
        .source          = signaling::IoEventSource::Analog,
        .component_index = 3,
        .int32_value     = static_cast<int32_t>(io::analog::Filter::POSITION_MAX_VALUE),
        .float_value     = 1.0F,
        .direction       = signaling::SignalDirection::Out,
    });

    ASSERT_TRUE(wait_for_sent_packets(1));
    const auto sent = _hwa.sent_packets();
    ASSERT_EQ(sent.size(), 1U);

    const auto message = osc::parse_message(sent.front().data);
    ASSERT_TRUE(message);

    EXPECT_EQ(message->address(), "/opendeck/input/analog/3");
    EXPECT_EQ(message->type_tags(), ",f");
    EXPECT_FLOAT_EQ(*message->arg<osc::OscFloat32>(0), 1.0F);
}

TEST_F(OscProtocolTest, ReceivesLedOutputCommand)
{
    publish_network_identity();

    RawIoCollector    collector;
    osc::PacketBuffer packet = {};
    const auto        size   = osc::make_packet(packet,
                                                osc::OscIndexedAddress{
                                                    .prefix = osc::paths::OUTPUT_LED.c_str(),
                                                    .index  = 5,
                                                },
                                                osc::OscInt32{ 1 });

    ASSERT_TRUE(size);
    _hwa.push_received(packet_span(packet, *size), endpoint(192, 168, 1, 10, 50000));

    ASSERT_TRUE(_osc.init());
    ASSERT_TRUE(wait_for_raw_io(collector, 1));

    const auto signals = collector.signals();
    ASSERT_EQ(signals.size(), 1U);

    EXPECT_EQ(signals.front().source, signaling::IoEventSource::Led);
    EXPECT_EQ(signals.front().component_index, 5U);
    ASSERT_TRUE(signals.front().int32_value.has_value());
    EXPECT_EQ(*signals.front().int32_value, 1);
    EXPECT_EQ(signals.front().direction, signaling::SignalDirection::In);
}

TEST_F(OscProtocolTest, DiscoveryResponseUsesConfiguredDestinationPort)
{
    publish_network_identity();

    osc::PacketBuffer packet = {};
    const auto        size   = osc::make_packet(packet, osc::paths::DISCOVERY.c_str());
    ASSERT_TRUE(size);

    _hwa.push_received(packet_span(packet, *size), endpoint(192, 168, 1, 77, 54321));

    ASSERT_TRUE(_osc.init());
    ASSERT_TRUE(wait_for_sent_packets(2));

    const auto sent           = _hwa.sent_packets();
    bool       found_response = false;

    for (const auto& sent_packet : sent)
    {
        const auto message = osc::parse_message(sent_packet.data);

        if (!message || (message->address() != osc::paths::DEVICE_INFO.c_str()))
        {
            continue;
        }

        if (sent_packet.dest.sin_addr.s_addr != endpoint(192, 168, 1, 77, 0).sin_addr.s_addr)
        {
            continue;
        }

        found_response = true;
        EXPECT_EQ(sys_be16_to_cpu(sent_packet.dest.sin_port), DEST_PORT);
        EXPECT_EQ(message->type_tags(), ",sssiss");
        EXPECT_EQ(*message->arg<osc::OscInt32>(3), LISTEN_PORT);
        EXPECT_EQ(*message->arg<osc::OscString>(4), "opendeck-test.local");
        EXPECT_EQ(*message->arg<osc::OscString>(5), "192.168.1.112");
    }

    EXPECT_TRUE(found_response);
}

TEST_F(OscProtocolTest, RestrictsIncomingPacketsToConfiguredDestinationIp)
{
    publish_network_identity();

    ASSERT_TRUE(set_osc_setting(osc::Setting::RestrictIncomingToDestIp, 1));

    RawIoCollector    collector;
    osc::PacketBuffer packet = {};
    const auto        size   = osc::make_packet(packet,
                                                osc::OscIndexedAddress{
                                                    .prefix = osc::paths::OUTPUT_LED.c_str(),
                                                    .index  = 5,
                                                },
                                                osc::OscInt32{ 1 });

    ASSERT_TRUE(size);
    _hwa.push_received(packet_span(packet, *size), endpoint(192, 168, 1, 77, 50000));

    ASSERT_TRUE(_osc.init());
    k_msleep(20);

    EXPECT_EQ(collector.count(), 0U);
}
