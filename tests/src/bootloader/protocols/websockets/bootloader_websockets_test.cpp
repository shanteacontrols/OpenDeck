/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "tests/shared/helpers/dfu_stream.h"
#include "tests/shared/helpers/misc.h"
#include "bootloader/src/dfu/direct_update_writer/builder/builder.h"
#include "bootloader/src/signaling/signaling.h"
#include "bootloader/src/protocols/websockets/instance/impl/websockets.h"
#include "common/src/protocols/websockets/shared/firmware_upload.h"

#include "zlibs/utils/misc/mutex.h"

#include <algorithm>
#include <cstring>
#include <deque>
#include <span>
#include <vector>

namespace
{
    std::vector<uint8_t> command_frame(opendeck::common::protocols::websockets::FirmwareUploadCommand command)
    {
        return {
            static_cast<uint8_t>(command),
        };
    }

    std::vector<uint8_t> chunk_frame(const std::vector<uint8_t>& payload)
    {
        auto frame = command_frame(opendeck::common::protocols::websockets::FirmwareUploadCommand::Chunk);
        frame.insert(frame.end(), payload.begin(), payload.end());
        return frame;
    }

    uint32_t ack_bytes_written(const std::vector<uint8_t>& response)
    {
        uint32_t bytes_written = 0;
        std::memcpy(&bytes_written, response.data() + 3, sizeof(bytes_written));
        return bytes_written;
    }

    void expect_ack(const std::vector<uint8_t>&                                    response,
                    opendeck::common::protocols::websockets::FirmwareUploadCommand command,
                    opendeck::common::protocols::websockets::FirmwareUploadStatus  status,
                    uint32_t                                                       bytes_written)
    {
        ASSERT_EQ(response.size(), opendeck::common::protocols::websockets::FIRMWARE_UPLOAD_ACK_SIZE);
        EXPECT_EQ(response.at(0), static_cast<uint8_t>(opendeck::common::protocols::websockets::FirmwareUploadResponse::Ack));
        EXPECT_EQ(response.at(1), static_cast<uint8_t>(command));
        EXPECT_EQ(response.at(2), static_cast<uint8_t>(status));
        EXPECT_EQ(ack_bytes_written(response), bytes_written);
    }

    void assert_written_image(const std::vector<uint8_t>& expected, const std::vector<uint8_t>& actual)
    {
        ASSERT_GE(actual.size(), expected.size());

        for (size_t i = 0; i < expected.size(); i++)
        {
            ASSERT_EQ(expected.at(i), actual.at(i)) << "Difference on byte " << i;
        }

        for (size_t i = expected.size(); i < actual.size(); i++)
        {
            ASSERT_EQ(0xFF, actual.at(i)) << "Unexpected padding on byte " << i;
        }
    }

    class WebSocketsHwaTest : public opendeck::common::protocols::websockets::Hwa
    {
        public:
        struct SentFrame
        {
            int                  socket = -1;
            std::vector<uint8_t> data   = {};
        };

        WebSocketsHwaTest()
        {
            k_sem_init(&_receive_wakeup, 0, 1);
        }

        int start_server(opendeck::common::protocols::websockets::Endpoint& endpoint) override
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _endpoint = &endpoint;
            return 0;
        }

        void stop_server() override
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _server_stopped = true;
        }

        int receive(int socket, std::span<uint8_t> buffer, opendeck::common::protocols::websockets::FrameInfo& info) override
        {
            while (true)
            {
                {
                    const zlibs::utils::misc::LockGuard lock(_mutex);

                    if (socket_closed(socket))
                    {
                        return -ENOTCONN;
                    }

                    if (!_received_frames.empty())
                    {
                        const auto frame = _received_frames.front();
                        _received_frames.pop_front();

                        const size_t copied = std::min(buffer.size(), frame.size());
                        std::copy(frame.begin(), frame.begin() + copied, buffer.begin());
                        info = {
                            .binary    = true,
                            .close     = false,
                            .remaining = 0,
                        };

                        return static_cast<int>(copied);
                    }
                }

                k_sem_take(&_receive_wakeup, K_FOREVER);
            }
        }

        int send(int socket, std::span<const uint8_t> data) override
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _sent_frames.push_back({
                .socket = socket,
                .data   = std::vector<uint8_t>(data.begin(), data.end()),
            });

            return static_cast<int>(data.size());
        }

        void unregister(int socket) override
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _closed_sockets.push_back(socket);
            k_sem_give(&_receive_wakeup);
        }

        void push_frame(const std::vector<uint8_t>& data)
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _received_frames.push_back(data);
            k_sem_give(&_receive_wakeup);
        }

        size_t sent_frame_count() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _sent_frames.size();
        }

        std::vector<SentFrame> sent_frames() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _sent_frames;
        }

        private:
        bool socket_closed(int socket) const
        {
            return std::find(_closed_sockets.begin(), _closed_sockets.end(), socket) != _closed_sockets.end();
        }

        mutable zlibs::utils::misc::Mutex                  _mutex;
        k_sem                                              _receive_wakeup  = {};
        opendeck::common::protocols::websockets::Endpoint* _endpoint        = nullptr;
        std::deque<std::vector<uint8_t>>                   _received_frames = {};
        std::vector<SentFrame>                             _sent_frames     = {};
        std::vector<int>                                   _closed_sockets  = {};
        bool                                               _server_stopped  = false;
    };

    bool wait_for_sent_frames(const WebSocketsHwaTest& hwa, size_t count)
    {
        return tests::wait_until([&hwa, count]
                                 {
                                     return hwa.sent_frame_count() >= count;
                                 },
                                 200,
                                 1);
    }
}    // namespace

