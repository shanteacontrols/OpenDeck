/*

Copyright 2015-2022 Igor Petrovic

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

#include <inttypes.h>
#include <vector>

// STM32-specific internal APIs

namespace Board
{
    namespace detail
    {
        namespace st
        {
            class Peripheral
            {
                public:
                Peripheral() = default;

                virtual std::vector<core::io::mcuPin_t> pins()         = 0;
                virtual void*                           interface()    = 0;
                virtual IRQn_Type                       irqn()         = 0;
                virtual void                            enableClock()  = 0;
                virtual void                            disableClock() = 0;
            };

            /// Used to retrieve physical UART interface used on MCU for a given UART channel index as well
            /// as pins on which the interface is connected.
            Peripheral* uartDescriptor(uint8_t channel);

            /// Used to retrieve physical I2C interface used on MCU for a given I2C channel index as well
            /// as pins on which the interface is connected.
            Peripheral* i2cDescriptor(uint8_t channel);

            /// Used to retrieve UART channel on board for a specified UART interface.
            /// If no channels are mapped to the provided interface, return false.
            bool uartChannel(USART_TypeDef* interface, uint8_t& channel);

            /// Used to retrieve I2C channel on board for a specified UART interface.
            /// If no channels are mapped to the provided interface, return false.
            bool i2cChannel(I2C_TypeDef* interface, uint8_t& channel);
        }    // namespace st
    }        // namespace detail
}    // namespace Board
