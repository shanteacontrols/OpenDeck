/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

namespace opendeck::io::indicators
{
    /**
     * @brief Hardware-backed indicator backend that drives GPIO outputs.
     */
    class HwaHw : public Hwa
    {
        public:
        /**
         * @brief Configures all available indicator GPIOs and turns them off.
         *
         * @return `true` if configuration succeeded, otherwise `false`.
         */
        bool init() override
        {
            if (!configure_all())
            {
                return false;
            }

            off(Type::All);
            return true;
        }

        /**
         * @brief Turns on the selected indicator output.
         *
         * @param type Indicator to enable.
         */
        void on(Type type) override
        {
            set(type, true);
        }

        /**
         * @brief Turns off the selected indicator output.
         *
         * @param type Indicator to disable.
         */
        void off(Type type) override
        {
            set(type, false);
        }

        private:
        static constexpr bool HAS_USB_IN  = DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_midi_indicators), usb_in_gpios);
        static constexpr bool HAS_USB_OUT = DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_midi_indicators), usb_out_gpios);
        static constexpr bool HAS_DIN_IN  = DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_midi_indicators), din_in_gpios);
        static constexpr bool HAS_DIN_OUT = DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_midi_indicators), din_out_gpios);
        static constexpr bool HAS_BLE_IN  = DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_midi_indicators), ble_in_gpios);
        static constexpr bool HAS_BLE_OUT = DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_midi_indicators), ble_out_gpios);

        const gpio_dt_spec _usb_in  = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(opendeck_midi_indicators), usb_in_gpios, {});
        const gpio_dt_spec _usb_out = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(opendeck_midi_indicators), usb_out_gpios, {});
        const gpio_dt_spec _din_in  = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(opendeck_midi_indicators), din_in_gpios, {});
        const gpio_dt_spec _din_out = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(opendeck_midi_indicators), din_out_gpios, {});
        const gpio_dt_spec _ble_in  = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(opendeck_midi_indicators), ble_in_gpios, {});
        const gpio_dt_spec _ble_out = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(opendeck_midi_indicators), ble_out_gpios, {});

        /**
         * @brief Configures all indicator GPIOs present in the devicetree.
         *
         * @return `true` if every configured GPIO was initialized, otherwise `false`.
         */
        bool configure_all()
        {
            return configure_optional<Type::UsbIn>() &&
                   configure_optional<Type::UsbOut>() &&
                   configure_optional<Type::DinIn>() &&
                   configure_optional<Type::DinOut>() &&
                   configure_optional<Type::BleIn>() &&
                   configure_optional<Type::BleOut>();
        }

        /**
         * @brief Configures one optional indicator GPIO when it exists.
         *
         * @tparam Type Indicator type to configure.
         *
         * @return `true` if the GPIO is absent or configured successfully, otherwise `false`.
         */
        template<Type Type>
        bool configure_optional()
        {
            if constexpr (Type == Type::UsbIn)
            {
                if constexpr (HAS_USB_IN)
                {
                    return configure(_usb_in);
                }
            }
            else if constexpr (Type == Type::UsbOut)
            {
                if constexpr (HAS_USB_OUT)
                {
                    return configure(_usb_out);
                }
            }
            else if constexpr (Type == Type::DinIn)
            {
                if constexpr (HAS_DIN_IN)
                {
                    return configure(_din_in);
                }
            }
            else if constexpr (Type == Type::DinOut)
            {
                if constexpr (HAS_DIN_OUT)
                {
                    return configure(_din_out);
                }
            }
            else if constexpr (Type == Type::BleIn)
            {
                if constexpr (HAS_BLE_IN)
                {
                    return configure(_ble_in);
                }
            }
            else if constexpr (Type == Type::BleOut)
            {
                if constexpr (HAS_BLE_OUT)
                {
                    return configure(_ble_out);
                }
            }

            return true;
        }

        /**
         * @brief Configures one indicator GPIO for output.
         *
         * @param spec GPIO specification to configure.
         *
         * @return `true` if the GPIO is ready and configured, otherwise `false`.
         */
        bool configure(const gpio_dt_spec& spec)
        {
            if (!gpio_is_ready_dt(&spec))
            {
                return false;
            }

            return gpio_pin_configure_dt(&spec, GPIO_OUTPUT_INACTIVE) == 0;
        }

        /**
         * @brief Sets one optional indicator GPIO when it exists.
         *
         * @tparam Type Indicator type to update.
         *
         * @param state `true` to drive the GPIO active, `false` to drive it inactive.
         */
        template<Type Type>
        void set_optional(bool state)
        {
            if constexpr (Type == Type::UsbIn)
            {
                if constexpr (HAS_USB_IN)
                {
                    gpio_pin_set_dt(&_usb_in, state);
                }
            }
            else if constexpr (Type == Type::UsbOut)
            {
                if constexpr (HAS_USB_OUT)
                {
                    gpio_pin_set_dt(&_usb_out, state);
                }
            }
            else if constexpr (Type == Type::DinIn)
            {
                if constexpr (HAS_DIN_IN)
                {
                    gpio_pin_set_dt(&_din_in, state);
                }
            }
            else if constexpr (Type == Type::DinOut)
            {
                if constexpr (HAS_DIN_OUT)
                {
                    gpio_pin_set_dt(&_din_out, state);
                }
            }
            else if constexpr (Type == Type::BleIn)
            {
                if constexpr (HAS_BLE_IN)
                {
                    gpio_pin_set_dt(&_ble_in, state);
                }
            }
            else if constexpr (Type == Type::BleOut)
            {
                if constexpr (HAS_BLE_OUT)
                {
                    gpio_pin_set_dt(&_ble_out, state);
                }
            }
        }

        /**
         * @brief Applies the requested state to one indicator or to all of them.
         *
         * @param type Indicator selector to update.
         * @param state `true` to enable, `false` to disable.
         */
        void set(Type type, bool state)
        {
            switch (type)
            {
            case Type::UsbIn:
            {
                set_optional<Type::UsbIn>(state);
            }
            break;

            case Type::UsbOut:
            {
                set_optional<Type::UsbOut>(state);
            }
            break;

            case Type::DinIn:
            {
                set_optional<Type::DinIn>(state);
            }
            break;

            case Type::DinOut:
            {
                set_optional<Type::DinOut>(state);
            }
            break;

            case Type::BleIn:
            {
                set_optional<Type::BleIn>(state);
            }
            break;

            case Type::BleOut:
            {
                set_optional<Type::BleOut>(state);
            }
            break;

            case Type::All:
            {
                set_optional<Type::UsbIn>(state);
                set_optional<Type::UsbOut>(state);
                set_optional<Type::DinIn>(state);
                set_optional<Type::DinOut>(state);
                set_optional<Type::BleIn>(state);
                set_optional<Type::BleOut>(state);
            }
            break;

            default:
                break;
            }
        }
    };
}    // namespace opendeck::io::indicators
