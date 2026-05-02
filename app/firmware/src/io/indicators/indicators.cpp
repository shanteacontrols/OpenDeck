/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "indicators.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

using namespace io::indicators;

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
{
    messaging::subscribe<messaging::MidiTrafficSignal>(
        [this](const messaging::MidiTrafficSignal& signal)
        {
            on_traffic(signal);
        });

    messaging::subscribe<messaging::UsbUmpBurstSignal>(
        [this]([[maybe_unused]] const messaging::UsbUmpBurstSignal& signal)
        {
            if (_factory_reset_in_progress)
            {
                return;
            }

            set_active(Type::UsbOut);
            schedule_idle(Type::UsbOut);
        });

    messaging::subscribe<messaging::SystemSignal>(
        [this](const messaging::SystemSignal& signal)
        {
            switch (signal.system_message)
            {
            case messaging::SystemMessage::InitComplete:
            {
                indicate_startup();
            }
            break;

            case messaging::SystemMessage::ConfigurationSessionOpened:
            {
                _invert = true;
                cancel_idle_work();
                _hwa.on(Type::All);
            }
            break;

            case messaging::SystemMessage::ConfigurationSessionClosed:
            {
                _invert = false;
                cancel_idle_work();
                _hwa.off(Type::All);
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

void Indicators::on_traffic(const messaging::MidiTrafficSignal& signal)
{
    if (_factory_reset_in_progress)
    {
        return;
    }

    const auto type = indicator_type(signal.transport, signal.direction);

    set_active(type);
    schedule_idle(type);
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
        _usb_in_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
        break;

    case Type::UsbOut:
        _usb_out_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
        break;

    case Type::DinIn:
        _din_in_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
        break;

    case Type::DinOut:
        _din_out_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
        break;

    case Type::BleIn:
        _ble_in_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
        break;

    case Type::BleOut:
        _ble_out_off_work.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
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
}

Type Indicators::indicator_type(messaging::MidiTransport transport, messaging::MidiDirection direction)
{
    switch (transport)
    {
    case messaging::MidiTransport::Usb:
        return direction == messaging::MidiDirection::In ? Type::UsbIn : Type::UsbOut;

    case messaging::MidiTransport::Din:
        return direction == messaging::MidiDirection::In ? Type::DinIn : Type::DinOut;

    case messaging::MidiTransport::Ble:
        return direction == messaging::MidiDirection::In ? Type::BleIn : Type::BleOut;

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
    }
    else
    {
        _hwa.off(Type::UsbIn);
        _hwa.off(Type::DinIn);
        _hwa.off(Type::BleIn);
    }
}

void Indicators::set_output_indicators(bool state)
{
    if (state)
    {
        _hwa.on(Type::UsbOut);
        _hwa.on(Type::DinOut);
        _hwa.on(Type::BleOut);
    }
    else
    {
        _hwa.off(Type::UsbOut);
        _hwa.off(Type::DinOut);
        _hwa.off(Type::BleOut);
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
        _hwa.on(Type::All);
        k_msleep(STARTUP_INDICATOR_TIMEOUT_MS);
        _hwa.off(Type::All);
        k_msleep(STARTUP_INDICATOR_TIMEOUT_MS);
    }
}
