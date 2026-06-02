/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/io/indicators/instance/impl/indicators.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

using namespace opendeck::firmware::io::indicators;
using namespace opendeck::firmware;

namespace
{
    LOG_MODULE_REGISTER(indicators, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

Indicators::Indicators(opendeck::common::io::indicators::Hwa& hwa)
    : _hwa(hwa)
    , _usb_in_off_work([this]()
                       {
                           set_idle(opendeck::common::io::indicators::Type::UsbIn);
                       })
    , _usb_out_off_work([this]()
                        {
                            set_idle(opendeck::common::io::indicators::Type::UsbOut);
                        })
    , _din_in_off_work([this]()
                       {
                           set_idle(opendeck::common::io::indicators::Type::DinIn);
                       })
    , _din_out_off_work([this]()
                        {
                            set_idle(opendeck::common::io::indicators::Type::DinOut);
                        })
    , _ble_in_off_work([this]()
                       {
                           set_idle(opendeck::common::io::indicators::Type::BleIn);
                       })
    , _ble_out_off_work([this]()
                        {
                            set_idle(opendeck::common::io::indicators::Type::BleOut);
                        })
    , _network_in_off_work([this]()
                           {
                               set_idle(opendeck::common::io::indicators::Type::NetworkIn);
                           })
    , _network_out_off_work([this]()
                            {
                                set_idle(opendeck::common::io::indicators::Type::NetworkOut);
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

            set_active(opendeck::common::io::indicators::Type::UsbOut);
            schedule_idle(opendeck::common::io::indicators::Type::UsbOut);
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
                _hwa.on(opendeck::common::io::indicators::Type::TrafficAll);
            }
            break;

            case signaling::SystemEvent::ConfigurationSessionClosed:
            {
                _invert = false;
                cancel_idle_work();
                _hwa.off(opendeck::common::io::indicators::Type::TrafficAll);
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

    _hwa.off(opendeck::common::io::indicators::Type::All);
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

    _hwa.off(opendeck::common::io::indicators::Type::All);
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

void Indicators::set_idle(opendeck::common::io::indicators::Type type)
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

void Indicators::set_active(opendeck::common::io::indicators::Type type)
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

void Indicators::schedule_idle(opendeck::common::io::indicators::Type type)
{
    switch (type)
    {
    case opendeck::common::io::indicators::Type::UsbIn:
    {
        _usb_in_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
    }
    break;

    case opendeck::common::io::indicators::Type::UsbOut:
    {
        _usb_out_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
    }
    break;

    case opendeck::common::io::indicators::Type::DinIn:
    {
        _din_in_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
    }
    break;

    case opendeck::common::io::indicators::Type::DinOut:
    {
        _din_out_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
    }
    break;

    case opendeck::common::io::indicators::Type::BleIn:
    {
        _ble_in_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
    }
    break;

    case opendeck::common::io::indicators::Type::BleOut:
    {
        _ble_out_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
    }
    break;

    case opendeck::common::io::indicators::Type::NetworkIn:
    {
        _network_in_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
    }
    break;

    case opendeck::common::io::indicators::Type::NetworkOut:
    {
        _network_out_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
    }
    break;

    case opendeck::common::io::indicators::Type::All:
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
        _hwa.on(opendeck::common::io::indicators::Type::NetworkUp);
    }
    else
    {
        _hwa.off(opendeck::common::io::indicators::Type::NetworkUp);
    }
}

opendeck::common::io::indicators::Type Indicators::indicator_type(signaling::TrafficTransport transport,
                                                                  signaling::SignalDirection  direction)
{
    switch (transport)
    {
    case signaling::TrafficTransport::Usb:
        return direction == signaling::SignalDirection::In ? opendeck::common::io::indicators::Type::UsbIn : opendeck::common::io::indicators::Type::UsbOut;

    case signaling::TrafficTransport::Din:
        return direction == signaling::SignalDirection::In ? opendeck::common::io::indicators::Type::DinIn : opendeck::common::io::indicators::Type::DinOut;

    case signaling::TrafficTransport::Ble:
        return direction == signaling::SignalDirection::In ? opendeck::common::io::indicators::Type::BleIn : opendeck::common::io::indicators::Type::BleOut;

    case signaling::TrafficTransport::Network:
        return direction == signaling::SignalDirection::In ? opendeck::common::io::indicators::Type::NetworkIn : opendeck::common::io::indicators::Type::NetworkOut;

    default:
        return opendeck::common::io::indicators::Type::All;
    }
}

void Indicators::set_input_indicators(bool state)
{
    if (state)
    {
        _hwa.on(opendeck::common::io::indicators::Type::UsbIn);
        _hwa.on(opendeck::common::io::indicators::Type::DinIn);
        _hwa.on(opendeck::common::io::indicators::Type::BleIn);
        _hwa.on(opendeck::common::io::indicators::Type::NetworkIn);
    }
    else
    {
        _hwa.off(opendeck::common::io::indicators::Type::UsbIn);
        _hwa.off(opendeck::common::io::indicators::Type::DinIn);
        _hwa.off(opendeck::common::io::indicators::Type::BleIn);
        _hwa.off(opendeck::common::io::indicators::Type::NetworkIn);
    }
}

void Indicators::set_output_indicators(bool state)
{
    if (state)
    {
        _hwa.on(opendeck::common::io::indicators::Type::UsbOut);
        _hwa.on(opendeck::common::io::indicators::Type::DinOut);
        _hwa.on(opendeck::common::io::indicators::Type::BleOut);
        _hwa.on(opendeck::common::io::indicators::Type::NetworkOut);
    }
    else
    {
        _hwa.off(opendeck::common::io::indicators::Type::UsbOut);
        _hwa.off(opendeck::common::io::indicators::Type::DinOut);
        _hwa.off(opendeck::common::io::indicators::Type::BleOut);
        _hwa.off(opendeck::common::io::indicators::Type::NetworkOut);
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
        _hwa.on(opendeck::common::io::indicators::Type::TrafficAll);
        k_msleep(STARTUP_INDICATOR_TIMEOUT_MS);
        _hwa.off(opendeck::common::io::indicators::Type::TrafficAll);
        k_msleep(STARTUP_INDICATOR_TIMEOUT_MS);
    }
}
