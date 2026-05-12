/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "digital.h"
#include "util/thread_sleep.h"

using namespace opendeck::io::digital;

Digital::Digital(drivers::DriverBase&    driver,
                 FrameStore&             frame_store,
                 io::switches::Switches& switches,
                 io::encoders::Encoders& encoders)
    : _driver(driver)
    , _frame_store(frame_store)
    , _switches(switches)
    , _encoders(encoders)
    , _thread([&]()
              {
                  while (1)
                  {
                      wait_until_running();

                      if (is_frozen())
                      {
                          util::thread_sleep(CONFIG_DIGITAL_THREAD_SLEEP_MS);
                          continue;
                      }

                      auto frame = _driver.read();

                      if (frame.has_value())
                      {
                          _frame_store.set_frame(frame.value());
                          _switches.process_state_changes();
                          _encoders.process_state_changes();
                          _frame_store.clear();
                      }

                      util::thread_sleep(CONFIG_DIGITAL_THREAD_SLEEP_MS);
                  }
              })
{
}

Digital::~Digital()
{
    shutdown();
}

bool Digital::init()
{
    auto ret = _driver.init() && _switches.init() && _encoders.init();

    if (!ret)
    {
        return false;
    }

    _thread.run();

    return true;
}

void Digital::deinit()
{
    shutdown();
}

size_t Digital::refreshable_components() const
{
    return _switches.refreshable_components() + _encoders.refreshable_components();
}

void Digital::force_refresh(size_t start_index, size_t count)
{
    const auto switches_total = _switches.refreshable_components();
    const auto encoders_total = _encoders.refreshable_components();
    auto       offset         = start_index;
    auto       remaining      = count;

    if ((offset < switches_total) && remaining)
    {
        const auto switch_count = std::min(remaining, switches_total - offset);
        _switches.force_refresh(offset, switch_count);
        remaining -= switch_count;
        offset = 0;
    }
    else if (offset >= switches_total)
    {
        offset -= switches_total;
    }

    if ((offset < encoders_total) && remaining)
    {
        const auto encoder_count = std::min(remaining, encoders_total - offset);
        _encoders.force_refresh(offset, encoder_count);
    }
}

void Digital::shutdown()
{
    _thread.destroy();
}
