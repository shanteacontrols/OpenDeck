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
                    ADC_CHANNEL_1,
                    ADC_CHANNEL_2,
                    ADC_CHANNEL_3,
                    ADC_CHANNEL_8,
                    ADC_CHANNEL_9,
                    ADC_CHANNEL_11,
                    ADC_CHANNEL_12,
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

                    {
                        .port  = DI_15_PORT,
                        .index = DI_15_PIN,
                    },

                    {
                        .port  = DI_16_PORT,
                        .index = DI_16_PIN,
                    },

                    {
                        .port  = DI_17_PORT,
                        .index = DI_17_PIN,
                    },

                    {
                        .port  = DI_18_PORT,
                        .index = DI_18_PIN,
                    }
                };

                ///
                /// \brief Array holding ports and bits for all digital output pins.
                ///
                const core::io::mcuPin_t dOutPins[MAX_NUMBER_OF_LEDS] = {
                    {
                        .port  = DO_1_PORT,
                        .index = DO_1_PIN,
                    },

                    {
                        .port  = DO_2_PORT,
                        .index = DO_2_PIN,
                    },

                    {
                        .port  = DO_3_PORT,
                        .index = DO_3_PIN,
                    },

                    {
                        .port  = DO_4_PORT,
                        .index = DO_4_PIN,
                    },

                    {
                        .port  = DO_5_PORT,
                        .index = DO_5_PIN,
                    },

                    {
                        .port  = DO_6_PORT,
                        .index = DO_6_PIN,
                    },

                    {
                        .port  = DO_7_PORT,
                        .index = DO_7_PIN,
                    },

                    {
                        .port  = DO_8_PORT,
                        .index = DO_8_PIN,
                    },

                    {
                        .port  = DO_9_PORT,
                        .index = DO_9_PIN,
                    },

                    {
                        .port  = DO_10_PORT,
                        .index = DO_10_PIN,
                    },

                    {
                        .port  = DO_11_PORT,
                        .index = DO_11_PIN,
                    },

                    {
                        .port  = DO_12_PORT,
                        .index = DO_12_PIN,
                    },

                    {
                        .port  = DO_13_PORT,
                        .index = DO_13_PIN,
                    },

                    {
                        .port  = DO_14_PORT,
                        .index = DO_14_PIN,
                    },

                    {
                        .port  = DO_15_PORT,
                        .index = DO_15_PIN,
                    },

                    {
                        .port  = DO_16_PORT,
                        .index = DO_16_PIN,
                    }
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
                        return USART3;
                    }

                    IRQn_Type irqn() override
                    {
                        return _irqn;
                    }

                    void enableClock() override
                    {
                        __HAL_RCC_USART3_CLK_ENABLE();
                    }

                    void disableClock() override
                    {
                        __HAL_RCC_USART3_CLK_DISABLE();
                    }

                    private:
                    std::vector<core::io::mcuPin_t> _pins = {
                        {
                            .port      = UART_0_RX_PORT,
                            .index     = UART_0_RX_PIN,
                            .mode      = core::io::pinMode_t::alternatePP,
                            .pull      = core::io::pullMode_t::none,
                            .speed     = core::io::gpioSpeed_t::veryHigh,
                            .alternate = GPIO_AF7_USART3,
                        },

                        {
                            .port      = UART_0_TX_PORT,
                            .index     = UART_0_TX_PIN,
                            .mode      = core::io::pinMode_t::alternatePP,
                            .pull      = core::io::pullMode_t::none,
                            .speed     = core::io::gpioSpeed_t::veryHigh,
                            .alternate = GPIO_AF7_USART3,
                        },
                    };

                    const IRQn_Type _irqn = USART3_IRQn;
                } _uartDescriptor0;
            }    // namespace

            uint32_t adcChannel(uint8_t index)
            {
                return aInChannels[index];
            }

            core::io::mcuPin_t button(uint8_t index)
            {
                return dInPins[index];
            }

            core::io::mcuPin_t led(uint8_t index)
            {
                return dOutPins[index];
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

                if (interface == USART3)
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