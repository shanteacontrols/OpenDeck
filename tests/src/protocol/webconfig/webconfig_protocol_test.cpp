/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"
#include "firmware/src/protocol/osc/packet/packet.h"
#include "firmware/src/protocol/osc/shared/paths.h"
#include "firmware/src/protocol/webconfig/hwa/test/hwa_test.h"
#include "firmware/src/protocol/webconfig/instance/impl/webconfig.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/staged_update_writer/hwa/test/hwa_test.h"
#include "firmware/src/staged_update_writer/instance/impl/staged_update_writer.h"

#include "zlibs/utils/midi/midi.h"
#include "zlibs/utils/misc/mutex.h"

#include <zephyr/kernel.h>

#include <array>
#include <span>
#include <vector>

using namespace opendeck;
using namespace opendeck::protocol;

namespace
{
    constexpr int      CLIENT_SOCKET         = 7;
    constexpr uint32_t ACTIVE_CLIENT_SESSION = 1;
    constexpr uint32_t STALE_CLIENT_SESSION  = 0;

    class ConfigRequestCollector
    {
        public:
        ConfigRequestCollector()
        {
            signaling::subscribe<signaling::ConfigRequestSignal>(
                [this](const signaling::ConfigRequestSignal& signal)
                {
                    const zlibs::utils::misc::LockGuard lock(_mutex);
                    const auto                          data = signal.data();
                    _transports.push_back(signal.transport);
                    _session_ids.push_back(signal.session_id);
                    _frames.emplace_back(data.begin(), data.end());
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

        uint32_t session_id(size_t index) const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _session_ids.at(index);
        }

        private:
        mutable zlibs::utils::misc::Mutex       _mutex;
        std::vector<signaling::ConfigTransport> _transports  = {};
        std::vector<uint32_t>                   _session_ids = {};
        std::vector<std::vector<uint8_t>>       _frames      = {};
    };

    class WebConfigProtocolTest : public ::testing::Test
    {
        protected:
        void TearDown() override
        {
            webconfig.deinit();
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

        static std::vector<midi_ump> make_sysex_response_packets(std::span<const uint8_t> payload)
        {
            std::vector<midi_ump> packets = {};

            const bool written = zlibs::utils::midi::write_sysex7_payload_as_ump_packets(
                zlibs::utils::midi::DEFAULT_RX_GROUP,
                payload,
                [&packets](const midi_ump& generated)
                {
                    packets.push_back(generated);
                    return true;
                });

            EXPECT_TRUE(written);
            return packets;
        }

        static midi_ump make_sysex_response_packet(std::span<const uint8_t> payload)
        {
            const auto packets = make_sysex_response_packets(payload);

            EXPECT_FALSE(packets.empty());
            return packets.empty() ? midi_ump{} : packets.back();
        }

        webconfig::HwaTest                       hwa;
        staged_update_writer::HwaTest            firmware_hwa;
        staged_update_writer::StagedUpdateWriter firmware_update = staged_update_writer::StagedUpdateWriter(firmware_hwa);
        webconfig::WebConfig                     webconfig       = webconfig::WebConfig(hwa, firmware_update);
    };
}    // namespace

TEST_F(WebConfigProtocolTest, StartsAndStopsServerThroughHwa)
{
    ASSERT_TRUE(webconfig.init());
    ASSERT_TRUE(webconfig.deinit());

    EXPECT_TRUE(hwa.server_stopped());
}

TEST_F(WebConfigProtocolTest, InitFailsWhenServerCannotStart)
{
    ConfigRequestCollector collector;

    hwa.set_start_result(-EIO);

    ASSERT_FALSE(webconfig.init());
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
    EXPECT_FALSE(wait_for_config_requests(collector, 1));
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
    EXPECT_EQ(collector.session_id(0), ACTIVE_CLIENT_SESSION);
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
        .transport  = signaling::ConfigTransport::WebConfig,
        .packet     = make_sysex_response_packet(payload),
        .session_id = ACTIVE_CLIENT_SESSION,
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

TEST_F(WebConfigProtocolTest, DropsConfigResponsesFromStaleClientSession)
{
    ASSERT_TRUE(webconfig.init());
    ASSERT_EQ(webconfig.accept_client(CLIENT_SOCKET), 0);

    constexpr std::array<uint8_t, 4> payload = {
        0x00U,
        0x53U,
        0x43U,
        0x01U,
    };

    ASSERT_TRUE(signaling::publish(signaling::ConfigResponseSignal{
        .transport  = signaling::ConfigTransport::WebConfig,
        .packet     = make_sysex_response_packet(payload),
        .session_id = STALE_CLIENT_SESSION,
    }));

    ASSERT_TRUE(zlibs::utils::signaling::drain());
    EXPECT_EQ(hwa.sent_frame_count(), 0U);
}

TEST_F(WebConfigProtocolTest, StaleConfigResponseDoesNotClearActivePartialResponse)
{
    ASSERT_TRUE(webconfig.init());
    ASSERT_EQ(webconfig.accept_client(CLIENT_SOCKET), 0);

    constexpr std::array<uint8_t, 10> active_payload = {
        0x00U,
        0x53U,
        0x43U,
        0x01U,
        0x02U,
        0x03U,
        0x04U,
        0x05U,
        0x06U,
        0x07U,
    };
    constexpr std::array<uint8_t, 4> stale_payload = {
        0x00U,
        0x53U,
        0x43U,
        0x09U,
    };

    const auto active_packets = make_sysex_response_packets(active_payload);
    ASSERT_GE(active_packets.size(), 2U);

    ASSERT_TRUE(signaling::publish(signaling::ConfigResponseSignal{
        .transport  = signaling::ConfigTransport::WebConfig,
        .packet     = active_packets.front(),
        .session_id = ACTIVE_CLIENT_SESSION,
    }));
    ASSERT_TRUE(zlibs::utils::signaling::drain());
    EXPECT_EQ(hwa.sent_frame_count(), 0U);

    ASSERT_TRUE(signaling::publish(signaling::ConfigResponseSignal{
        .transport  = signaling::ConfigTransport::WebConfig,
        .packet     = make_sysex_response_packet(stale_payload),
        .session_id = STALE_CLIENT_SESSION,
    }));
    ASSERT_TRUE(zlibs::utils::signaling::drain());
    EXPECT_EQ(hwa.sent_frame_count(), 0U);

    for (auto it = active_packets.begin() + 1; it != active_packets.end(); ++it)
    {
        ASSERT_TRUE(signaling::publish(signaling::ConfigResponseSignal{
            .transport  = signaling::ConfigTransport::WebConfig,
            .packet     = *it,
            .session_id = ACTIVE_CLIENT_SESSION,
        }));
    }

    ASSERT_TRUE(wait_for_sent_frames(1));

    const auto sent = hwa.sent_frames();
    ASSERT_EQ(sent.size(), 1U);

    std::vector<uint8_t> expected = { 0xF0U };
    expected.insert(expected.end(), active_payload.begin(), active_payload.end());
    expected.push_back(0xF7U);

    EXPECT_EQ(sent.front().socket, CLIENT_SOCKET);
    EXPECT_EQ(sent.front().data, expected);
}

TEST_F(WebConfigProtocolTest, ForwardsOscPacketsToBrowser)
{
    ASSERT_TRUE(webconfig.init());
    ASSERT_EQ(webconfig.accept_client(CLIENT_SOCKET), 0);

    signaling::publish(signaling::OscIoSignal{
        .source          = signaling::IoEventSource::Switch,
        .component_index = 0,
        .int32_value     = 1,
        .direction       = signaling::SignalDirection::Out,
    });

    ASSERT_TRUE(wait_for_sent_frames(1));

    const auto sent = hwa.sent_frames();
    ASSERT_EQ(sent.size(), 1U);
    EXPECT_EQ(sent.front().socket, CLIENT_SOCKET);
    osc::PacketBuffer expected = {};
    const auto        size     = osc::make_packet(expected,
                                                  osc::OscIndexedAddress{
                                                      .prefix = osc::paths::SWITCH.c_str(),
                                                      .index  = 0,
                                                  },
                                                  osc::OscInt32{ 1 });

    ASSERT_TRUE(size);
    EXPECT_EQ(sent.front().data, std::vector<uint8_t>(expected.begin(), expected.begin() + *size));
}