TEST(BootloaderWebSockets, NetworkDfuWritesDirectUpdate)
{
    bootloader::signaling::clear_registry();

    constexpr int CLIENT_SOCKET = 7;

    const std::vector<uint8_t> payload = {
        0x10, 0x11, 0x12, 0x13, 0x14
    };

    WebSocketsHwaTest                                         websockets_hwa;
    bootloader::dfu::direct_update_writer::HwaTest            writer_hwa(4, 64, 1);
    bootloader::dfu::direct_update_writer::DirectUpdateWriter writer(writer_hwa);
    bootloader::protocols::websockets::WebSockets             websockets(websockets_hwa, writer);

    ASSERT_TRUE(websockets.init());
    ASSERT_EQ(websockets.accept_client(CLIENT_SOCKET), 0);

    websockets_hwa.push_frame(command_frame(opendeck::common::protocols::websockets::FirmwareUploadCommand::Begin));
    websockets_hwa.push_frame(chunk_frame(opendeck::tests::dfu_stream::make_stream(payload)));
    websockets_hwa.push_frame(command_frame(opendeck::common::protocols::websockets::FirmwareUploadCommand::Finish));

    ASSERT_TRUE(wait_for_sent_frames(websockets_hwa, 3));
    websockets.deinit();

    const auto frames = websockets_hwa.sent_frames();

    ASSERT_EQ(frames.size(), 3);
    expect_ack(frames.at(0).data,
               opendeck::common::protocols::websockets::FirmwareUploadCommand::Begin,
               opendeck::common::protocols::websockets::FirmwareUploadStatus::Ok,
               0);
    expect_ack(frames.at(1).data,
               opendeck::common::protocols::websockets::FirmwareUploadCommand::Chunk,
               opendeck::common::protocols::websockets::FirmwareUploadStatus::Ok,
               payload.size());
    expect_ack(frames.at(2).data,
               opendeck::common::protocols::websockets::FirmwareUploadCommand::Finish,
               opendeck::common::protocols::websockets::FirmwareUploadStatus::Ok,
               payload.size());

    ASSERT_TRUE(writer_hwa.updated);
    assert_written_image(payload, writer_hwa.written_bytes);
}

TEST(BootloaderWebSockets, NetworkDfuRejectsInvalidHeader)
{
    bootloader::signaling::clear_registry();

    constexpr int CLIENT_SOCKET = 7;

    const std::vector<uint8_t> payload = {
        0xAA, 0xBB, 0xCC, 0xDD
    };

    WebSocketsHwaTest                                         websockets_hwa;
    bootloader::dfu::direct_update_writer::HwaTest            writer_hwa(4, 64, 1);
    bootloader::dfu::direct_update_writer::DirectUpdateWriter writer(writer_hwa);
    bootloader::protocols::websockets::WebSockets             websockets(websockets_hwa, writer);

    ASSERT_TRUE(websockets.init());
    ASSERT_EQ(websockets.accept_client(CLIENT_SOCKET), 0);

    websockets_hwa.push_frame(command_frame(opendeck::common::protocols::websockets::FirmwareUploadCommand::Begin));
    websockets_hwa.push_frame(chunk_frame(opendeck::tests::dfu_stream::make_stream(payload, OPENDECK_TARGET_UID ^ 0x01)));

    ASSERT_TRUE(wait_for_sent_frames(websockets_hwa, 2));
    websockets.deinit();

    const auto frames = websockets_hwa.sent_frames();

    ASSERT_EQ(frames.size(), 2);
    expect_ack(frames.at(0).data,
               opendeck::common::protocols::websockets::FirmwareUploadCommand::Begin,
               opendeck::common::protocols::websockets::FirmwareUploadStatus::Ok,
               0);
    expect_ack(frames.at(1).data,
               opendeck::common::protocols::websockets::FirmwareUploadCommand::Chunk,
               opendeck::common::protocols::websockets::FirmwareUploadStatus::Failed,
               0);

    ASSERT_FALSE(writer_hwa.updated);
    ASSERT_TRUE(writer_hwa.erased_sectors.empty());
    ASSERT_EQ(0, writer_hwa.write_count);
}

TEST(BootloaderWebSockets, NetworkDfuAbortResetsDirectUpdate)
{
    bootloader::signaling::clear_registry();

    constexpr int CLIENT_SOCKET = 7;

    WebSocketsHwaTest                                         websockets_hwa;
    bootloader::dfu::direct_update_writer::HwaTest            writer_hwa(4, 64, 1);
    bootloader::dfu::direct_update_writer::DirectUpdateWriter writer(writer_hwa);
    bootloader::protocols::websockets::WebSockets             websockets(websockets_hwa, writer);

    ASSERT_TRUE(websockets.init());
    ASSERT_EQ(websockets.accept_client(CLIENT_SOCKET), 0);

    websockets_hwa.push_frame(command_frame(opendeck::common::protocols::websockets::FirmwareUploadCommand::Begin));
    websockets_hwa.push_frame(command_frame(opendeck::common::protocols::websockets::FirmwareUploadCommand::Abort));

    ASSERT_TRUE(wait_for_sent_frames(websockets_hwa, 2));
    websockets.deinit();

    const auto frames = websockets_hwa.sent_frames();

    ASSERT_EQ(frames.size(), 2);
    expect_ack(frames.at(0).data,
               opendeck::common::protocols::websockets::FirmwareUploadCommand::Begin,
               opendeck::common::protocols::websockets::FirmwareUploadStatus::Ok,
               0);
    expect_ack(frames.at(1).data,
               opendeck::common::protocols::websockets::FirmwareUploadCommand::Abort,
               opendeck::common::protocols::websockets::FirmwareUploadStatus::Ok,
               0);

    ASSERT_FALSE(writer_hwa.updated);
    ASSERT_TRUE(writer_hwa.written_bytes.empty());
}
