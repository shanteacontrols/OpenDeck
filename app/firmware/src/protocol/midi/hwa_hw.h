/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "zlibs/drivers/usb/usb_hw.h"
#include "zlibs/drivers/uart/uart_hw.h"
#include "zlibs/utils/misc/ring_buffer.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/usb/class/usbd_midi2.h>

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_BLE
#include "ble_service.h"

#include <algorithm>

#include <zephyr/bluetooth/bluetooth.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#include <ump_stream_responder.h>
#ifdef __cplusplus
}
#endif

#include <optional>
#include <inttypes.h>
#include <span>
#include <cerrno>

namespace opendeck::protocol::midi
{
#define OPENDECK_DIN_MIDI_NODE DT_NODELABEL(opendeck_din_midi)

#if DT_NODE_HAS_PROP(OPENDECK_DIN_MIDI_NODE, uart)
#define OPENDECK_DIN_MIDI_UART_NODE DT_PHANDLE(OPENDECK_DIN_MIDI_NODE, uart)
#endif

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_DIN_MIDI
#ifndef OPENDECK_DIN_MIDI_UART_NODE
#error "Chosen OpenDeck DIN MIDI node must define a UART phandle."
#endif
#endif

    /**
     * @brief Hardware-backed USB MIDI transport backend.
     */
    class HwaUsbHw : public HwaUsb
    {
        public:
        /**
         * @brief Number of enabled USB MIDI UMP blocks declared under the `usb_midi` devicetree node.
         */
        static constexpr size_t BLOCK_COUNT = DT_FOREACH_CHILD_SEP(DT_NODELABEL(usb_midi), DT_NODE_HAS_STATUS_OKAY, (+));

        HwaUsbHw() = default;

        /**
         * @brief Returns whether USB MIDI transport is supported.
         *
         * @return Always `true`.
         */
        bool supported() override
        {
            return true;
        }

        /**
         * @brief Returns the poll signal raised when USB MIDI data becomes available.
         *
         * @return Pointer to the receive-data poll signal.
         */
        k_poll_signal* data_available_signal() override
        {
            return &_data_available_signal;
        }

        /**
         * @brief Registers the callback invoked after the USB MIDI device becomes ready.
         *
         * @param handler Callback to invoke when the transport reports readiness.
         */
        void register_on_ready_handler(UsbReadyHandler&& handler) override
        {
            _ready_handler = std::move(handler);
        }

        /**
         * @brief Returns whether USB MIDI is ready for packet exchange.
         *
         * @return `true` when the USB MIDI function is ready for TX/RX.
         */
        bool ready() override
        {
            return _ready;
        }

        /**
         * @brief Initializes the USB MIDI transport and responder callbacks.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        bool init() override
        {
            instance = this;
            _ready   = false;

            k_fifo_init(&_rx_fifo);
            k_poll_signal_init(&_data_available_signal);

            if (k_mem_slab_init(&_rx_slab, _rx_slab_buffer, sizeof(RxPacket), RX_PACKET_COUNT) != 0)
            {
                return false;
            }

            if (!_usb_driver.init({}))
            {
                return false;
            }

            if (!device_is_ready(_midi_device))
            {
                return false;
            }

            _responder_cfg = UMP_STREAM_RESPONDER(_midi_device, usbd_midi_send, &midi_ump_endpoint());

            _ops.rx_packet_cb = []([[maybe_unused]] const struct device* dev, const struct midi_ump ump)
            {
                if (instance != nullptr)
                {
                    instance->on_midi_packet(ump);
                }
            };

            _ops.ready_cb = []([[maybe_unused]] const struct device* dev, const bool ready)
            {
                if (instance != nullptr)
                {
                    instance->_ready = ready;

                    if (ready && (instance->_ready_handler != nullptr))
                    {
                        instance->_ready_handler();
                    }
                }
            };

            usbd_midi_set_ops(_midi_device, &_ops);

            return true;
        }

        /**
         * @brief Deinitializes the USB MIDI transport.
         *
         * @return `true` if the USB backend deinitialized successfully, otherwise `false`.
         */
        bool deinit() override
        {
            _ready = false;
            k_poll_signal_reset(&_data_available_signal);
            return _usb_driver.deinit();
        }

