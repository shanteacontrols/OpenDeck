/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/touchscreen/deps.h"

#include "zlibs/drivers/uart/uart_hw.h"
#include "zlibs/utils/misc/ring_buffer.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>

#define OPENDECK_TOUCHSCREEN_NODE DT_NODELABEL(opendeck_touchscreen)

#if !DT_NODE_HAS_PROP(OPENDECK_TOUCHSCREEN_NODE, uart)
#error "Chosen OpenDeck touchscreen node must define a UART phandle."
#endif
#define OPENDECK_TOUCHSCREEN_UART_NODE DT_PHANDLE(OPENDECK_TOUCHSCREEN_NODE, uart)

namespace opendeck::io::touchscreen
{
    /**
     * @brief Hardware-backed touchscreen backend that proxies to a UART transport.
     */
    class HwaHw : public Hwa
    {
        public:
        HwaHw() = default;

        /**
         * @brief Initializes the UART transport used by the touchscreen.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        bool init() override
        {
            _initialized = _uart.init(_config);
            return _initialized;
        }

        /**
         * @brief Deinitializes the UART transport.
         *
         * @return `true` if deinitialization succeeded, otherwise `false`.
         */
        bool deinit() override
        {
            auto ret     = _uart.deinit();
            _initialized = false;
            return ret;
        }

        /**
         * @brief Writes one byte to the touchscreen UART.
         *
         * @param value Byte to transmit.
         *
         * @return `true` if the byte was queued successfully, otherwise `false`.
         */
        bool write(uint8_t value) override
        {
            return _uart.write(value);
        }

        /**
         * @brief Reads one byte from the touchscreen UART.
         *
         * @return Received byte, or `std::nullopt` when none is available.
         */
        std::optional<uint8_t> read() override
        {
            return _uart.read();
        }

        /**
         * @brief Returns whether the UART interface is currently allocated by this backend.
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
        static constexpr size_t   BUFFER_SIZE = 256;
        static constexpr uint32_t BAUDRATE    = 38400;

        const device* const                           _touchscreen_device = DEVICE_DT_GET(OPENDECK_TOUCHSCREEN_UART_NODE);
        zlibs::drivers::uart::RingBuffer<BUFFER_SIZE> _rx_buffer          = {};
        zlibs::drivers::uart::RingBuffer<BUFFER_SIZE> _tx_buffer          = {};
        zlibs::drivers::uart::Config                  _config             = zlibs::drivers::uart::Config(
            BAUDRATE,
            UART_CFG_STOP_BITS_1,
            _rx_buffer,
            _tx_buffer);
        zlibs::drivers::uart::UartHw _uart        = zlibs::drivers::uart::UartHw(_touchscreen_device);
        bool                         _initialized = false;
    };
}    // namespace opendeck::io::touchscreen
