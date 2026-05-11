/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"

#include "protocol/osc/packet/packet.h"
#include "protocol/osc/paths.h"

#include <array>
#include <cstdint>
#include <span>
#include <string_view>

using namespace opendeck::protocol::osc;

namespace
{
    std::span<const uint8_t> packet_span(const PacketBuffer& packet, size_t size)
    {
        return std::span<const uint8_t>(packet.data(), size);
    }
}    // namespace

TEST(OscPacketTest, BuildsAndReadsIntPacket)
{
    PacketBuffer packet = {};

    const auto size = make_packet(packet, "/opendeck/input/analog/0", OscInt32{ 1234 });

    ASSERT_TRUE(size);
    const auto message = parse_message(packet_span(packet, *size));
    ASSERT_TRUE(message);

    const auto value = message->arg<OscInt32>(0);
    ASSERT_TRUE(value);

    EXPECT_EQ(message->address(), "/opendeck/input/analog/0");
    EXPECT_EQ(message->type_tags(), ",i");
    EXPECT_EQ(*value, 1234);
}

TEST(OscPacketTest, BuildsIndexedAddressPacket)
{
    PacketBuffer packet = {};

    const auto size = make_packet(packet,
                                  OscIndexedAddress{
                                      .prefix = paths::INPUT_ANALOG.c_str(),
                                      .index  = 42,
                                  },
                                  OscInt32{ 1234 });

    ASSERT_TRUE(size);
    const auto message = parse_message(packet_span(packet, *size));
    ASSERT_TRUE(message);

    const auto value = message->arg<OscInt32>(0);
    ASSERT_TRUE(value);

    EXPECT_EQ(message->address(), "/opendeck/input/analog/42");
    EXPECT_EQ(*value, 1234);
}

TEST(OscPacketTest, PreservesNegativeIntValue)
{
    PacketBuffer packet = {};

    const auto size = make_packet(packet, "/opendeck/input/encoder/1", OscInt32{ -1 });

    ASSERT_TRUE(size);
    const auto message = parse_message(packet_span(packet, *size));
    ASSERT_TRUE(message);

    const auto value = message->arg<OscInt32>(0);
    ASSERT_TRUE(value);

    EXPECT_EQ(message->address(), "/opendeck/input/encoder/1");
    EXPECT_EQ(message->type_tags(), ",i");
    EXPECT_EQ(*value, -1);
}

TEST(OscPacketTest, BuildsAndReadsFloatPacket)
{
    PacketBuffer packet = {};

    const auto size = make_packet(packet, "/opendeck/test/float", OscFloat32{ 0.5F });

    ASSERT_TRUE(size);
    const auto message = parse_message(packet_span(packet, *size));
    ASSERT_TRUE(message);

    const auto value = message->arg<OscFloat32>(0);
    ASSERT_TRUE(value);

    EXPECT_EQ(message->address(), "/opendeck/test/float");
    EXPECT_EQ(message->type_tags(), ",f");
    EXPECT_FLOAT_EQ(*value, 0.5F);
}

TEST(OscPacketTest, RejectsMalformedIntPacket)
{
    constexpr std::array<uint8_t, 4> packet = {
        '/',
        'x',
        'x',
        'x',
    };

    EXPECT_FALSE(parse_message(packet));
}

TEST(OscPacketTest, ParsesDiscoveryWithoutArguments)
{
    PacketBuffer packet = {};

    const auto size = make_packet(packet, paths::DISCOVERY.c_str());

    ASSERT_TRUE(size);
    const auto message = parse_message(packet_span(packet, *size));
    ASSERT_TRUE(message);

    EXPECT_EQ(message->address(), paths::DISCOVERY.c_str());
    EXPECT_TRUE(message->empty());
}

