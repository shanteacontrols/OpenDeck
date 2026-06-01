/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/util/cinfo/cinfo.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/threads.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

using namespace opendeck;
using namespace opendeck::util;
using namespace opendeck::firmware;

namespace
{
    LOG_MODULE_REGISTER(cinfo, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

ComponentInfo::ComponentInfo()
    : _work([this]()
            {
                process_pending();
            },
            []()
            {
                return threads::SystemWorkqueue::handle();
            })
{
    signaling::subscribe<signaling::MidiIoSignal>(
        [this](const signaling::MidiIoSignal& signal)
        {
            switch (signal.source)
            {
            case signaling::IoEventSource::AnalogSwitch:
            case signaling::IoEventSource::Analog:
            {
                queue(database::Config::Block::Analog, signal.component_index);
            }
            break;

            case signaling::IoEventSource::Switch:
            {
                queue(database::Config::Block::Switches, signal.component_index);
            }
            break;

            case signaling::IoEventSource::Encoder:
            {
                queue(database::Config::Block::Encoders, signal.component_index);
            }
            break;

            case signaling::IoEventSource::TouchscreenSwitch:
            {
                queue(database::Config::Block::Touchscreen, signal.component_index);
            }
            break;

            default:
                break;
            }
        });
}

void ComponentInfo::register_handler(CinfoHandler&& handler)
{
    const zlibs::utils::misc::LockGuard lock(_lock);
    _handler = std::move(handler);
}

void ComponentInfo::queue(database::Config::Block block, size_t index)
{
    const auto block_index = static_cast<size_t>(block);
    const auto now         = k_uptime_get_32();

    {
        const zlibs::utils::misc::LockGuard lock(_lock);

        auto& pending = _pending[block_index];

        if (pending.last_ms.has_value() &&
            (pending.index == index))
        {
            const auto last_ms = pending.last_ms.value();

            if ((now - last_ms) <= SAME_COMPONENT_INFO_TIMEOUT)
            {
                return;
            }
        }

        pending.pending = true;
        pending.index   = index;
        pending.last_ms = now;
    }

    _work.reschedule(0);
}

void ComponentInfo::process_pending()
{
    while (true)
    {
        CinfoHandler handler = nullptr;
        size_t       block   = 0;
        size_t       index   = 0;

        {
            const zlibs::utils::misc::LockGuard lock(_lock);

            for (size_t i = 0; i < _pending.size(); i++)
            {
                if (_pending[i].pending)
                {
                    block               = i;
                    index               = _pending[i].index;
                    _pending[i].pending = false;
                    handler             = _handler;
                    break;
                }
            }
        }

        if (handler == nullptr)
        {
            break;
        }

        handler(block, index);
    }
}
