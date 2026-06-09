/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/protocols/webusb/instance/impl/webusb.h"

#include <algorithm>
#include <cstring>

using namespace opendeck::bootloader::protocols;

webusb::WebUsb::WebUsb(Hwa& hwa, opendeck::common::dfu::writer::DfuWriter& writer)
    : _hwa(hwa)
    , _firmware_upload(writer)
    , _rx_thread([this]()
                 {
                     process_rx();
                 },
                 [this]()
                 {
                     stop_rx();
                 })
{
    k_msgq_init(&_rx_msgq, _rx_msgq_buffer.data(), sizeof(RxMessage), RX_QUEUE_DEPTH);

    _hwa.register_rx_callback([this](DfuRxChunk chunk)
                              {
                                  return receive(chunk);
                              });

    _hwa.register_connection_state_callback([this](bool connected)
                                            {
                                                handle_connection_state(connected);
                                            });

    opendeck::bootloader::signaling::subscribe<opendeck::common::signaling::DfuStatusSignal>(
        [this](const opendeck::common::signaling::DfuStatusSignal& signal)
        {
            _hwa.status(signal.message());
        });
}

webusb::WebUsb::~WebUsb()
{
    if (_initialized)
    {
        _rx_thread.destroy();
        _hwa.deinit();
    }
}

bool webusb::WebUsb::init()
{
    const bool initialized = _hwa.init();

    if (initialized)
    {
        _initialized = true;
        _rx_thread.run();
    }

    return initialized;
}

bool webusb::WebUsb::receive(DfuRxChunk chunk)
{
    if (chunk.data.empty() || (chunk.data.size() > USB_BUFFER_SIZE))
    {
        _hwa.status("WebUSB DFU RX buffer overflow");
        queue_control(RxMessageType::Reset);
        _accepting_rx = _connected.load();

        return false;
    }

    if (!_accepting_rx)
    {
        return false;
    }

    if (k_msgq_num_free_get(&_rx_msgq) <= RX_CONTROL_QUEUE_RESERVE)
    {
        _hwa.status("WebUSB DFU RX buffer overflow");
        queue_control(RxMessageType::Reset);

        return false;
    }

    const RxMessage message = {
        .type = RxMessageType::Data,
        .data = chunk,
    };

    if (k_msgq_put(&_rx_msgq, &message, K_NO_WAIT) != 0)
    {
        _hwa.status("WebUSB DFU RX buffer overflow");
        queue_control(RxMessageType::Reset);

        return false;
    }

    return true;
}

void webusb::WebUsb::handle_connection_state(const bool connected)
{
    _connected    = connected;
    _accepting_rx = connected;

    queue_control(connected ? RxMessageType::Connect : RxMessageType::Disconnect);
}

void webusb::WebUsb::queue_control(const RxMessageType type)
{
    const RxMessage message = {
        .type = type,
    };

    k_msgq_put(&_rx_msgq, &message, K_FOREVER);
}

void webusb::WebUsb::process_rx()
{
    while (_initialized)
    {
        RxMessage message = {};

        if (k_msgq_get(&_rx_msgq, &message, K_FOREVER) != 0)
        {
            continue;
        }

        switch (message.type)
        {
        case RxMessageType::Data:
        {
            const auto data     = message.data.data;
            bool       released = false;

            if ((_rx_size + data.size()) > _rx_buffer.size())
            {
                _hwa.status("WebUSB DFU RX buffer overflow");
                _hwa.release_rx_buffer(message.data);
                reset(true);
                _accepting_rx = _connected.load();
                break;
            }

            std::copy(data.begin(), data.end(), _rx_buffer.begin() + static_cast<ptrdiff_t>(_rx_size));
            _rx_size += data.size();

            while (_rx_size != 0)
            {
                const auto pending = std::span<const uint8_t>(_rx_buffer.data(), _rx_size);
                const auto info    = opendeck::common::dfu::upload::Upload::frame_info(pending);

                if (!info)
                {
                    _hwa.status("Invalid WebUSB DFU frame");
                    _hwa.release_rx_buffer(message.data);
                    released = true;
                    reset(true);
                    _accepting_rx = _connected.load();
                    break;
                }

                if (!info->complete)
                {
                    break;
                }

                const auto response = _firmware_upload.handle(pending.first(info->size));

                if (!response)
                {
                    _hwa.status("Unsupported WebUSB DFU command");
                    _hwa.release_rx_buffer(message.data);
                    released = true;
                    reset(true);
                    _accepting_rx = _connected.load();
                    break;
                }

                if (response->response.status != opendeck::common::dfu::upload::Status::Ok)
                {
                    _hwa.status("WebUSB DFU command failed");
                    _hwa.release_rx_buffer(message.data);
                    released = true;
                    reset(true);
                    _accepting_rx = _connected.load();
                    break;
                }

                const size_t remaining = _rx_size - info->size;

                if (remaining != 0)
                {
                    std::memmove(_rx_buffer.data(), _rx_buffer.data() + info->size, remaining);
                }

                _rx_size = remaining;
            }

            if (!released)
            {
                _hwa.release_rx_buffer(message.data);
            }
        }
        break;

        case RxMessageType::Connect:
        {
            reset(false);
            _connected    = true;
            _accepting_rx = true;
            _hwa.status("WebUSB connected");
        }
        break;

        case RxMessageType::Disconnect:
        {
            reset(true);
            _connected    = false;
            _accepting_rx = false;
        }
        break;

        case RxMessageType::Reset:
        {
            reset(false);
            _accepting_rx = _connected.load();
        }
        break;

        case RxMessageType::Stop:
        {
            reset(true);
            _initialized  = false;
            _connected    = false;
            _accepting_rx = false;
            return;
        }
        break;
        }
    }
}

void webusb::WebUsb::reset(const bool drain_queue)
{
    _firmware_upload.abort();
    _rx_size = 0;

    if (!drain_queue)
    {
        return;
    }

    RxMessage message       = {};
    size_t    control_count = 0;

    while (k_msgq_get(&_rx_msgq, &message, K_NO_WAIT) == 0)
    {
        if (message.type == RxMessageType::Data)
        {
            _hwa.release_rx_buffer(message.data);
        }
        else if (control_count < _control_buffer.size())
        {
            _control_buffer[control_count++] = message.type;
        }
    }

    for (size_t i = 0; i < control_count; i++)
    {
        queue_control(_control_buffer[i]);
    }
}

void webusb::WebUsb::stop_rx()
{
    _connected    = false;
    _accepting_rx = false;

    const RxMessage message = {
        .type = RxMessageType::Stop,
    };

    k_msgq_put(&_rx_msgq, &message, K_FOREVER);
}