TEST(OscPacketTest, BuildsDiscoveryResponse)
{
    PacketBuffer packet = {};

    const auto size = make_packet(packet,
                                  paths::DEVICE_INFO.c_str(),
                                  OscString{ "opendeck" },
                                  OscString{ OPENDECK_TARGET },
                                  OscString{ "8.0.1" },
                                  OscInt32{ 9001 },
                                  OscString{ "opendeck-test.local" },
                                  OscString{ "192.168.1.112" });

    ASSERT_TRUE(size);

    const auto message = parse_message(packet_span(packet, *size));
    ASSERT_TRUE(message);

    const auto product = message->arg<OscString>(0);
    const auto target  = message->arg<OscString>(1);
    const auto version = message->arg<OscString>(2);
    const auto port    = message->arg<OscInt32>(3);
    const auto name    = message->arg<OscString>(4);
    const auto ip      = message->arg<OscString>(5);

    ASSERT_TRUE(product);
    ASSERT_TRUE(target);
    ASSERT_TRUE(version);
    ASSERT_TRUE(port);
    ASSERT_TRUE(name);
    ASSERT_TRUE(ip);

    EXPECT_EQ(message->address(), paths::DEVICE_INFO.c_str());
    EXPECT_EQ(message->type_tags(), ",sssiss");
    EXPECT_EQ(*product, "opendeck");
    EXPECT_EQ(*target, OPENDECK_TARGET);
    EXPECT_EQ(*version, "8.0.1");
    EXPECT_EQ(*port, 9001);
    EXPECT_EQ(*name, "opendeck-test.local");
    EXPECT_EQ(*ip, "192.168.1.112");
}

TEST(OscPacketTest, RejectsPacketWhichDoesNotFitBuffer)
{
    PacketBuffer packet = {};

    const std::string_view too_long_address(
        "/opendeck/input/analog/000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");

    EXPECT_FALSE(make_packet(packet, too_long_address, OscInt32{ 1 }));
}

TEST(OscPacketTest, RejectsTruncatedArgument)
{
    PacketBuffer packet = {};

    const auto size = make_packet(packet, "/opendeck/input/analog/0", OscInt32{ 1234 });
    ASSERT_TRUE(size);
    ASSERT_GT(*size, sizeof(uint32_t));

    EXPECT_FALSE(parse_message(packet_span(packet, *size - 1U)));
}

TEST(OscPacketTest, RejectsUnterminatedString)
{
    constexpr std::array<uint8_t, 8> packet = {
        '/',
        'o',
        'p',
        'e',
        'n',
        'd',
        'e',
        'c',
    };

    EXPECT_FALSE(parse_message(packet));
}

TEST(OscPacketTest, RejectsUnsupportedArgumentCount)
{
    PacketBuffer packet = {};

    const auto size = make_packet(packet,
                                  "/opendeck/test/too-many",
                                  OscInt32{ 0 },
                                  OscInt32{ 1 },
                                  OscInt32{ 2 },
                                  OscInt32{ 3 },
                                  OscInt32{ 4 },
                                  OscInt32{ 5 },
                                  OscInt32{ 6 },
                                  OscInt32{ 7 },
                                  OscInt32{ 8 });

    ASSERT_TRUE(size);
    EXPECT_FALSE(parse_message(packet_span(packet, *size)));
}

TEST(OscPacketTest, RejectsExtraTrailingBytes)
{
    PacketBuffer packet = {};

    const auto size = make_packet(packet, "/opendeck/input/analog/0", OscInt32{ 1234 });
    ASSERT_TRUE(size);
    ASSERT_LT(*size, packet.size());

    packet[*size] = 0xff;

    EXPECT_FALSE(parse_message(packet_span(packet, *size + 1U)));
}

TEST(OscAddressTest, ParsesIndexedAddress)
{
    const auto index = paths::parse_indexed("/opendeck/output/led/42", paths::OUTPUT_LED.c_str());

    ASSERT_TRUE(index);
    EXPECT_EQ(*index, 42);
}

TEST(OscAddressTest, RejectsInvalidIndexedAddress)
{
    EXPECT_FALSE(paths::parse_indexed("/opendeck/output/led", paths::OUTPUT_LED.c_str()));
    EXPECT_FALSE(paths::parse_indexed("/opendeck/output/led/foo", paths::OUTPUT_LED.c_str()));
    EXPECT_FALSE(paths::parse_indexed("/opendeck/input/button/1", paths::OUTPUT_LED.c_str()));
}
