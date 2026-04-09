/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "digital.h"
#include "util/thread_sleep.h"

using namespace io::digital;

Digital::Digital(drivers::DriverBase&    driver,
                 FrameStore&             frame_store,
                 io::buttons::Buttons&   buttons,
                 io::encoders::Encoders& encoders)
    : _driver(driver)
    , _frame_store(frame_store)
    , _buttons(buttons)
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
                          _buttons.process_state_changes();
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
    auto ret = _driver.init() && _buttons.init() && _encoders.init();

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
    return _buttons.refreshable_components() + _encoders.refreshable_components();
}

void Digital::force_refresh(size_t start_index, size_t count)
{
    const auto buttons_total  = _buttons.refreshable_components();
    const auto encoders_total = _encoders.refreshable_components();
    auto       offset         = start_index;
    auto       remaining      = count;

    if ((offset < buttons_total) && remaining)
    {
        const auto button_count = std::min(remaining, buttons_total - offset);
        _buttons.force_refresh(offset, button_count);
        remaining -= button_count;
        offset = 0;
    }
    else if (offset >= buttons_total)
    {
        offset -= buttons_total;
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
