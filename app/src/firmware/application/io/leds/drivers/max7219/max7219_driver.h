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

#pragma once

#include "../driver_base.h"
#include "count.h"

#include "zlibs/utils/misc/bit.h"

#include <array>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

namespace io::leds
{
    class Driver : public DriverBase
    {
        public:
        Driver()
        {
            if (device_is_ready(_data.port))
            {
                gpio_pin_configure_dt(&_data, GPIO_OUTPUT_INACTIVE);
            }

            if (device_is_ready(_clock.port))
            {
                gpio_pin_configure_dt(&_clock, GPIO_OUTPUT_INACTIVE);
            }

            if (device_is_ready(_latch.port))
            {
                gpio_pin_configure_dt(&_latch, GPIO_OUTPUT_INACTIVE);
            }

            sendCommand(MAX7219_REG_DISPLAYTEST, 0);
            sendCommand(MAX7219_REG_SCANLIMIT, 7);
            sendCommand(MAX7219_REG_DECODEMODE, 0);

            for (uint8_t col = 0; col < 8; col++)
            {
                sendCommand(col + 1, 0);
            }

            sendCommand(MAX7219_REG_INTENSITY, 10);
            sendCommand(MAX7219_REG_SHUTDOWN, 1);
        }

        void update() override
        {
        }

        void setState(size_t index, brightness_t brightness) override
        {
            const uint8_t column = index % 8;
            const uint8_t row    = index / 8;

            zlibs::utils::misc::bit_write(_columns[column], row, brightness != brightness_t::OFF);
            sendCommand(column + 1, _columns[column]);
        }

        size_t rgbFromOutput(size_t index) override
        {
            const uint8_t row      = index / 8;
            const uint8_t mod      = row % 3;
            const uint8_t base_row = row - mod;
            const uint8_t column   = index % 8;
            const uint8_t result   = (base_row * 8) / 3 + column;
            return result < rgbLedCount() ? result : (rgbLedCount() ? rgbLedCount() - 1 : 0);
        }

        size_t rgbComponentFromRgb(size_t index, rgbComponent_t component) override
        {
            const uint8_t column  = index % 8;
            const uint8_t row     = (index / 8) * 3;
            const uint8_t address = column + 8 * row;
            return address + 8 * static_cast<uint8_t>(component);
        }

        private:
        static constexpr uint8_t MAX7219_REG_DECODEMODE  = 0x9;
        static constexpr uint8_t MAX7219_REG_INTENSITY   = 0xA;
        static constexpr uint8_t MAX7219_REG_SCANLIMIT   = 0xB;
        static constexpr uint8_t MAX7219_REG_SHUTDOWN    = 0xC;
        static constexpr uint8_t MAX7219_REG_DISPLAYTEST = 0xF;

        gpio_dt_spec           _data    = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_leds), data_gpios);
        gpio_dt_spec           _clock   = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_leds), clock_gpios);
        gpio_dt_spec           _latch   = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_leds), latch_gpios);
        std::array<uint8_t, 8> _columns = {};

        void sendByte(uint8_t byte)
        {
            for (int bit = 7; bit >= 0; bit--)
            {
                gpio_pin_set_dt(&_data, zlibs::utils::misc::bit_read(byte, static_cast<size_t>(bit)));
                gpio_pin_set_dt(&_clock, 0);
                gpio_pin_set_dt(&_clock, 1);
            }
        }

        void sendCommand(uint8_t reg, uint8_t data)
        {
            gpio_pin_set_dt(&_latch, 0);
            sendByte(reg);
            sendByte(data);
            gpio_pin_set_dt(&_latch, 1);
        }

        static constexpr size_t rgbLedCount()
        {
            return 64 / 3;
        }
    };
}    // namespace io::leds
