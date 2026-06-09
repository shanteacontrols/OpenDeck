/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "tests/shared/helpers/dfu_upload.h"
#include "tests/shared/helpers/dfu_stream.h"
#include "tests/shared/helpers/misc.h"
#include "bootloader/src/protocols/webusb/instance/impl/webusb.h"
#include "common/src/dfu/dfu_stream_parser/writer/test/dfu_writer_test.h"
#include "bootloader/src/signaling/signaling.h"

#include <algorithm>
#include <deque>
#include <memory>
#include <span>
#include <vector>

using namespace opendeck;

namespace
{
    class WebUsbHwaTest : public bootloader::protocols::webusb::Hwa
    {
        public:
        bool init() override
        {
            initialized = true;
            return init_result;
        }

        bool deinit() override
        {
            deinitialized             = true;
            rx_callback               = {};
            connection_state_callback = {};

            return deinit_result;
        }

        void status(std::string_view message) override
        {
            statuses.emplace_back(message);
        }

        void register_rx_callback(RxCallback callback) override
        {
            rx_callback = std::move(callback);
        }

        void register_connection_state_callback(ConnectionStateCallback callback) override
        {
            connection_state_callback = std::move(callback);
        }

        bool push_rx(std::span<const uint8_t> data)
        {
            if (!rx_callback)
            {
                return false;
            }

            received_chunks.emplace_back(data.begin(), data.end());
            const auto received_data = std::span<const uint8_t>(received_chunks.back().data(), received_chunks.back().size());

            const bool accepted = rx_callback(bootloader::protocols::webusb::DfuRxChunk{
                .data = received_data,
            });

            if (accepted)
            {
                accepted_buffers.push_back(received_data.data());
            }

            return accepted;
        }

        void release_rx_buffer(bootloader::protocols::webusb::DfuRxChunk chunk) override
        {
            release_after_deinit = release_after_deinit || deinitialized;

            const auto accepted = std::find(accepted_buffers.begin(), accepted_buffers.end(), chunk.data.data());
            const auto released = std::find(released_buffers.begin(), released_buffers.end(), chunk.data.data());

            EXPECT_NE(accepted, accepted_buffers.end());
            EXPECT_EQ(released, released_buffers.end());

            released_buffers.push_back(chunk.data.data());
        }

        void connect()
        {
            if (connection_state_callback)
            {
                connection_state_callback(true);
            }
        }

        void disconnect()
        {
            if (connection_state_callback)
            {
                connection_state_callback(false);
            }
        }

        bool                             init_result               = true;
        bool                             deinit_result             = true;
        bool                             initialized               = false;
        bool                             deinitialized             = false;
        bool                             release_after_deinit      = false;
        RxCallback                       rx_callback               = {};
        ConnectionStateCallback          connection_state_callback = {};
        std::vector<std::string>         statuses                  = {};
        std::deque<std::vector<uint8_t>> received_chunks           = {};
        std::vector<const uint8_t*>      accepted_buffers          = {};
        std::vector<const uint8_t*>      released_buffers          = {};
    };

    class BlockingDfuWriterTest : public common::dfu::dfu_stream_parser::DfuWriterTest
    {
        public:
        bool begin(const common::dfu::dfu_stream_parser::Header& header, const uint32_t size) override
        {
            k_sem_give(&entered);
            k_sem_take(&release, K_FOREVER);
            return DfuWriterTest::begin(header, size);
        }

        k_sem entered = Z_SEM_INITIALIZER(entered, 0, 1);
        k_sem release = Z_SEM_INITIALIZER(release, 0, 1);
    };

    bool wait_for_status_count(const WebUsbHwaTest& hwa, size_t count)
    {
        return tests::wait_until([&hwa, count]
                                 {
                                     return hwa.statuses.size() >= count;
                                 },
                                 500,
                                 5);
    }

    bool wait_for_finish_count(const common::dfu::dfu_stream_parser::DfuWriterTest& writer, size_t count)
    {
        return tests::wait_until([&writer, count]
                                 {
                                     return writer.finish_called >= count;
                                 },
                                 500,
                                 5);
    }

    bool wait_for_begin_count(const common::dfu::dfu_stream_parser::DfuWriterTest& writer, size_t count)
    {
        return tests::wait_until([&writer, count]
                                 {
                                     return writer.begin_called >= count;
                                 },
                                 500,
                                 5);
    }