        /**
         * @brief Sends one UMP packet over USB MIDI.
         *
         * @param packet UMP packet to transmit.
         *
         * @return `true` if the packet was sent successfully, otherwise `false`.
         */
        bool write(const midi_ump& packet) override
        {
            if (!_ready)
            {
                return false;
            }

            return usbd_midi_send(_midi_device, packet) == 0;
        }

        /**
         * @brief Sends a burst of UMP packets over USB MIDI.
         *
         * @param packets UMP packets to transmit.
         *
         * @return `true` if the burst was sent successfully, otherwise `false`.
         */
        bool write(std::span<const midi_ump> packets) override
        {
            if (!_ready)
            {
                return false;
            }

            if (packets.empty())
            {
                return true;
            }

            return usbd_midi_sendv(_midi_device, packets.data(), packets.size()) == 0;
        }

        /**
         * @brief Reads one queued UMP packet received over USB MIDI.
         *
         * @return Received UMP packet, or `std::nullopt` when no packet is queued.
         */
        std::optional<midi_ump> read() override
        {
            auto* packet = static_cast<RxPacket*>(k_fifo_get(&_rx_fifo, K_NO_WAIT));

            if (packet == nullptr)
            {
                return {};
            }

            const auto ump = packet->ump;
            k_mem_slab_free(&_rx_slab, packet);

            return ump;
        }

        private:
        /**
         * @brief Static USB endpoint descriptor storage used to bridge devicetree data to the UMP responder.
         */
        struct EndpointStorage
        {
            const char*       name;
            size_t            n_blocks;
            ump_block_dt_spec blocks[BLOCK_COUNT];
        };

        /**
         * @brief FIFO node used to queue received USB MIDI packets.
         */
        struct RxPacket
        {
            void*    fifo_reserved = nullptr;
            midi_ump ump           = {};
        };

        static constexpr size_t RX_PACKET_COUNT = 16;

