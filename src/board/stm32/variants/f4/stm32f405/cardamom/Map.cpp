/*

Copyright 2015-2020 Igor Petrovic

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

#include "Pins.h"
#include "board/Internal.h"

namespace Board
{
    namespace detail
    {
        namespace map
        {
            namespace
            {
                const uint32_t aInChannels[MAX_NUMBER_OF_ANALOG] = {
                    ADC_CHANNEL_15,
                    ADC_CHANNEL_14,
                };

                ///
                /// \brief Array holding ports and bits for all digital input pins.
                ///
                const core::io::mcuPin_t dInPins[MAX_NUMBER_OF_BUTTONS] = {
                    {
                        .port  = DI_1_PORT,
                        .index = DI_1_PIN,
                    },

                    {
                        .port  = DI_2_PORT,
                        .index = DI_2_PIN,
                    },

                    {
                        .port  = DI_3_PORT,
                        .index = DI_3_PIN,
                    },

                    {
                        .port  = DI_4_PORT,
                        .index = DI_4_PIN,
                    },

                    {
                        .port  = DI_5_PORT,
                        .index = DI_5_PIN,
                    },

                    {
                        .port  = DI_6_PORT,
                        .index = DI_6_PIN,
                    },

                    {
                        .port  = DI_7_PORT,
                        .index = DI_7_PIN,
                    },

                    {
                        .port  = DI_8_PORT,
                        .index = DI_8_PIN,
                    },

                    {
                        .port  = DI_9_PORT,
                        .index = DI_9_PIN,
                    },

                    {
                        .port  = DI_10_PORT,
                        .index = DI_10_PIN,
                    },

                    {
                        .port  = DI_11_PORT,
                        .index = DI_11_PIN,
                    },

                    {
                        .port  = DI_12_PORT,
                        .index = DI_12_PIN,
                    },

                    {
                        .port  = DI_13_PORT,
                        .index = DI_13_PIN,
                    },

                    {
                        .port  = DI_14_PORT,
                        .index = DI_14_PIN,
                    },
                };

                EmuEEPROM::pageDescriptor_t flashPage1 = {
                    .startAddress = EEPROM_PAGE1_START_ADDRESS,
                    .sector       = EEPROM_PAGE1_SECTOR
                };

                EmuEEPROM::pageDescriptor_t flashPage2 = {
                    .startAddress = EEPROM_PAGE2_START_ADDRESS,
                    .sector       = EEPROM_PAGE2_SECTOR
                };

                class UARTdescriptor0 : public Board::detail::map::STMPeripheral
                {
                    public:
                    UARTdescriptor0() {}

                    std::vector<core::io::mcuPin_t> pins() override
                    {
                        return _pins;
                    }

                    void* interface() override
                    {
                        return USART1;
                    }

                    IRQn_Type irqn() override
                    {
                        return _irqn;
                    }

                    void enableClock() override
                    {
                        __HAL_RCC_USART1_CLK_ENABLE();
                    }

                    void disableClock() override
                    {
                        __HAL_RCC_USART1_CLK_DISABLE();
                    }

                    private:
                    std::vector<core::io::mcuPin_t> _pins = {
                        {
                            .port      = UART_0_RX_PORT,
                            .index     = UART_0_RX_PIN,
                            .mode      = core::io::pinMode_t::alternatePP,
                            .pull      = core::io::pullMode_t::none,
                            .speed     = core::io::gpioSpeed_t::veryHigh,
                            .alternate = GPIO_AF7_USART1,
                        },

                        {
                            .port      = UART_0_TX_PORT,
                            .index     = UART_0_TX_PIN,
                            .mode      = core::io::pinMode_t::alternatePP,
                            .pull      = core::io::pullMode_t::none,
                            .speed     = core::io::gpioSpeed_t::veryHigh,
                            .alternate = GPIO_AF7_USART1,
                        },
                    };

                    const IRQn_Type _irqn = USART1_IRQn;
                } _uartDescriptor0;
            }    // namespace

            uint8_t muxChannel(uint8_t index)
            {
                return index;
            }

            uint32_t adcChannel(uint8_t index)
            {
                return aInChannels[index];
            }

            core::io::mcuPin_t button(uint8_t index)
            {
                return dInPins[index];
            }

            STMPeripheral* uartDescriptor(uint8_t channel)
            {
                if (channel >= UART_INTERFACES)
                    return nullptr;

                //only one uart
                return &_uartDescriptor0;
            }

            bool uartChannel(USART_TypeDef* interface, uint8_t& channel)
            {
                bool returnValue = true;

                if (interface == USART1)
                {
                    channel = 0;
                }
                else
                {
                    returnValue = false;
                }

                return returnValue;
            }

            ADC_TypeDef* adcInterface()
            {
                return ADC1;
            }

            TIM_TypeDef* mainTimerInstance()
            {
                return TIM7;
            }

            EmuEEPROM::pageDescriptor_t& eepromFlashPage1()
            {
                return flashPage1;
            }

            EmuEEPROM::pageDescriptor_t& eepromFlashPage2()
            {
                return flashPage2;
            }
        }    // namespace map
    }        // namespace detail
}    // namespace Board