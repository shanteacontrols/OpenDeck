/*

Copyright Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

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

#ifdef __cplusplus
extern "C"
{
#endif
#include <ump_stream_responder.h>
#ifdef __cplusplus
}
#endif

#include <optional>

namespace protocol::midi
{
#ifndef CONFIG_TARGET_SUPPORT_USB
#error "CONFIG_TARGET_SUPPORT_USB must be enabled through OpenDeck devicetree/Kconfig."
#endif

#define OPENDECK_DIN_MIDI_NODE DT_CHOSEN(opendeck_din_midi)

#if DT_NODE_HAS_PROP(OPENDECK_DIN_MIDI_NODE, uart)
#define OPENDECK_DIN_MIDI_UART_NODE DT_PHANDLE(OPENDECK_DIN_MIDI_NODE, uart)
#endif

#ifdef CONFIG_TARGET_SUPPORT_DIN_MIDI
#ifndef OPENDECK_DIN_MIDI_UART_NODE
#error "Chosen OpenDeck DIN MIDI node must define a UART phandle."
#endif
#endif

    class HwaUsbHw : public HwaUsb
    {
        public:
        HwaUsbHw() = default;

        bool supported() override
        {
            return true;
        }

        bool init() override
        {
            _instance = this;
            _ready    = false;

            k_fifo_init(&_rx_fifo);

            if (k_mem_slab_init(&_rx_slab, _rx_slab_buffer, sizeof(RxPacket), RX_PACKET_COUNT) != 0)
            {
                return false;
            }

            if (!_usb_driver.init())
            {
                return false;
            }

            if (!device_is_ready(MIDI_DEVICE))
            {
                return false;
            }

            _responder_cfg = UMP_STREAM_RESPONDER(MIDI_DEVICE, usbd_midi_send, &_midi_ump_ep_dt);

            _ops.rx_packet_cb = []([[maybe_unused]] const struct device* dev, const struct midi_ump ump)
            {
                if (_instance != nullptr)
                {
                    _instance->on_midi_packet(ump);
                }
            };

            _ops.ready_cb = []([[maybe_unused]] const struct device* dev, const bool ready)
            {
                if (_instance != nullptr)
                {
                    _instance->_ready = ready;
                }
            };

            usbd_midi_set_ops(MIDI_DEVICE, &_ops);

            return true;
        }

        bool deinit() override
        {
            _ready = false;
            return _usb_driver.deinit();
        }

        bool write(const midi_ump& packet) override
        {
            if (!_ready)
            {
                return false;
            }

            return usbd_midi_send(MIDI_DEVICE, packet) == 0;
        }

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
        struct RxPacket
        {
            void*    fifo_reserved = nullptr;
            midi_ump ump           = {};
        };

        static constexpr size_t                  RX_PACKET_COUNT                      = 16;
        static inline HwaUsbHw*                  _instance                            = nullptr;
        static inline const ump_endpoint_dt_spec _midi_ump_ep_dt                      = UMP_ENDPOINT_DT_SPEC_GET(DT_NODELABEL(usb_midi));
        const device* const                      MIDI_DEVICE                          = DEVICE_DT_GET(DT_NODELABEL(usb_midi));
        const device* const                      UDC_DEVICE                           = DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0));
        zlibs::drivers::usb::UsbHw&              _usb_driver                          = zlibs::drivers::usb::UsbHw::instance(UDC_DEVICE);
        usbd_midi_ops                            _ops                                 = {};
        ump_stream_responder_cfg                 _responder_cfg                       = {};
        k_fifo                                   _rx_fifo                             = {};
        k_mem_slab                               _rx_slab                             = {};
        alignas(RxPacket) uint8_t _rx_slab_buffer[RX_PACKET_COUNT * sizeof(RxPacket)] = {};
        bool _ready                                                                   = false;

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

        RxPacket* allocate_rx_packet()
        {
            void* memory = nullptr;

            return (k_mem_slab_alloc(&_rx_slab, &memory, K_NO_WAIT) == 0) ? static_cast<RxPacket*>(memory) : nullptr;
        }
    };

    class HwaSerialHw : public HwaSerial
    {
        public:
        HwaSerialHw() = default;

        bool supported() override
        {
#ifdef CONFIG_TARGET_SUPPORT_DIN_MIDI
            return true;
#else
            return false;
#endif
        }

        bool init() override
        {
#ifndef CONFIG_TARGET_SUPPORT_DIN_MIDI
            return false;
#else
            _initialized = _uart.init(_config);

            return _initialized;
#endif
        }

        bool deinit() override
        {
#ifndef CONFIG_TARGET_SUPPORT_DIN_MIDI
            return true;
#else
            const bool result = _uart.deinit();
            _initialized      = false;
            return result;
#endif
        }

        bool setLoopback(bool state) override
        {
#ifndef CONFIG_TARGET_SUPPORT_DIN_MIDI
            return false;
#else
            if (_config.loopback == state)
            {
                return true;
            }

            const bool was_initialized = _initialized;

            if (was_initialized && !_uart.deinit())
            {
                return false;
            }

            _initialized     = false;
            _config.loopback = state;

            if (!was_initialized)
            {
                return true;
            }

            _initialized = _uart.init(_config);
            return _initialized;
#endif
        }

        bool write(uint8_t data) override
        {
#ifdef CONFIG_TARGET_SUPPORT_DIN_MIDI
            return _uart.write(data);
#else
            return false;
#endif
        }

        std::optional<uint8_t> read() override
        {
#ifdef CONFIG_TARGET_SUPPORT_DIN_MIDI
            return _uart.read();
#else
            return {};
#endif
        }

        bool allocated(io::common::Allocatable::interface_t interface) override
        {
            return interface == io::common::Allocatable::interface_t::UART ? _initialized : false;
        }

        private:
        static constexpr size_t   BUFFER_SIZE = 128;
        static constexpr uint32_t BAUDRATE    = 31250;
#ifdef CONFIG_TARGET_SUPPORT_DIN_MIDI
        const device* const _device = DEVICE_DT_GET(OPENDECK_DIN_MIDI_UART_NODE);
#else
        const device* const _device = nullptr;
#endif
        zlibs::utils::misc::RingBuffer<BUFFER_SIZE> _rx_buffer = {};
        zlibs::utils::misc::RingBuffer<BUFFER_SIZE> _tx_buffer = {};
        zlibs::drivers::uart::Config                _config    = zlibs::drivers::uart::Config(
            BAUDRATE,
            UART_CFG_STOP_BITS_1,
            _rx_buffer,
            _tx_buffer);
        zlibs::drivers::uart::UartHw _uart        = zlibs::drivers::uart::UartHw(_device);
        bool                         _initialized = false;
    };

    class HwaBleHw : public HwaBle
    {
        public:
        HwaBleHw() = default;

        bool supported() override
        {
            return false;
        }

        bool init() override
        {
            return false;
        }

        bool deinit() override
        {
            return true;
        }

        bool write(BlePacket& packet) override
        {
            return false;
        }

        std::optional<BlePacket> read() override
        {
            return {};
        }
    };
}    // namespace protocol::midi
