/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_I2C

#include "i2c.h"
#include "util/thread_sleep.h"

#include <zephyr/logging/log.h>

using namespace opendeck::io::i2c;

namespace
{
    LOG_MODULE_REGISTER(i2c, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

I2c::~I2c()
{
    shutdown();

    for (size_t i = 0; i < I2c::MAX_PERIPHERALS; i++)
    {
        peripherals.at(i) = nullptr;
    }

    peripheral_counter = 0;
}

I2c::I2c()
    : _thread([&]()
              {
                  while (1)
                  {
                      wait_until_running();
                      update_peripherals();
                      util::thread_sleep(CONFIG_I2C_THREAD_SLEEP_MS);
                  }
              })
{
}

bool I2c::init()
{
    for (size_t i = 0; i < peripheral_counter; i++)
    {
        if (peripherals.at(i) != nullptr)
        {
            if (!peripherals.at(i)->init())
            {
                return false;
            }
        }
    }

    _thread.run();

    return true;
}

void I2c::deinit()
{
    shutdown();
}

void I2c::shutdown()
{
    _thread.destroy();
}

void I2c::register_peripheral(Peripheral* instance)
{
    if (peripheral_counter >= MAX_PERIPHERALS)
    {
        return;
    }

    peripherals[peripheral_counter++] = instance;
}

void I2c::update_peripherals()
{
    if (is_frozen())
    {
        return;
    }

    for (size_t i = 0; i < peripheral_counter; i++)
    {
        if (peripherals.at(i) != nullptr)
        {
            peripherals.at(i)->update();
        }
    }
}

#endif