    class WebUsbTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            bootloader::signaling::clear_registry();
            webusb = std::make_unique<bootloader::protocols::webusb::WebUsb>(hwa, writer);
        }

        WebUsbHwaTest                                          hwa;
        common::dfu::dfu_stream_parser::DfuWriterTest          writer;
        std::unique_ptr<bootloader::protocols::webusb::WebUsb> webusb;
    };
}    // namespace

TEST_F(WebUsbTest, ConnectPublishesStatus)
{
    ASSERT_TRUE(webusb->init());

    hwa.connect();

    ASSERT_TRUE(wait_for_status_count(hwa, 1));
    EXPECT_EQ(hwa.statuses.at(0), "WebUSB connected");
}

TEST_F(WebUsbTest, DfuStatusSignalPublishesStatus)
{
    ASSERT_TRUE(webusb->init());
    hwa.connect();
    ASSERT_TRUE(wait_for_status_count(hwa, 1));

    ASSERT_TRUE(bootloader::signaling::publish(common::signaling::DfuStatusSignal("test status")));

    ASSERT_TRUE(wait_for_status_count(hwa, 2));
    EXPECT_EQ(hwa.statuses.back(), "test status");
}

TEST_F(WebUsbTest, DestructorDeinitializesHwaAndClearsCallbacks)
{
    ASSERT_TRUE(webusb->init());
    ASSERT_TRUE(hwa.rx_callback);
    ASSERT_TRUE(hwa.connection_state_callback);

    hwa.connect();
    ASSERT_TRUE(hwa.push_rx(tests::dfu_upload::command_frame(common::dfu::upload::Command::Begin)));

    webusb.reset();

    EXPECT_TRUE(hwa.deinitialized);
    EXPECT_FALSE(hwa.rx_callback);
    EXPECT_FALSE(hwa.connection_state_callback);
    EXPECT_FALSE(hwa.push_rx(tests::dfu_upload::command_frame(common::dfu::upload::Command::Begin)));
    EXPECT_FALSE(hwa.release_after_deinit);
    EXPECT_EQ(hwa.released_buffers, hwa.accepted_buffers);
}

TEST_F(WebUsbTest, CompleteUploadWorksWithSplitChunkFrame)
{
    const std::vector<uint8_t> payload = {
        0x10, 0x11, 0x12, 0x13, 0x14
    };

    ASSERT_TRUE(webusb->init());
    hwa.connect();

    const auto dfu    = tests::dfu_stream_parser::make_stream(payload);
    const auto begin  = tests::dfu_upload::command_frame(common::dfu::upload::Command::Begin);
    const auto chunk  = tests::dfu_upload::chunk_frame(dfu);
    const auto finish = tests::dfu_upload::command_frame(common::dfu::upload::Command::Finish);

    ASSERT_TRUE(hwa.push_rx(begin));
    ASSERT_TRUE(hwa.push_rx(std::span<const uint8_t>(chunk.data(), 2)));
    ASSERT_TRUE(hwa.push_rx(std::span<const uint8_t>(chunk.data() + 2, chunk.size() - 2)));
    ASSERT_TRUE(hwa.push_rx(finish));

    ASSERT_TRUE(wait_for_finish_count(writer, 1));
    EXPECT_EQ(writer.payload(), payload);
}

TEST_F(WebUsbTest, HandlesMultipleFramesInSingleRxBuffer)
{
    const std::vector<uint8_t> payload = {
        0x21, 0x22, 0x23, 0x24
    };

    ASSERT_TRUE(webusb->init());
    hwa.connect();

    const auto dfu    = tests::dfu_stream_parser::make_stream(payload);
    const auto begin  = tests::dfu_upload::command_frame(common::dfu::upload::Command::Begin);
    const auto chunk  = tests::dfu_upload::chunk_frame(dfu);
    const auto finish = tests::dfu_upload::command_frame(common::dfu::upload::Command::Finish);

    std::vector<uint8_t> combined = begin;
    combined.insert(combined.end(), chunk.begin(), chunk.end());
    combined.insert(combined.end(), finish.begin(), finish.end());

    ASSERT_TRUE(hwa.push_rx(combined));

    ASSERT_TRUE(wait_for_finish_count(writer, 1));
    EXPECT_EQ(writer.payload(), payload);
}

