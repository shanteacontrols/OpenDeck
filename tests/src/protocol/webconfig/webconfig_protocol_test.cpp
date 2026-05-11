/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"

#include "protocol/webconfig/hwa_test.h"
#include "protocol/webconfig/webconfig.h"
#include "signaling/signaling.h"
#include "staged_update/hwa_test.h"
#include "staged_update/staged_update.h"
#include "zlibs/utils/midi/midi.h"
#include "zlibs/utils/misc/mutex.h"

#include <array>
#include <span>
#include <vector>

#include <zephyr/kernel.h>

using namespace opendeck;
using namespace opendeck::protocol;

namespace
{
    constexpr int CLIENT_SOCKET = 7;

    class ConfigRequestCollector
    {
        public:
        ConfigRequestCollector()
        {
            signaling::subscribe<signaling::ConfigRequestSignal>(
                [this](const signaling::ConfigRequestSignal& signal)
                {
                    const zlibs::utils::misc::LockGuard lock(_mutex);
                    _transports.push_back(signal.transport);
                    _frames.emplace_back(signal.data.begin(), signal.data.end());
                });
        }

        size_t count() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _frames.size();
        }

        std::vector<uint8_t> frame(size_t index) const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _frames.at(index);
        }

        signaling::ConfigTransport transport(size_t index) const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _transports.at(index);
        }

        private:
        mutable zlibs::utils::misc::Mutex       _mutex;
        std::vector<signaling::ConfigTransport> _transports = {};
        std::vector<std::vector<uint8_t>>       _frames     = {};
    };

    class WebConfigProtocolTest : public ::testing::Test
    {
        protected:
        void TearDown() override
        {
            webconfig.deinit();
            k_msleep(10);
            signaling::clear_registry();
        }

        bool wait_for_sent_frames(size_t count) const
        {
            for (size_t i = 0; i < 200; i++)
            {
                if (hwa.sent_frame_count() >= count)
                {
                    return true;
                }

                k_msleep(1);
            }

            return false;
        }

        bool wait_for_config_requests(const ConfigRequestCollector& collector, size_t count) const
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

        static midi_ump make_sysex_response_packet(std::span<const uint8_t> payload)
        {
            midi_ump packet = {};

            const bool written = zlibs::utils::midi::write_sysex7_payload_as_ump_packets(
                zlibs::utils::midi::DEFAULT_RX_GROUP,
                payload,
                [&packet](const midi_ump& generated)
                {
                    packet = generated;
                    return true;
                });

            EXPECT_TRUE(written);
            return packet;
        }

        webconfig::HwaTest          hwa;
        staged_update::HwaTest      firmware_hwa;
        staged_update::StagedUpdate firmware_update = staged_update::StagedUpdate(firmware_hwa);
        webconfig::WebConfig        webconfig       = webconfig::WebConfig(hwa, firmware_update);
    };
}    // namespace

TEST_F(WebConfigProtocolTest, StartsAndStopsServerThroughHwa)
{
    ASSERT_TRUE(webconfig.init());
    ASSERT_TRUE(webconfig.deinit());

    EXPECT_TRUE(hwa.server_stopped());
}

TEST_F(WebConfigProtocolTest, PublishesBinarySysExFramesAsConfigRequests)
{
    ConfigRequestCollector collector;

    ASSERT_TRUE(webconfig.init());
    ASSERT_EQ(webconfig.accept_client(CLIENT_SOCKET), 0);

    constexpr std::array<uint8_t, 7> request = {
        0xF0U,
        0x00U,
        0x53U,
        0x43U,
        0x00U,
        0x01U,
        0xF7U,
    };

    hwa.push_frame(request);

    ASSERT_TRUE(wait_for_config_requests(collector, 1));
    EXPECT_EQ(collector.transport(0), signaling::ConfigTransport::WebConfig);
    EXPECT_EQ(collector.frame(0), std::vector<uint8_t>(request.begin(), request.end()));
}

TEST_F(WebConfigProtocolTest, IgnoresNonBinaryFrames)
{
    ConfigRequestCollector collector;

    ASSERT_TRUE(webconfig.init());
    ASSERT_EQ(webconfig.accept_client(CLIENT_SOCKET), 0);

    constexpr std::array<uint8_t, 2> request = {
        0xF0U,
        0xF7U,
    };

    hwa.push_frame(request, webconfig::FrameInfo{
                                .binary    = false,
                                .close     = false,
                                .remaining = 0,
                            });

    k_msleep(20);
    EXPECT_EQ(collector.count(), 0);
}

TEST_F(WebConfigProtocolTest, ReplacesActiveClient)
{
    ASSERT_TRUE(webconfig.init());
    ASSERT_EQ(webconfig.accept_client(CLIENT_SOCKET), 0);
    ASSERT_EQ(webconfig.accept_client(CLIENT_SOCKET + 1), 0);

    const auto closed_sockets = hwa.closed_sockets();

    ASSERT_EQ(closed_sockets.size(), 1U);
    EXPECT_EQ(closed_sockets.front(), CLIENT_SOCKET);
}

TEST_F(WebConfigProtocolTest, SendsConfigResponsesAsSysExBytes)
{
    ASSERT_TRUE(webconfig.init());
    ASSERT_EQ(webconfig.accept_client(CLIENT_SOCKET), 0);

    constexpr std::array<uint8_t, 4> payload = {
        0x00U,
        0x53U,
        0x43U,
        0x01U,
    };

    signaling::publish(signaling::ConfigResponseSignal{
        .transport = signaling::ConfigTransport::WebConfig,
        .packet    = make_sysex_response_packet(payload),
    });

    ASSERT_TRUE(wait_for_sent_frames(1));

    const auto sent = hwa.sent_frames();
    ASSERT_EQ(sent.size(), 1U);

    const std::vector<uint8_t> expected = {
        0xF0U,
        0x00U,
        0x53U,
        0x43U,
        0x01U,
        0xF7U,
    };

    EXPECT_EQ(sent.front().socket, CLIENT_SOCKET);
    EXPECT_EQ(sent.front().data, expected);
}

TEST_F(WebConfigProtocolTest, ForwardsOscPacketsToBrowser)
{
    ASSERT_TRUE(webconfig.init());
    ASSERT_EQ(webconfig.accept_client(CLIENT_SOCKET), 0);

    constexpr std::array<uint8_t, 8> packet = {
        '/',
        'o',
        's',
        'c',
        0,
        0,
        0,
        0,
    };

    signaling::publish(signaling::OscSignal{
        .direction = signaling::SignalDirection::Out,
        .packet    = packet,
    });

    ASSERT_TRUE(wait_for_sent_frames(1));

    const auto sent = hwa.sent_frames();
    ASSERT_EQ(sent.size(), 1U);
    EXPECT_EQ(sent.front().socket, CLIENT_SOCKET);
    EXPECT_EQ(sent.front().data, std::vector<uint8_t>(packet.begin(), packet.end()));
}
