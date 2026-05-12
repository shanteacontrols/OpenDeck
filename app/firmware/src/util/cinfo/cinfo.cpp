/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cinfo.h"
#include "signaling/signaling.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

using namespace opendeck;
using namespace opendeck::util;

namespace
{
    LOG_MODULE_REGISTER(cinfo, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

ComponentInfo::ComponentInfo()
{
    signaling::subscribe<signaling::MidiIoSignal>(
        [this](const signaling::MidiIoSignal& signal)
        {
            switch (signal.source)
            {
            case signaling::IoEventSource::AnalogSwitch:
            case signaling::IoEventSource::Analog:
            {
                send(database::Config::Block::Analog, signal.component_index);
            }
            break;

            case signaling::IoEventSource::Switch:
            {
                send(database::Config::Block::Switches, signal.component_index);
            }
            break;

            case signaling::IoEventSource::Encoder:
            {
                send(database::Config::Block::Encoders, signal.component_index);
            }
            break;

            case signaling::IoEventSource::TouchscreenSwitch:
            {
                send(database::Config::Block::Touchscreen, signal.component_index);
            }
            break;

            default:
                break;
            }
        });
}

void ComponentInfo::register_handler(CinfoHandler&& handler)
{
    _handler = std::move(handler);
}

void ComponentInfo::send(database::Config::Block block, size_t index)
{
    if ((k_uptime_get_32() - _last_cinfo_msg_time[static_cast<size_t>(block)]) > COMPONENT_INFO_TIMEOUT)
    {
        if (_handler != nullptr)
        {
            _handler(static_cast<size_t>(block), index);
        }

        _last_cinfo_msg_time[static_cast<size_t>(block)] = k_uptime_get_32();
    }
}