TEST_F(WebUsbTest, InvalidFrameResetsAndRestartWorks)
{
    const std::vector<uint8_t> payload = {
        0x31, 0x32, 0x33, 0x34
    };

    ASSERT_TRUE(webusb->init());
    hwa.connect();

    constexpr std::array<uint8_t, 1> invalid = {
        0x7FU,
    };

    ASSERT_TRUE(hwa.push_rx(invalid));
    ASSERT_TRUE(wait_for_status_count(hwa, 2));
    EXPECT_EQ(hwa.statuses.back(), "Invalid WebUSB DFU frame");

    const auto dfu    = tests::dfu_stream_parser::make_stream(payload);
    const auto begin  = tests::dfu_upload::command_frame(common::dfu::upload::Command::Begin);
    const auto chunk  = tests::dfu_upload::chunk_frame(dfu);
    const auto finish = tests::dfu_upload::command_frame(common::dfu::upload::Command::Finish);

    ASSERT_TRUE(hwa.push_rx(begin));
    ASSERT_TRUE(hwa.push_rx(chunk));
    ASSERT_TRUE(wait_for_begin_count(writer, 1));
    ASSERT_TRUE(hwa.push_rx(finish));

    ASSERT_TRUE(wait_for_finish_count(writer, 1));
    EXPECT_EQ(writer.payload(), payload);
}

TEST_F(WebUsbTest, FailedUploadResetsAndRestartWorks)
{
    const std::vector<uint8_t> payload = {
        0x41, 0x42, 0x43, 0x44
    };

    ASSERT_TRUE(webusb->init());
    hwa.connect();

    const auto begin     = tests::dfu_upload::command_frame(common::dfu::upload::Command::Begin);
    const auto bad_dfu   = tests::dfu_stream_parser::make_stream(payload, OPENDECK_TARGET_UID + 1U);
    const auto bad_chunk = tests::dfu_upload::chunk_frame(bad_dfu);

    ASSERT_TRUE(hwa.push_rx(begin));
    ASSERT_TRUE(hwa.push_rx(bad_chunk));
    ASSERT_TRUE(wait_for_status_count(hwa, 2));
    EXPECT_EQ(hwa.statuses.back(), "WebUSB DFU command failed");

    const auto good_dfu   = tests::dfu_stream_parser::make_stream(payload);
    const auto good_chunk = tests::dfu_upload::chunk_frame(good_dfu);
    const auto finish     = tests::dfu_upload::command_frame(common::dfu::upload::Command::Finish);

    ASSERT_TRUE(hwa.push_rx(begin));
    ASSERT_TRUE(hwa.push_rx(good_chunk));
    ASSERT_TRUE(hwa.push_rx(finish));

    ASSERT_TRUE(wait_for_finish_count(writer, 1));
    EXPECT_EQ(writer.payload(), payload);
}

TEST_F(WebUsbTest, DisconnectClearsPartialUploadAndReconnectStartsClean)
{
    const std::vector<uint8_t> payload = {
        0x51, 0x52, 0x53, 0x54
    };

    ASSERT_TRUE(webusb->init());
    hwa.connect();

    const auto begin = tests::dfu_upload::command_frame(common::dfu::upload::Command::Begin);
    const auto dfu   = tests::dfu_stream_parser::make_stream(payload);
    const auto chunk = tests::dfu_upload::chunk_frame(dfu);

    ASSERT_TRUE(hwa.push_rx(begin));
    ASSERT_TRUE(hwa.push_rx(std::span<const uint8_t>(chunk.data(), 2)));

    hwa.disconnect();
    hwa.connect();
    ASSERT_TRUE(wait_for_status_count(hwa, 2));

    const auto finish = tests::dfu_upload::command_frame(common::dfu::upload::Command::Finish);

    ASSERT_TRUE(hwa.push_rx(begin));
    ASSERT_TRUE(hwa.push_rx(chunk));
    ASSERT_TRUE(hwa.push_rx(finish));

    ASSERT_TRUE(wait_for_finish_count(writer, 1));
    EXPECT_EQ(writer.payload(), payload);
}

