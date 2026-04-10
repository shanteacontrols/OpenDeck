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

#include "deps.h"

#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

namespace io::indicators
{
    class HwaHw : public Hwa
    {
        public:
        bool init() override
        {
            if (!configureAll())
            {
                return false;
            }

            off(type_t::ALL);
            return true;
        }

        void on(type_t type) override
        {
            set(type, true);
        }

        void off(type_t type) override
        {
            set(type, false);
        }

        private:
        static constexpr bool HAS_USB_IN  = DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_midi_indicators), usb_in_gpios);
        static constexpr bool HAS_USB_OUT = DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_midi_indicators), usb_out_gpios);
        static constexpr bool HAS_DIN_IN  = DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_midi_indicators), din_in_gpios);
        static constexpr bool HAS_DIN_OUT = DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_midi_indicators), din_out_gpios);
        static constexpr bool HAS_BLE_IN  = DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_midi_indicators), ble_in_gpios);
        static constexpr bool HAS_BLE_OUT = DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_midi_indicators), ble_out_gpios);

        const gpio_dt_spec _usbIn  = GPIO_DT_SPEC_GET_OR(DT_CHOSEN(opendeck_midi_indicators), usb_in_gpios, {});
        const gpio_dt_spec _usbOut = GPIO_DT_SPEC_GET_OR(DT_CHOSEN(opendeck_midi_indicators), usb_out_gpios, {});
        const gpio_dt_spec _dinIn  = GPIO_DT_SPEC_GET_OR(DT_CHOSEN(opendeck_midi_indicators), din_in_gpios, {});
        const gpio_dt_spec _dinOut = GPIO_DT_SPEC_GET_OR(DT_CHOSEN(opendeck_midi_indicators), din_out_gpios, {});
        const gpio_dt_spec _bleIn  = GPIO_DT_SPEC_GET_OR(DT_CHOSEN(opendeck_midi_indicators), ble_in_gpios, {});
        const gpio_dt_spec _bleOut = GPIO_DT_SPEC_GET_OR(DT_CHOSEN(opendeck_midi_indicators), ble_out_gpios, {});

        bool configureAll()
        {
            return configureOptional<type_t::USB_IN>() &&
                   configureOptional<type_t::USB_OUT>() &&
                   configureOptional<type_t::DIN_IN>() &&
                   configureOptional<type_t::DIN_OUT>() &&
                   configureOptional<type_t::BLE_IN>() &&
                   configureOptional<type_t::BLE_OUT>();
        }

        template<type_t Type>
        bool configureOptional()
        {
            if constexpr (Type == type_t::USB_IN)
            {
                if constexpr (HAS_USB_IN)
                {
                    return configure(_usbIn);
                }
            }
            else if constexpr (Type == type_t::USB_OUT)
            {
                if constexpr (HAS_USB_OUT)
                {
                    return configure(_usbOut);
                }
            }
            else if constexpr (Type == type_t::DIN_IN)
            {
                if constexpr (HAS_DIN_IN)
                {
                    return configure(_dinIn);
                }
            }
            else if constexpr (Type == type_t::DIN_OUT)
            {
                if constexpr (HAS_DIN_OUT)
                {
                    return configure(_dinOut);
                }
            }
            else if constexpr (Type == type_t::BLE_IN)
            {
                if constexpr (HAS_BLE_IN)
                {
                    return configure(_bleIn);
                }
            }
            else if constexpr (Type == type_t::BLE_OUT)
            {
                if constexpr (HAS_BLE_OUT)
                {
                    return configure(_bleOut);
                }
            }

            return true;
        }

        bool configure(const gpio_dt_spec& spec)
        {
            if (!gpio_is_ready_dt(&spec))
            {
                return false;
            }

            return gpio_pin_configure_dt(&spec, GPIO_OUTPUT_INACTIVE) == 0;
        }

        template<type_t Type>
        void setOptional(bool state)
        {
            if constexpr (Type == type_t::USB_IN)
            {
                if constexpr (HAS_USB_IN)
                {
                    gpio_pin_set_dt(&_usbIn, state);
                }
            }
            else if constexpr (Type == type_t::USB_OUT)
            {
                if constexpr (HAS_USB_OUT)
                {
                    gpio_pin_set_dt(&_usbOut, state);
                }
            }
            else if constexpr (Type == type_t::DIN_IN)
            {
                if constexpr (HAS_DIN_IN)
                {
                    gpio_pin_set_dt(&_dinIn, state);
                }
            }
            else if constexpr (Type == type_t::DIN_OUT)
            {
                if constexpr (HAS_DIN_OUT)
                {
                    gpio_pin_set_dt(&_dinOut, state);
                }
            }
            else if constexpr (Type == type_t::BLE_IN)
            {
                if constexpr (HAS_BLE_IN)
                {
                    gpio_pin_set_dt(&_bleIn, state);
                }
            }
            else if constexpr (Type == type_t::BLE_OUT)
            {
                if constexpr (HAS_BLE_OUT)
                {
                    gpio_pin_set_dt(&_bleOut, state);
                }
            }
        }

        void set(type_t type, bool state)
        {
            switch (type)
            {
            case type_t::USB_IN:
                setOptional<type_t::USB_IN>(state);
                break;

            case type_t::USB_OUT:
                setOptional<type_t::USB_OUT>(state);
                break;

            case type_t::DIN_IN:
                setOptional<type_t::DIN_IN>(state);
                break;

            case type_t::DIN_OUT:
                setOptional<type_t::DIN_OUT>(state);
                break;

            case type_t::BLE_IN:
                setOptional<type_t::BLE_IN>(state);
                break;

            case type_t::BLE_OUT:
                setOptional<type_t::BLE_OUT>(state);
                break;

            case type_t::ALL:
                setOptional<type_t::USB_IN>(state);
                setOptional<type_t::USB_OUT>(state);
                setOptional<type_t::DIN_IN>(state);
                setOptional<type_t::DIN_OUT>(state);
                setOptional<type_t::BLE_IN>(state);
                setOptional<type_t::BLE_OUT>(state);
                break;

            default:
                break;
            }
        }
    };
}    // namespace io::indicators
