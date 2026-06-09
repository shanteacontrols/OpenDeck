/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "tests/shared/helpers/dfu_upload.h"
#include "tests/shared/helpers/dfu_stream.h"
#include "tests/shared/helpers/misc.h"
#include "bootloader/src/dfu/direct_update_writer/builder/builder.h"
#include "bootloader/src/signaling/signaling.h"
#include "bootloader/src/protocols/websockets/handler/builder/builder.h"
#include "bootloader/src/protocols/websockets/instance/impl/websockets.h"
#include "common/src/dfu/upload/shared/common.h"

#include "zlibs/utils/misc/mutex.h"

#include <algorithm>
#include <deque>
#include <span>
#include <vector>

using namespace opendeck;

namespace
{
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

    class WebSocketsHwaTest : public common::protocols::websockets::Hwa
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

        int start_server(common::protocols::websockets::Endpoint& endpoint) override
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

        int receive(int socket, std::span<uint8_t> buffer, common::protocols::websockets::FrameInfo& info) override
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

        std::vector<int> closed_sockets() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _closed_sockets;
        }

        bool server_stopped() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _server_stopped;
        }

        private:
        bool socket_closed(int socket) const
        {
            return std::find(_closed_sockets.begin(), _closed_sockets.end(), socket) != _closed_sockets.end();
        }

        mutable zlibs::utils::misc::Mutex        _mutex;
        k_sem                                    _receive_wakeup  = {};
        common::protocols::websockets::Endpoint* _endpoint        = nullptr;
        std::deque<std::vector<uint8_t>>         _received_frames = {};
        std::vector<SentFrame>                   _sent_frames     = {};
        std::vector<int>                         _closed_sockets  = {};
        bool                                     _server_stopped  = false;
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

TEST(BootloaderWebSockets, DestructorStopsActiveServer)
{
    bootloader::signaling::clear_registry();

    constexpr int CLIENT_SOCKET = 7;

    WebSocketsHwaTest                                         websockets_hwa;
    bootloader::dfu::direct_update_writer::HwaTest            writer_hwa(4, 64, 1);
    bootloader::dfu::direct_update_writer::DirectUpdateWriter writer(writer_hwa, writer_hwa);
    bootloader::protocols::websockets::handler::Builder       handlers(writer);

    {
        bootloader::protocols::websockets::WebSockets websockets(websockets_hwa);

        ASSERT_TRUE(websockets.init());
        ASSERT_EQ(websockets.accept_client(CLIENT_SOCKET), 0);
    }

    EXPECT_TRUE(websockets_hwa.server_stopped());

    const auto closed_sockets = websockets_hwa.closed_sockets();

    ASSERT_EQ(closed_sockets.size(), 1U);
    EXPECT_EQ(closed_sockets.front(), CLIENT_SOCKET);
}

TEST(BootloaderWebSockets, NetworkDfuWritesDirectUpdate)
{
    bootloader::signaling::clear_registry();

    constexpr int CLIENT_SOCKET = 7;

    const std::vector<uint8_t> payload = {
        0x10, 0x11, 0x12, 0x13, 0x14
    };

    WebSocketsHwaTest                                         websockets_hwa;
    bootloader::dfu::direct_update_writer::HwaTest            writer_hwa(4, 64, 1);
    bootloader::dfu::direct_update_writer::DirectUpdateWriter writer(writer_hwa, writer_hwa);
    bootloader::protocols::websockets::handler::Builder       handlers(writer);
    bootloader::protocols::websockets::WebSockets             websockets(websockets_hwa);

    ASSERT_TRUE(websockets.init());
    ASSERT_EQ(websockets.accept_client(CLIENT_SOCKET), 0);

    websockets_hwa.push_frame(tests::dfu_upload::command_frame(common::dfu::upload::Command::Begin));
    websockets_hwa.push_frame(tests::dfu_upload::chunk_frame(tests::dfu_stream_parser::make_stream(payload)));
    websockets_hwa.push_frame(tests::dfu_upload::command_frame(common::dfu::upload::Command::Finish));

    ASSERT_TRUE(wait_for_sent_frames(websockets_hwa, 3));
    websockets.deinit();

    const auto frames = websockets_hwa.sent_frames();

    ASSERT_EQ(frames.size(), 3);
    tests::dfu_upload::expect_ack(frames.at(0).data,
                                  common::dfu::upload::Command::Begin,
                                  common::dfu::upload::Status::Ok,
                                  0);
    tests::dfu_upload::expect_ack(frames.at(1).data,
                                  common::dfu::upload::Command::Chunk,
                                  common::dfu::upload::Status::Ok,
                                  payload.size());
    tests::dfu_upload::expect_ack(frames.at(2).data,
                                  common::dfu::upload::Command::Finish,
                                  common::dfu::upload::Status::Ok,
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
    bootloader::dfu::direct_update_writer::DirectUpdateWriter writer(writer_hwa, writer_hwa);
    bootloader::protocols::websockets::handler::Builder       handlers(writer);
    bootloader::protocols::websockets::WebSockets             websockets(websockets_hwa);

    ASSERT_TRUE(websockets.init());
    ASSERT_EQ(websockets.accept_client(CLIENT_SOCKET), 0);

    websockets_hwa.push_frame(tests::dfu_upload::command_frame(common::dfu::upload::Command::Begin));
    websockets_hwa.push_frame(tests::dfu_upload::chunk_frame(tests::dfu_stream_parser::make_stream(payload, OPENDECK_TARGET_UID ^ 0x01)));

    ASSERT_TRUE(wait_for_sent_frames(websockets_hwa, 2));
    websockets.deinit();

    const auto frames = websockets_hwa.sent_frames();

    ASSERT_EQ(frames.size(), 2);
    tests::dfu_upload::expect_ack(frames.at(0).data,
                                  common::dfu::upload::Command::Begin,
                                  common::dfu::upload::Status::Ok,
                                  0);
    tests::dfu_upload::expect_ack(frames.at(1).data,
                                  common::dfu::upload::Command::Chunk,
                                  common::dfu::upload::Status::Failed,
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
    bootloader::dfu::direct_update_writer::DirectUpdateWriter writer(writer_hwa, writer_hwa);
    bootloader::protocols::websockets::handler::Builder       handlers(writer);
    bootloader::protocols::websockets::WebSockets             websockets(websockets_hwa);

    ASSERT_TRUE(websockets.init());
    ASSERT_EQ(websockets.accept_client(CLIENT_SOCKET), 0);

    websockets_hwa.push_frame(tests::dfu_upload::command_frame(common::dfu::upload::Command::Begin));
    websockets_hwa.push_frame(tests::dfu_upload::command_frame(common::dfu::upload::Command::Abort));

    ASSERT_TRUE(wait_for_sent_frames(websockets_hwa, 2));
    websockets.deinit();

    const auto frames = websockets_hwa.sent_frames();

    ASSERT_EQ(frames.size(), 2);
    tests::dfu_upload::expect_ack(frames.at(0).data,
                                  common::dfu::upload::Command::Begin,
                                  common::dfu::upload::Status::Ok,
                                  0);
    tests::dfu_upload::expect_ack(frames.at(1).data,
                                  common::dfu::upload::Command::Abort,
                                  common::dfu::upload::Status::Ok,
                                  0);

    ASSERT_FALSE(writer_hwa.updated);
    ASSERT_TRUE(writer_hwa.written_bytes.empty());
}