TEST_F(WebUsbTest, BeginRestartsPartialUploadWithoutDisconnect)
{
    const std::vector<uint8_t> payload = {
        0x59, 0x5A, 0x5B, 0x5C
    };

    ASSERT_TRUE(webusb->init());
    hwa.connect();

    const auto dfu   = tests::dfu_stream_parser::make_stream(payload);
    const auto begin = tests::dfu_upload::command_frame(common::dfu::upload::Command::Begin);

    constexpr size_t DFU_HEADER_SIZE = 16U;
    const auto       partial_chunk   = tests::dfu_upload::chunk_frame(std::vector<uint8_t>(dfu.begin(), dfu.begin() + DFU_HEADER_SIZE));

    ASSERT_TRUE(hwa.push_rx(begin));
    ASSERT_TRUE(hwa.push_rx(partial_chunk));
    ASSERT_TRUE(wait_for_begin_count(writer, 1));

    ASSERT_TRUE(hwa.push_rx(begin));
    ASSERT_TRUE(tests::wait_until([this]
                                  {
                                      return writer.abort_called == 1;
                                  },
                                  500,
                                  5));

    const auto chunk  = tests::dfu_upload::chunk_frame(dfu);
    const auto finish = tests::dfu_upload::command_frame(common::dfu::upload::Command::Finish);

    ASSERT_TRUE(hwa.push_rx(chunk));
    ASSERT_TRUE(hwa.push_rx(finish));

    ASSERT_TRUE(wait_for_finish_count(writer, 1));
    EXPECT_EQ(writer.begin_called, 2U);
    EXPECT_EQ(writer.payload(), payload);
}

TEST_F(WebUsbTest, AbortCommandResetsAndAllowsRetry)
{
    const std::vector<uint8_t> payload = {
        0x61, 0x62, 0x63, 0x64
    };

    ASSERT_TRUE(webusb->init());
    hwa.connect();

    const auto begin = tests::dfu_upload::command_frame(common::dfu::upload::Command::Begin);
    const auto abort = tests::dfu_upload::command_frame(common::dfu::upload::Command::Abort);

    ASSERT_TRUE(hwa.push_rx(begin));
    ASSERT_TRUE(hwa.push_rx(abort));
    k_msleep(10);

    const auto dfu    = tests::dfu_stream_parser::make_stream(payload);
    const auto chunk  = tests::dfu_upload::chunk_frame(dfu);
    const auto finish = tests::dfu_upload::command_frame(common::dfu::upload::Command::Finish);

    ASSERT_TRUE(hwa.push_rx(begin));
    ASSERT_TRUE(hwa.push_rx(chunk));
    ASSERT_TRUE(hwa.push_rx(finish));

    ASSERT_TRUE(wait_for_finish_count(writer, 1));
    EXPECT_EQ(writer.payload(), payload);
}

TEST_F(WebUsbTest, OversizedPacketIsRejectedAndRestartWorks)
{
    const std::vector<uint8_t> raw(bootloader::protocols::webusb::USB_BUFFER_SIZE + 1U, 0);

    ASSERT_FALSE(hwa.push_rx(raw));

    ASSERT_FALSE(hwa.statuses.empty());
    EXPECT_EQ(hwa.statuses.back(), "WebUSB DFU RX buffer overflow");

    const std::vector<uint8_t> payload = {
        0x71, 0x72, 0x73, 0x74
    };

    ASSERT_TRUE(webusb->init());
    hwa.connect();

    const auto begin  = tests::dfu_upload::command_frame(common::dfu::upload::Command::Begin);
    const auto chunk  = tests::dfu_upload::chunk_frame(tests::dfu_stream_parser::make_stream(payload));
    const auto finish = tests::dfu_upload::command_frame(common::dfu::upload::Command::Finish);

    ASSERT_TRUE(hwa.push_rx(begin));
    ASSERT_TRUE(hwa.push_rx(chunk));
    ASSERT_TRUE(hwa.push_rx(finish));

    ASSERT_TRUE(wait_for_finish_count(writer, 1));
    EXPECT_EQ(writer.payload(), payload);
}

