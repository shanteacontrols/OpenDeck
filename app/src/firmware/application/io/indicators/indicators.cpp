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
    , _usbInOffWork([this]()
                    {
                        _hwa.off(type_t::USB_IN);
                    })
    , _usbOutOffWork([this]()
                     {
                         _hwa.off(type_t::USB_OUT);
                     })
    , _dinInOffWork([this]()
                    {
                        _hwa.off(type_t::DIN_IN);
                    })
    , _dinOutOffWork([this]()
                     {
                         _hwa.off(type_t::DIN_OUT);
                     })
    , _bleInOffWork([this]()
                    {
                        _hwa.off(type_t::BLE_IN);
                    })
    , _bleOutOffWork([this]()
                     {
                         _hwa.off(type_t::BLE_OUT);
                     })
    , _factoryResetBlinkWork([this]()
                             {
                                 toggleFactoryResetIndication();
                             })
{
    messaging::subscribe<messaging::MidiTrafficSignal>(
        [this](const messaging::MidiTrafficSignal& signal)
        {
            onTraffic(signal);
        });

    messaging::subscribe<messaging::SystemSignal>(
        [this](const messaging::SystemSignal& signal)
        {
            switch (signal.systemMessage)
            {
            case messaging::systemMessage_t::FACTORY_RESET_START:
                startFactoryResetIndication();
                break;

            case messaging::systemMessage_t::FACTORY_RESET_END:
                stopFactoryResetIndication();
                break;

            case messaging::systemMessage_t::INIT_COMPLETE:
                indicateStartup();
                break;

            default:
                break;
            }
        });
}

bool Indicators::init()
{
    if (!_hwa.init())
    {
        return false;
    }

    _hwa.off(type_t::ALL);
    return true;
}

void Indicators::updateSingle(size_t index, bool forceRefresh)
{
}

void Indicators::updateAll(bool forceRefresh)
{
}

size_t Indicators::maxComponentUpdateIndex()
{
    return 0;
}

void Indicators::onTraffic(const messaging::MidiTrafficSignal& signal)
{
    if (_factoryResetInProgress)
    {
        return;
    }

    const auto type = indicatorType(signal.transport, signal.direction);

    _hwa.on(type);
    scheduleOff(type);
}

void Indicators::scheduleOff(type_t type)
{
    switch (type)
    {
    case type_t::USB_IN:
        _usbInOffWork.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
        break;

    case type_t::USB_OUT:
        _usbOutOffWork.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
        break;

    case type_t::DIN_IN:
        _dinInOffWork.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
        break;

    case type_t::DIN_OUT:
        _dinOutOffWork.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
        break;

    case type_t::BLE_IN:
        _bleInOffWork.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
        break;

    case type_t::BLE_OUT:
        _bleOutOffWork.reschedule(TRAFFIC_INDICATOR_TIMEOUT_MS);
        break;

    case type_t::ALL:
    default:
        break;
    }
}

type_t Indicators::indicatorType(messaging::MidiTransport transport, messaging::MidiDirection direction)
{
    switch (transport)
    {
    case messaging::MidiTransport::Usb:
        return direction == messaging::MidiDirection::In ? type_t::USB_IN : type_t::USB_OUT;

    case messaging::MidiTransport::Din:
        return direction == messaging::MidiDirection::In ? type_t::DIN_IN : type_t::DIN_OUT;

    case messaging::MidiTransport::Ble:
        return direction == messaging::MidiDirection::In ? type_t::BLE_IN : type_t::BLE_OUT;

    default:
        return type_t::ALL;
    }
}

void Indicators::setInputIndicators(bool state)
{
    if (state)
    {
        _hwa.on(type_t::USB_IN);
        _hwa.on(type_t::DIN_IN);
        _hwa.on(type_t::BLE_IN);
    }
    else
    {
        _hwa.off(type_t::USB_IN);
        _hwa.off(type_t::DIN_IN);
        _hwa.off(type_t::BLE_IN);
    }
}

void Indicators::setOutputIndicators(bool state)
{
    if (state)
    {
        _hwa.on(type_t::USB_OUT);
        _hwa.on(type_t::DIN_OUT);
        _hwa.on(type_t::BLE_OUT);
    }
    else
    {
        _hwa.off(type_t::USB_OUT);
        _hwa.off(type_t::DIN_OUT);
        _hwa.off(type_t::BLE_OUT);
    }
}

void Indicators::startFactoryResetIndication()
{
    _hwa.off(type_t::ALL);
    _factoryResetInProgress  = true;
    _factoryResetIndicatorIn = true;
    setInputIndicators(true);
    setOutputIndicators(false);
    _factoryResetBlinkWork.reschedule(FACTORY_RESET_INDICATOR_TIMEOUT_MS);
}

void Indicators::stopFactoryResetIndication()
{
    _factoryResetInProgress = false;
    _hwa.off(type_t::ALL);
}

void Indicators::indicateStartup()
{
    if (_factoryResetInProgress)
    {
        return;
    }

    for (size_t flash = 0; flash < STARTUP_INDICATOR_FLASH_COUNT; flash++)
    {
        _hwa.on(type_t::ALL);
        k_msleep(STARTUP_INDICATOR_TIMEOUT_MS);
        _hwa.off(type_t::ALL);
        k_msleep(STARTUP_INDICATOR_TIMEOUT_MS);
    }
}

void Indicators::toggleFactoryResetIndication()
{
    if (!_factoryResetInProgress)
    {
        return;
    }

    _factoryResetIndicatorIn = !_factoryResetIndicatorIn;

    if (_factoryResetIndicatorIn)
    {
        setInputIndicators(true);
        setOutputIndicators(false);
    }
    else
    {
        setInputIndicators(false);
        setOutputIndicators(true);
    }

    _factoryResetBlinkWork.reschedule(FACTORY_RESET_INDICATOR_TIMEOUT_MS);
}
