/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/threads.h"
#include "bootloader/src/signaling/signaling.h"
#include "bootloader/src/protocols/webusb/instance/impl/deps.h"
#include "bootloader/src/protocols/webusb/shared/common.h"
#include "common/src/dfu/upload/instance/impl/upload.h"

#include <zephyr/kernel.h>

#include <atomic>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace opendeck::bootloader::protocols::webusb
{
    /**
     * @brief Bootloader WebUSB endpoint.
     */
    class WebUsb
    {
        public:
        /**
         * @brief Constructs the WebUSB endpoint.
         *
         * @param hwa Platform hooks used by WebUSB.
         * @param writer DFU writer that receives accepted firmware payload bytes.
         */
        explicit WebUsb(Hwa& hwa, opendeck::common::dfu::writer::DfuWriter& writer);

        /**
         * @brief Deinitializes the hardware adapter when WebUSB was initialized.
         */
        ~WebUsb();

        /**
         * @brief Initializes WebUSB.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool init();

        private:
        static constexpr size_t RX_QUEUE_DEPTH           = 8;
        static constexpr size_t RX_CONTROL_QUEUE_RESERVE = 3;

        enum class RxMessageType
        {
            Data,
            Connect,
            Disconnect,
            Reset,
            Stop,
        };

        struct RxMessage
        {
            RxMessageType type = RxMessageType::Data;
            DfuRxChunk    data = {};
        };

        Hwa&                                                                _hwa;
        opendeck::common::dfu::upload::Upload                               _firmware_upload;
        std::array<uint8_t, opendeck::common::dfu::upload::FRAME_SIZE * 2U> _rx_buffer      = {};
        std::array<RxMessageType, RX_QUEUE_DEPTH>                           _control_buffer = {};
        size_t                                                              _rx_size        = 0;
        k_msgq                                                              _rx_msgq        = {};
        std::array<char, sizeof(RxMessage) * RX_QUEUE_DEPTH>                _rx_msgq_buffer = {};
        std::atomic_bool                                                    _initialized    = false;
        std::atomic_bool                                                    _connected      = false;
        std::atomic_bool                                                    _accepting_rx   = false;
        bootloader::threads::WebUsbRxThread                                 _rx_thread;

        /**
         * @brief Queues one received WebUSB OUT buffer for the RX thread.
         *
         * @param chunk Received USB buffer and ownership metadata.
         *
         * @return `true` when the RX thread took ownership and will release the buffer, otherwise `false`.
         */
        bool receive(DfuRxChunk chunk);

        /**
         * @brief Queues a WebUSB connection-state change for serialized RX-thread handling.
         *
         * @param connected `true` when WebUSB is connected, otherwise `false`.
         */
        void handle_connection_state(bool connected);

        /**
         * @brief Queues a control message for the RX thread.
         *
         * @param type Control message type.
         */
        void queue_control(RxMessageType type);

        /**
         * @brief Runs the RX-thread loop that owns DFU upload parsing state.
         */
        void process_rx();

        /**
         * @brief Aborts the active upload and clears pending parser state.
         *
         * @param drain_queue When `true`, releases queued data buffers and preserves queued control messages.
         */
        void reset(bool drain_queue);

        /**
         * @brief Requests the RX thread to stop and reject further received buffers.
         */
        void stop_rx();
    };
}    // namespace opendeck::bootloader::protocols::webusb
