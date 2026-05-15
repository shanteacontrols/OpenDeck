/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/io/indicators/indicators.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

using namespace opendeck;
using namespace opendeck::io::indicators;

namespace
{
    LOG_MODULE_REGISTER(indicators, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

Indicators::Indicators(Hwa& hwa)
    : _hwa(hwa)
    , _usb_in_off_work([this]()
                       {
                           set_idle(Type::UsbIn);
                       })
    , _usb_out_off_work([this]()
                        {
                            set_idle(Type::UsbOut);
                        })
    , _din_in_off_work([this]()
                       {
                           set_idle(Type::DinIn);
                       })
    , _din_out_off_work([this]()
                        {
                            set_idle(Type::DinOut);
                        })
    , _ble_in_off_work([this]()
                       {
                           set_idle(Type::BleIn);
                       })
    , _ble_out_off_work([this]()
                        {
                            set_idle(Type::BleOut);
                        })
    , _network_in_off_work([this]()
                           {
                               set_idle(Type::NetworkIn);
                           })
    , _network_out_off_work([this]()
                            {
                                set_idle(Type::NetworkOut);
                            })
{
    signaling::subscribe<signaling::TrafficSignal>(
        [this](const signaling::TrafficSignal& signal)
        {
            on_traffic(signal);
        });

    signaling::subscribe<signaling::NetworkIdentitySignal>(
        [this](const signaling::NetworkIdentitySignal& identity)
        {
            on_network_identity(identity);
        });

    signaling::subscribe<signaling::UsbUmpBurstSignal>(
        [this]([[maybe_unused]] const signaling::UsbUmpBurstSignal& signal)
        {
            if (_factory_reset_in_progress)
            {
                return;
            }

            set_active(Type::UsbOut);
            schedule_idle(Type::UsbOut);
        });

    signaling::subscribe<signaling::SystemSignal>(
        [this](const signaling::SystemSignal& signal)
        {
            switch (signal.system_event)
            {
            case signaling::SystemEvent::InitComplete:
            {
                indicate_startup();
            }
            break;

            case signaling::SystemEvent::ConfigurationSessionOpened:
            {
                _invert = true;
                cancel_idle_work();
                _hwa.on(Type::TrafficAll);
            }
            break;

            case signaling::SystemEvent::ConfigurationSessionClosed:
            {
                _invert = false;
                cancel_idle_work();
                _hwa.off(Type::TrafficAll);
                apply_network_up_state();
            }
            break;

            default:
                break;
            }
        });
}

Indicators::~Indicators()
{
    shutdown();
}

bool Indicators::init()
{
    if (!_hwa.init())
    {
        return false;
    }

    _hwa.off(Type::All);
    apply_network_up_state();
    return true;
}

void Indicators::deinit()
{
    shutdown();
}

void Indicators::shutdown()
{
    cancel_idle_work();

    _hwa.off(Type::All);
}

void Indicators::on_traffic(const signaling::TrafficSignal& signal)
{
    if (_factory_reset_in_progress)
    {
        return;
    }

    const auto type = indicator_type(signal.transport, signal.direction);

    set_active(type);
    schedule_idle(type);
}

void Indicators::on_network_identity(const signaling::NetworkIdentitySignal& identity)
{
    _network_up = !identity.ipv4_address().empty();
    apply_network_up_state();
}

void Indicators::set_idle(Type type)
{
    if (_invert)
    {
        _hwa.on(type);
    }
    else
    {
        _hwa.off(type);
    }
}

void Indicators::set_active(Type type)
{
    if (_invert)
    {
        _hwa.off(type);
    }
    else
    {
        _hwa.on(type);
    }
}

void Indicators::schedule_idle(Type type)
{
    switch (type)
    {
    case Type::UsbIn:
    {
        _usb_in_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
    }
    break;

    case Type::UsbOut:
    {
        _usb_out_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
    }
    break;

    case Type::DinIn:
    {
        _din_in_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
    }
    break;

    case Type::DinOut:
    {
        _din_out_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
    }
    break;

    case Type::BleIn:
    {
        _ble_in_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
    }
    break;

    case Type::BleOut:
    {
        _ble_out_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
    }
    break;

    case Type::NetworkIn:
    {
        _network_in_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
    }
    break;

    case Type::NetworkOut:
    {
        _network_out_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
    }
    break;

    case Type::All:
    default:
        break;
    }
}

void Indicators::cancel_idle_work()
{
    _usb_in_off_work.cancel();
    _usb_out_off_work.cancel();
    _din_in_off_work.cancel();
    _din_out_off_work.cancel();
    _ble_in_off_work.cancel();
    _ble_out_off_work.cancel();
    _network_in_off_work.cancel();
    _network_out_off_work.cancel();
}

void Indicators::apply_network_up_state()
{
    if (_factory_reset_in_progress)
    {
        return;
    }

    if (_network_up)
    {
        _hwa.on(Type::NetworkUp);
    }
    else
    {
        _hwa.off(Type::NetworkUp);
    }
}

Type Indicators::indicator_type(signaling::TrafficTransport transport, signaling::SignalDirection direction)
{
    switch (transport)
    {
    case signaling::TrafficTransport::Usb:
        return direction == signaling::SignalDirection::In ? Type::UsbIn : Type::UsbOut;

    case signaling::TrafficTransport::Din:
        return direction == signaling::SignalDirection::In ? Type::DinIn : Type::DinOut;

    case signaling::TrafficTransport::Ble:
        return direction == signaling::SignalDirection::In ? Type::BleIn : Type::BleOut;

    case signaling::TrafficTransport::Network:
        return direction == signaling::SignalDirection::In ? Type::NetworkIn : Type::NetworkOut;

    default:
        return Type::All;
    }
}

void Indicators::set_input_indicators(bool state)
{
    if (state)
    {
        _hwa.on(Type::UsbIn);
        _hwa.on(Type::DinIn);
        _hwa.on(Type::BleIn);
        _hwa.on(Type::NetworkIn);
    }
    else
    {
        _hwa.off(Type::UsbIn);
        _hwa.off(Type::DinIn);
        _hwa.off(Type::BleIn);
        _hwa.off(Type::NetworkIn);
    }
}

void Indicators::set_output_indicators(bool state)
{
    if (state)
    {
        _hwa.on(Type::UsbOut);
        _hwa.on(Type::DinOut);
        _hwa.on(Type::BleOut);
        _hwa.on(Type::NetworkOut);
    }
    else
    {
        _hwa.off(Type::UsbOut);
        _hwa.off(Type::DinOut);
        _hwa.off(Type::BleOut);
        _hwa.off(Type::NetworkOut);
    }
}

void Indicators::indicate_startup()
{
    if (_factory_reset_in_progress)
    {
        return;
    }

    for (size_t flash = 0; flash < STARTUP_INDICATOR_FLASH_COUNT; flash++)
    {
        _hwa.on(Type::TrafficAll);
        k_msleep(STARTUP_INDICATOR_TIMEOUT_MS);
        _hwa.off(Type::TrafficAll);
        k_msleep(STARTUP_INDICATOR_TIMEOUT_MS);
    }
}
