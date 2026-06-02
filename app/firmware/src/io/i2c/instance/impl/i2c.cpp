/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/io/i2c/instance/impl/i2c.h"
#include "firmware/src/util/thread_sleep.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

using namespace opendeck::firmware::io::i2c;

namespace
{
    LOG_MODULE_REGISTER(i2c, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

I2c::~I2c()
{
    shutdown();
    peripherals.clear();
}

I2c::I2c(HwaBase& hwa)
    : _hwa(hwa)
    , _thread([&]()
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
    if (!_hwa.init())
    {
        LOG_WRN("I2C backend init failed");
        return false;
    }

    LOG_INF("I2C backend initialized, registered peripherals=%u", static_cast<unsigned int>(peripherals.size()));

    const auto now_ms = k_uptime_get();

    for (auto& peripheral : peripherals)
    {
        update_peripheral(peripheral, now_ms);
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
    peripherals.push_back({
        .instance = instance,
    });
}

std::optional<size_t> I2c::find_address(Peripheral& peripheral)
{
    size_t address_index = 0;

    for (const auto address : peripheral.i2c_addresses())
    {
        if (_hwa.device_available(address))
        {
            return address_index;
        }

        address_index++;
    }

    return std::nullopt;
}

uint8_t I2c::address_at(Peripheral& peripheral, size_t address_index)
{
    return peripheral.i2c_addresses()[address_index];
}

void I2c::update_peripherals()
{
    if (is_frozen())
    {
        return;
    }

    const auto now_ms = k_uptime_get();

    for (auto& peripheral : peripherals)
    {
        update_peripheral(peripheral, now_ms);
    }
}

void I2c::update_peripheral(PeripheralState& state, int64_t now_ms)
{
    auto* peripheral = state.instance;

    if (peripheral == nullptr)
    {
        return;
    }

    if (now_ms >= state.next_probe_ms)
    {
        state.next_probe_ms = now_ms + DEVICE_PROBE_INTERVAL_MS;

        if (!state.initialized)
        {
            const auto address_index = find_address(*peripheral);

            if (!address_index.has_value())
            {
                LOG_DBG("I2C peripheral %s not found", peripheral->name().data());
                return;
            }

            state.address_index = address_index.value();
            state.initialized   = peripheral->init(state.address_index);

            if (!state.initialized)
            {
                peripheral->deinit();
                return;
            }

            LOG_INF("I2C peripheral %s initialized at address 0x%02x",
                    peripheral->name().data(),
                    address_at(*peripheral, state.address_index));
            return;
        }
    }

    if (state.initialized && !peripheral->update())
    {
        LOG_WRN("I2C peripheral %s update failed, deinitializing", peripheral->name().data());
        peripheral->deinit();
        state.initialized = false;
    }
}