TEST_F(WebUsbTest, OversizedPacketWhileConnectedDoesNotBlockRetry)
{
    const std::vector<uint8_t> oversized(bootloader::protocols::webusb::USB_BUFFER_SIZE + 1U, 0);
    const std::vector<uint8_t> payload = {
        0x75, 0x76, 0x77, 0x78
    };

    ASSERT_TRUE(webusb->init());
    hwa.connect();
    ASSERT_TRUE(wait_for_status_count(hwa, 1));

    ASSERT_FALSE(hwa.push_rx(oversized));
    ASSERT_FALSE(hwa.statuses.empty());
    EXPECT_EQ(hwa.statuses.back(), "WebUSB DFU RX buffer overflow");

    const auto begin  = tests::dfu_upload::command_frame(common::dfu::upload::Command::Begin);
    const auto chunk  = tests::dfu_upload::chunk_frame(tests::dfu_stream_parser::make_stream(payload));
    const auto finish = tests::dfu_upload::command_frame(common::dfu::upload::Command::Finish);

    ASSERT_TRUE(hwa.push_rx(begin));
    ASSERT_TRUE(hwa.push_rx(chunk));
    ASSERT_TRUE(hwa.push_rx(finish));

    ASSERT_TRUE(wait_for_finish_count(writer, 1));
    EXPECT_EQ(writer.payload(), payload);
}

TEST_F(WebUsbTest, DataBeforeConnectIsRejectedAndConnectStartsClean)
{
    const auto begin = tests::dfu_upload::command_frame(common::dfu::upload::Command::Begin);

    ASSERT_TRUE(webusb->init());
    EXPECT_FALSE(hwa.push_rx(begin));
    EXPECT_TRUE(hwa.accepted_buffers.empty());
    EXPECT_TRUE(hwa.released_buffers.empty());

    const std::vector<uint8_t> payload = {
        0x81, 0x82, 0x83, 0x84
    };

    hwa.connect();

    const auto chunk  = tests::dfu_upload::chunk_frame(tests::dfu_stream_parser::make_stream(payload));
    const auto finish = tests::dfu_upload::command_frame(common::dfu::upload::Command::Finish);

    ASSERT_TRUE(hwa.push_rx(begin));
    ASSERT_TRUE(hwa.push_rx(chunk));
    ASSERT_TRUE(hwa.push_rx(finish));

    ASSERT_TRUE(wait_for_finish_count(writer, 1));
    EXPECT_EQ(writer.payload(), payload);
}

TEST(WebUsbRuntimeTest, RuntimeQueueFullResetsAndAllowsRetry)
{
    bootloader::signaling::clear_registry();

    WebUsbHwaTest              hwa;
    BlockingDfuWriterTest      writer;
    auto                       webusb           = bootloader::protocols::webusb::WebUsb(hwa, writer);
    const auto                 begin            = tests::dfu_upload::command_frame(common::dfu::upload::Command::Begin);
    const std::vector<uint8_t> blocking_payload = {
        0x89, 0x8A, 0x8B, 0x8C
    };
    const auto blocking_chunk = tests::dfu_upload::chunk_frame(tests::dfu_stream_parser::make_stream(blocking_payload));

    ASSERT_TRUE(webusb.init());
    hwa.connect();
    ASSERT_TRUE(wait_for_status_count(hwa, 1));

    ASSERT_TRUE(hwa.push_rx(begin));
    ASSERT_TRUE(hwa.push_rx(blocking_chunk));
    ASSERT_EQ(k_sem_take(&writer.entered, K_MSEC(500)), 0);

    while (hwa.push_rx(begin))
    {
    }

    EXPECT_EQ(hwa.statuses.back(), "WebUSB DFU RX buffer overflow");
    ASSERT_FALSE(hwa.accepted_buffers.empty());

    k_sem_give(&writer.release);

    ASSERT_TRUE(tests::wait_until([&hwa]
                                  {
                                      return hwa.released_buffers.size() == hwa.accepted_buffers.size();
                                  },
                                  500,
                                  5));
    EXPECT_EQ(hwa.released_buffers, hwa.accepted_buffers);

    const auto chunk  = tests::dfu_upload::chunk_frame(tests::dfu_stream_parser::make_stream(blocking_payload));
    const auto finish = tests::dfu_upload::command_frame(common::dfu::upload::Command::Finish);

    ASSERT_TRUE(hwa.push_rx(begin));
    ASSERT_TRUE(hwa.push_rx(chunk));
    ASSERT_EQ(k_sem_take(&writer.entered, K_MSEC(500)), 0);
    k_sem_give(&writer.release);
    ASSERT_TRUE(hwa.push_rx(finish));

    ASSERT_TRUE(wait_for_finish_count(writer, 1));
    EXPECT_EQ(writer.payload(), blocking_payload);
}
