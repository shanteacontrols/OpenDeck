/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "../driver_base.h"
#include "count.h"

#include "zlibs/utils/misc/bit.h"

#include <array>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

namespace opendeck::io::leds
{
    /**
     * @brief LED driver for an 8x8 matrix controlled by a MAX7219.
     */
    class Driver : public DriverBase
    {
        public:
        /**
         * @brief Configures the control GPIOs and initializes the MAX7219.
         */
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

            send_command(MAX7219_REG_DISPLAYTEST, DISPLAY_TEST_DISABLED);
            send_command(MAX7219_REG_SCANLIMIT, MAX7219_SCAN_LIMIT);
            send_command(MAX7219_REG_DECODEMODE, DECODE_MODE_DISABLED);

            for (uint8_t col = 0; col < MATRIX_SIZE; col++)
            {
                send_command(static_cast<uint8_t>(col + MAX7219_COLUMN_OFFSET), DISPLAY_COLUMN_RESET);
            }

            send_command(MAX7219_REG_INTENSITY, DEFAULT_INTENSITY);
            send_command(MAX7219_REG_SHUTDOWN, DISPLAY_ENABLED);
        }

        /**
         * @brief Flushes no additional state because writes are pushed immediately.
         */
        void update() override
        {
        }

        /**
         * @brief Sets one matrix pixel and updates the corresponding MAX7219 column.
         *
         * @param index Output index to update.
         * @param brightness Brightness value to apply.
         */
        void set_state(size_t index, Brightness brightness) override
        {
            const auto column = static_cast<uint8_t>(index % MATRIX_SIZE);
            const auto row    = static_cast<uint8_t>(index / MATRIX_SIZE);

            zlibs::utils::misc::bit_write(_columns[column], row, brightness != Brightness::Off);
            send_command(static_cast<uint8_t>(column + MAX7219_COLUMN_OFFSET), _columns[column]);
        }

        /**
         * @brief Maps a physical output index to the corresponding RGB LED index.
         *
         * @param index Output index to map.
         *
         * @return RGB LED index corresponding to the output.
         */
        size_t rgb_from_output(size_t index) override
        {
            const auto    row      = static_cast<uint8_t>(index / MATRIX_SIZE);
            const auto    mod      = static_cast<uint8_t>(row % RGB_COMPONENT_COUNT);
            const uint8_t base_row = row - mod;
            const auto    column   = static_cast<uint8_t>(index % MATRIX_SIZE);
            const auto    result =
                static_cast<uint8_t>(((base_row * MATRIX_SIZE) / RGB_COMPONENT_COUNT) + column);
            return result < rgb_led_count() ? result : (rgb_led_count() ? rgb_led_count() - 1 : 0);
        }

        /**
         * @brief Maps an RGB LED index and component to a physical output index.
         *
         * @param index RGB LED index to map.
         * @param component RGB component to map.
         *
         * @return Physical output index corresponding to the RGB component.
         */
        size_t rgb_component_from_rgb(size_t index, RgbComponent component) override
        {
            const auto column  = static_cast<uint8_t>(index % MATRIX_SIZE);
            const auto row     = static_cast<uint8_t>((index / MATRIX_SIZE) * RGB_COMPONENT_COUNT);
            const auto address = static_cast<uint8_t>(column + MATRIX_SIZE * row);
            return address + MATRIX_SIZE * static_cast<uint8_t>(component);
        }

        private:
        static constexpr uint8_t MAX7219_REG_DECODEMODE  = 0x9;
        static constexpr uint8_t MAX7219_REG_INTENSITY   = 0xA;
        static constexpr uint8_t MAX7219_REG_SCANLIMIT   = 0xB;
        static constexpr uint8_t MAX7219_REG_SHUTDOWN    = 0xC;
        static constexpr uint8_t MAX7219_REG_DISPLAYTEST = 0xF;
        static constexpr uint8_t MATRIX_SIZE             = 8;
        static constexpr uint8_t MAX7219_SCAN_LIMIT      = MATRIX_SIZE - 1;
        static constexpr uint8_t MAX7219_COLUMN_OFFSET   = 1;
        static constexpr uint8_t DISPLAY_TEST_DISABLED   = 0;
        static constexpr uint8_t DECODE_MODE_DISABLED    = 0;
        static constexpr uint8_t DISPLAY_COLUMN_RESET    = 0;
        static constexpr uint8_t DEFAULT_INTENSITY       = 10;
        static constexpr uint8_t DISPLAY_ENABLED         = 1;
        static constexpr uint8_t RGB_COMPONENT_COUNT     = 3;
        static constexpr int     LAST_DATA_BIT           = 7;

        gpio_dt_spec                     _data    = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_leds), data_gpios);
        gpio_dt_spec                     _clock   = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_leds), clock_gpios);
        gpio_dt_spec                     _latch   = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_leds), latch_gpios);
        std::array<uint8_t, MATRIX_SIZE> _columns = {};

        /**
         * @brief Shifts one byte to the MAX7219 serial interface.
         *
         * @param byte Byte to transmit.
         */
        void send_byte(uint8_t byte)
        {
            for (int bit = LAST_DATA_BIT; bit >= 0; bit--)
            {
                gpio_pin_set_dt(&_data, zlibs::utils::misc::bit_read(byte, static_cast<size_t>(bit)));
                gpio_pin_set_dt(&_clock, 0);
                gpio_pin_set_dt(&_clock, 1);
            }
        }

        /**
         * @brief Writes one register-value pair to the MAX7219.
         *
         * @param reg Register address.
         * @param data Register value.
         */
        void send_command(uint8_t reg, uint8_t data)
        {
            gpio_pin_set_dt(&_latch, 0);
            send_byte(reg);
            send_byte(data);
            gpio_pin_set_dt(&_latch, 1);
        }

        /**
         * @brief Returns the number of complete RGB LEDs exposed by the matrix.
         *
         * @return Number of RGB LEDs.
         */
        static constexpr size_t rgb_led_count()
        {
            return (MATRIX_SIZE * MATRIX_SIZE) / RGB_COMPONENT_COUNT;
        }
    };
}    // namespace opendeck::io::leds