        static inline HwaUsbHw*     instance                                          = nullptr;
        const device* const         _midi_device                                      = DEVICE_DT_GET(DT_NODELABEL(usb_midi));
        const device* const         _udc_device                                       = DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0));
        zlibs::drivers::usb::UsbHw& _usb_driver                                       = zlibs::drivers::usb::UsbHw::instance(_udc_device);
        usbd_midi_ops               _ops                                              = {};
        ump_stream_responder_cfg    _responder_cfg                                    = {};
        k_fifo                      _rx_fifo                                          = {};
        k_mem_slab                  _rx_slab                                          = {};
        alignas(RxPacket) uint8_t _rx_slab_buffer[RX_PACKET_COUNT * sizeof(RxPacket)] = {};
        k_poll_signal   _data_available_signal                                        = {};
        UsbReadyHandler _ready_handler                                                = nullptr;
        bool            _ready                                                        = false;

        static constexpr EndpointStorage ENDPOINT_STORAGE = {
            .name     = DT_PROP_OR(DT_NODELABEL(usb_midi), label, nullptr),
            .n_blocks = BLOCK_COUNT,
            .blocks   = { DT_FOREACH_CHILD(DT_NODELABEL(usb_midi), UMP_BLOCK_SEP_IF_OKAY) },
        };

        /**
         * @brief Returns the static endpoint descriptor used by the UMP responder.
         *
         * @return Reference to the USB MIDI UMP endpoint descriptor.
         */
        static const ump_endpoint_dt_spec& midi_ump_endpoint()
        {
            return *reinterpret_cast<const ump_endpoint_dt_spec*>(&ENDPOINT_STORAGE);
        }

        /**
         * @brief Processes one received USB MIDI UMP packet.
         *
         * @param ump Received UMP packet.
         */
        void on_midi_packet(const midi_ump ump)
        {
            switch (UMP_MT(ump))
            {
            case UMP_MT_MIDI1_CHANNEL_VOICE:
            case UMP_MT_DATA_64:
            {
                auto* packet = allocate_rx_packet();

                if (packet == nullptr)
                {
                    break;
                }

                packet->ump = ump;
                k_fifo_put(&_rx_fifo, packet);
                k_poll_signal_raise(&_data_available_signal, 0);
            }
            break;

            case UMP_MT_UMP_STREAM:
            {
                ump_stream_respond(&_responder_cfg, ump);
            }
            break;

            default:
                break;
            }
        }

        /**
         * @brief Allocates one FIFO packet node from the receive slab.
         *
         * @return Pointer to the allocated packet node, or `nullptr` when the slab is exhausted.
         */
        RxPacket* allocate_rx_packet()
        {
            void* memory = nullptr;

            return (k_mem_slab_alloc(&_rx_slab, &memory, K_NO_WAIT) == 0) ? static_cast<RxPacket*>(memory) : nullptr;
        }
    };

    /**
     * @brief Hardware-backed serial/DIN MIDI transport backend.
     */
    class HwaSerialHw : public HwaSerial
    {
        public:
        /**
         * @brief Number of bytes retained in the DIN MIDI transmit and receive buffers.
         */
        static constexpr size_t BUFFER_SIZE = 128;

        /**
         * @brief Standard DIN MIDI UART baud rate.
         */
        static constexpr uint32_t BAUDRATE = 31250;

        /**
         * @brief Constructs the hardware-backed serial MIDI backend.
         */
        HwaSerialHw()
            : _rx_buffer(_data_available_signal)
            , _config(
                  BAUDRATE,
                  UART_CFG_STOP_BITS_1,
                  _rx_buffer,
                  _tx_buffer)
        {
            k_poll_signal_init(&_data_available_signal);
        }

        /**
         * @brief Returns whether DIN MIDI transport is enabled in the target configuration.
         *
         * @return `true` when DIN MIDI support is compiled in, otherwise `false`.
         */
        bool supported() override
        {
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_DIN_MIDI
            return true;
#else
            return false;
#endif
        }

        /**
         * @brief Returns the poll signal raised when serial MIDI data becomes available.
         *
         * @return Pointer to the receive-data poll signal.
         */
        k_poll_signal* data_available_signal() override
        {
            return &_data_available_signal;
        }

        /**
         * @brief Initializes the DIN MIDI UART transport.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        bool init() override
        {
#ifndef CONFIG_PROJECT_TARGET_SUPPORT_DIN_MIDI
            return false;
#else
            k_poll_signal_reset(&_data_available_signal);
            _initialized = _uart.init(_config);

            return _initialized;
#endif
        }

        /**
         * @brief Deinitializes the DIN MIDI UART transport.
         *
         * @return `true` if deinitialization succeeded, otherwise `false`.
         */
        bool deinit() override
        {
#ifndef CONFIG_PROJECT_TARGET_SUPPORT_DIN_MIDI
            return true;
#else
            auto ret     = _uart.deinit();
            _initialized = false;
            k_poll_signal_reset(&_data_available_signal);
            return ret;
#endif
        }

        /**
         * @brief Enables or disables UART loopback mode.
         *
         * @param state `true` to enable loopback, `false` to disable it.
         *
         * @return `true` if the requested loopback state was applied, otherwise `false`.
         */
        bool set_loopback([[maybe_unused]] bool state) override
        {
#ifndef CONFIG_PROJECT_TARGET_SUPPORT_DIN_MIDI
            return false;
#else
            if (_config.loopback == state)
            {
                return true;
            }

            auto was_inititialized = _initialized;

            if (was_inititialized && !_uart.deinit())
            {
                return false;
            }

            _initialized     = false;
            _config.loopback = state;

            if (!was_inititialized)
            {
                return true;
            }

            _initialized = _uart.init(_config);
            return _initialized;
#endif
        }

        /**
         * @brief Sends one serial MIDI byte.
         *
         * @param data MIDI byte to transmit.
         *
         * @return `true` if the byte was queued successfully, otherwise `false`.
         */
        bool write([[maybe_unused]] uint8_t data) override
        {
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_DIN_MIDI
            return _uart.write(data);
#else
            return false;
#endif
        }

        /**
         * @brief Reads one received serial MIDI byte.
         *
         * @return Received byte, or `std::nullopt` when no byte is available.
         */
        std::optional<uint8_t> read() override
        {
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_DIN_MIDI
            return _uart.read();
#else
            return {};
#endif
        }

        /**
         * @brief Returns whether the requested interface is currently allocated by this backend.
         *
         * @param interface Interface to query.
         *
         * @return `true` only for UART when the backend is initialized, otherwise `false`.
         */
        bool allocated(io::common::Allocatable::Interface interface) override
        {
            return interface == io::common::Allocatable::Interface::Uart ? _initialized : false;
        }

        private:
        /**
         * @brief UART receive ring buffer that raises a poll signal whenever new data is inserted.
         */
        class SignalAwareRingBuffer : public zlibs::drivers::uart::RingBufferBase
        {
            public:
            /**
             * @brief Constructs a ring buffer that raises a poll signal on successful inserts.
             *
             * @param signal Poll signal raised when new data is written to the buffer.
             */
            explicit SignalAwareRingBuffer(k_poll_signal& signal)
                : _signal(signal)
            {}

            /**
             * @brief Returns the ring buffer capacity in bytes.
             *
             * @return Number of bytes the buffer can hold.
             */
            size_t size() override
            {
                return _buffer.size();
            }

            /**
             * @brief Returns whether the receive buffer currently holds no data.
             *
             * @return `true` if the buffer is empty, otherwise `false`.
             */
            bool is_empty() override
            {
                return _buffer.is_empty();
            }

            /**
             * @brief Returns whether the receive buffer is full.
             *
             * @return `true` if the buffer cannot accept more data, otherwise `false`.
             */
            bool is_full() override
            {
                return _buffer.is_full();
            }

            /**
             * @brief Inserts one received byte into the buffer and raises the poll signal on success.
             *
             * @param data Byte to queue.
             *
             * @return `true` if the byte was inserted, otherwise `false`.
             */
            bool insert(uint8_t data) override
            {
                const bool inserted = _buffer.insert(data);

                if (inserted)
                {
                    k_poll_signal_raise(&_signal, 0);
                }

                return inserted;
            }

            /**
             * @brief Returns the next queued byte without removing it.
             *
             * @return Next queued byte, or `std::nullopt` when the buffer is empty.
             */
            std::optional<uint8_t> peek() override
            {
                return _buffer.peek();
            }

            /**
             * @brief Removes and returns the next queued byte.
             *
             * @return Removed byte, or `std::nullopt` when the buffer is empty.
             */
            std::optional<uint8_t> remove() override
            {
                return _buffer.remove();
            }

            /**
             * @brief Clears all queued receive data.
             */
            void reset() override
            {
                _buffer.reset();
            }

            private:
            zlibs::utils::misc::RingBuffer<BUFFER_SIZE, false, uint8_t> _buffer = {};
            k_poll_signal&                                              _signal;
        };

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_DIN_MIDI
        const device* const _uart_device = DEVICE_DT_GET(OPENDECK_DIN_MIDI_UART_NODE);
#else
        const device* const _uart_device = nullptr;
#endif
        k_poll_signal                                 _data_available_signal = {};
        SignalAwareRingBuffer                         _rx_buffer;
        zlibs::drivers::uart::RingBuffer<BUFFER_SIZE> _tx_buffer = {};
        zlibs::drivers::uart::Config                  _config;
        zlibs::drivers::uart::UartHw                  _uart        = zlibs::drivers::uart::UartHw(_uart_device);
        bool                                          _initialized = false;
    };

    /**
     * @brief Hardware-backed BLE MIDI transport backend.
     */
    class HwaBleHw : public HwaBle
    {
        public:
        /**
         * @brief Number of raw BLE MIDI packets retained in the receive queue.
         */
        static constexpr size_t RX_PACKET_COUNT = 8;

        /**
         * @brief Constructs the hardware-backed BLE MIDI backend.
         */
        HwaBleHw()
        {
            k_poll_signal_init(&_data_available_signal);
        }

        /**
         * @brief Returns whether BLE MIDI transport is supported.
         *
         * @return `true` when BLE MIDI support is enabled for the build.
         */
        bool supported() override
        {
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_BLE
            return true;
#else
            return false;
#endif
        }

        /**
         * @brief Returns the poll signal raised when BLE MIDI data becomes available.
         *
         * @return Pointer to the BLE receive-data poll signal.
         */
        k_poll_signal* data_available_signal() override
        {
            return &_data_available_signal;
        }

        /**
         * @brief Initializes the BLE MIDI backend and starts advertising.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        bool init() override
        {
#ifndef CONFIG_PROJECT_TARGET_SUPPORT_BLE
            return false;
#else
            if (_initialized)
            {
                return true;
            }

            instance     = this;
            _ready       = false;
            _drop_logged = false;
            _rx_packets.reset();
            k_poll_signal_reset(&_data_available_signal);

            if (!bt_initialized)
            {
                if (bt_enable(nullptr) != 0)
                {
                    return false;
                }

                bt_initialized = true;
            }

            if (!ble_service::init(&ready_state_changed, &raw_packet_received))
            {
                return false;
            }

            if (!ble_service::start_advertising())
            {
                return false;
            }

            _initialized = true;
            return true;
#endif
        }

        /**
         * @brief Deinitializes the BLE MIDI backend and stops advertising.
         *
         * @return `true` if deinitialization succeeded, otherwise `false`.
         */
        bool deinit() override
        {
#ifndef CONFIG_PROJECT_TARGET_SUPPORT_BLE
            return true;
#else
            _initialized = false;
            _ready       = false;
            _drop_logged = false;
            _rx_packets.reset();
            k_poll_signal_reset(&_data_available_signal);
            return ble_service::stop_advertising();
#endif
        }

        /**
         * @brief Sends one BLE MIDI packet.
         *
         * @param packet BLE MIDI packet to transmit.
         *
         * @return `true` if the packet was sent successfully, otherwise `false`.
         */
        bool write([[maybe_unused]] BlePacket& packet) override
        {
#ifndef CONFIG_PROJECT_TARGET_SUPPORT_BLE
            return false;
#else
            if (!_initialized || !_ready)
            {
                return false;
            }

            return ble_service::send_packet(packet.data.data(), static_cast<uint16_t>(packet.size));
#endif
        }

        /**
         * @brief Reads one queued BLE MIDI packet.
         *
         * @return Received packet, or `std::nullopt` when no packet is queued.
         */
        std::optional<BlePacket> read() override
        {
            return _rx_packets.remove();
        }

        /**
         * @brief Returns whether the BLE MIDI backend is ready for packet transmission.
         *
         * @return `true` when a central is connected and notifications are enabled.
         */
        bool ready() override
        {
            return _ready;
        }

        private:
        static inline HwaBleHw* instance       = nullptr;
        static inline bool      bt_initialized = false;

        /**
         * @brief Updates the backend ready state when the BLE service connection changes.
         *
         * @param ready True if the BLE MIDI transport is ready for TX, false otherwise.
         */
        static void ready_state_changed(bool ready)
        {
            if (instance != nullptr)
            {
                instance->_ready = ready;
            }
        }

        /**
         * @brief Queues a raw BLE MIDI packet received from the GATT service.
         *
         * @param bytes Pointer to the received BLE MIDI packet bytes.
         * @param size  Packet length in bytes.
         */
        static void raw_packet_received(const uint8_t* bytes, uint16_t size)
        {
            if ((instance == nullptr) || (bytes == nullptr))
            {
                return;
            }

            instance->enqueue_packet(bytes, size);
        }

        /**
         * @brief Stores a received BLE MIDI packet in the RX queue and signals availability.
         *
         * Packets larger than the backend scratch buffer are ignored. If the RX queue is full,
         * the packet is dropped.
         *
         * @param bytes Pointer to the received BLE MIDI packet bytes.
         * @param size  Packet length in bytes.
         */
        void enqueue_packet(const uint8_t* bytes, uint16_t size)
        {
            if (size > _rx_scratch.data.size())
            {
                return;
            }

            _rx_scratch.size = size;
            std::copy_n(bytes, size, _rx_scratch.data.begin());

            if (!_rx_packets.insert(_rx_scratch))
            {
                if (!_drop_logged)
                {
                    _drop_logged = true;
                }

                return;
            }

            _drop_logged = false;
            k_poll_signal_raise(&_data_available_signal, 0);
        }

        k_poll_signal                                                     _data_available_signal = {};
        zlibs::utils::misc::RingBuffer<RX_PACKET_COUNT, false, BlePacket> _rx_packets            = {};
        BlePacket                                                         _rx_scratch            = {};
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_BLE
        bool _initialized = false;
#endif
        bool _ready       = false;
        bool _drop_logged = false;
    };
}    // namespace opendeck::protocol::midi
