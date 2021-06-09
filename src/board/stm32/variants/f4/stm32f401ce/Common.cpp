/*

Copyright 2015-2021 Igor Petrovic

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

#include "board/Board.h"
#include "board/Internal.h"
#include "board/common/io/Helpers.h"
#include "core/src/general/IO.h"
#include "core/src/general/Atomic.h"
#include "core/src/general/ADC.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Helpers.h"
#include "MCU.h"

namespace
{
    class UARTdescriptor1 : public Board::detail::map::STMPeripheral
    {
        public:
        UARTdescriptor1() = default;

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
                .port      = GPIOA,
                .index     = GPIO_PIN_10,
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF7_USART1,
            },

            {
                .port      = GPIOA,
                .index     = GPIO_PIN_9,
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF7_USART1,
            },
        };

        const IRQn_Type _irqn = USART1_IRQn;
    } _uartDescriptor1;

    class UARTdescriptor2 : public Board::detail::map::STMPeripheral
    {
        public:
        UARTdescriptor2() = default;

        std::vector<core::io::mcuPin_t> pins() override
        {
            return _pins;
        }

        void* interface() override
        {
            return USART2;
        }

        IRQn_Type irqn() override
        {
            return _irqn;
        }

        void enableClock() override
        {
            __HAL_RCC_USART2_CLK_ENABLE();
        }

        void disableClock() override
        {
            __HAL_RCC_USART2_CLK_DISABLE();
        }

        private:
        std::vector<core::io::mcuPin_t> _pins = {
            {
                .port      = GPIOA,
                .index     = GPIO_PIN_3,
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF7_USART2,
            },

            {
                .port      = GPIOA,
                .index     = GPIO_PIN_2,
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF7_USART2,
            },
        };

        const IRQn_Type _irqn = USART2_IRQn;
    } _uartDescriptor2;

    class UARTdescriptor3 : public Board::detail::map::STMPeripheral
    {
        public:
        UARTdescriptor3() = default;

        std::vector<core::io::mcuPin_t> pins() override
        {
            return _pins;
        }

        void* interface() override
        {
            return USART6;
        }

        IRQn_Type irqn() override
        {
            return _irqn;
        }

        void enableClock() override
        {
            __HAL_RCC_USART6_CLK_ENABLE();
        }

        void disableClock() override
        {
            __HAL_RCC_USART6_CLK_DISABLE();
        }

        private:
        std::vector<core::io::mcuPin_t> _pins = {
            {
                .port      = GPIOA,
                .index     = GPIO_PIN_11,
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF8_USART6,
            },

            {
                .port      = GPIOA,
                .index     = GPIO_PIN_12,
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF8_USART6,
            },
        };

        const IRQn_Type _irqn = USART6_IRQn;
    } _uartDescriptor3;

    class I2Cdescriptor1 : public Board::detail::map::STMPeripheral
    {
        public:
        I2Cdescriptor1() = default;

        std::vector<core::io::mcuPin_t> pins() override
        {
            return _pins;
        }

        void* interface() override
        {
            return I2C1;
        }

        IRQn_Type irqn() override
        {
            return _irqn;
        }

        void enableClock() override
        {
            __HAL_RCC_I2C1_CLK_ENABLE();
        }

        void disableClock() override
        {
            __HAL_RCC_I2C1_CLK_DISABLE();
        }

        private:
        std::vector<core::io::mcuPin_t> _pins = {
            {
                .port      = GPIOB,
                .index     = GPIO_PIN_6,
                .mode      = core::io::pinMode_t::alternateOD,
                .pull      = core::io::pullMode_t::up,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF4_I2C1,
            },

            {
                .port      = GPIOB,
                .index     = GPIO_PIN_7,
                .mode      = core::io::pinMode_t::alternateOD,
                .pull      = core::io::pullMode_t::up,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF4_I2C1,
            },
        };

        const IRQn_Type _irqn = static_cast<IRQn_Type>(0);
    } _i2cDescriptor1;

    class I2Cdescriptor2 : public Board::detail::map::STMPeripheral
    {
        public:
        I2Cdescriptor2() = default;

        std::vector<core::io::mcuPin_t> pins() override
        {
            return _pins;
        }

        void* interface() override
        {
            return I2C2;
        }

        IRQn_Type irqn() override
        {
            return _irqn;
        }

        void enableClock() override
        {
            __HAL_RCC_I2C2_CLK_ENABLE();
        }

        void disableClock() override
        {
            __HAL_RCC_I2C2_CLK_DISABLE();
        }

        private:
        std::vector<core::io::mcuPin_t> _pins = {
            {
                .port      = GPIOB,
                .index     = GPIO_PIN_10,
                .mode      = core::io::pinMode_t::alternateOD,
                .pull      = core::io::pullMode_t::up,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF4_I2C2,
            },

            {
                .port      = GPIOB,
                .index     = GPIO_PIN_3,
                .mode      = core::io::pinMode_t::alternateOD,
                .pull      = core::io::pullMode_t::up,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF4_I2C2,
            },
        };

        const IRQn_Type _irqn = static_cast<IRQn_Type>(0);
    } _i2cDescriptor2;

    class I2Cdescriptor3 : public Board::detail::map::STMPeripheral
    {
        public:
        I2Cdescriptor3() = default;

        std::vector<core::io::mcuPin_t> pins() override
        {
            return _pins;
        }

        void* interface() override
        {
            return I2C3;
        }

        IRQn_Type irqn() override
        {
            return _irqn;
        }

        void enableClock() override
        {
            __HAL_RCC_I2C3_CLK_ENABLE();
        }

        void disableClock() override
        {
            __HAL_RCC_I2C3_CLK_DISABLE();
        }

        private:
        std::vector<core::io::mcuPin_t> _pins = {
            {
                .port      = GPIOA,
                .index     = GPIO_PIN_8,
                .mode      = core::io::pinMode_t::alternateOD,
                .pull      = core::io::pullMode_t::up,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF4_I2C3,
            },

            {
                .port      = GPIOB,
                .index     = GPIO_PIN_4,
                .mode      = core::io::pinMode_t::alternateOD,
                .pull      = core::io::pullMode_t::up,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF4_I2C3,
            },
        };

        const IRQn_Type _irqn = static_cast<IRQn_Type>(0);
    } _i2cDescriptor3;

    Board::detail::map::STMPeripheral* uart[MAX_UART_INTERFACES] = {
        &_uartDescriptor1,
        &_uartDescriptor2,
        &_uartDescriptor3,
    };

    Board::detail::map::STMPeripheral* i2c[MAX_I2C_INTERFACES] = {
        &_i2cDescriptor1,
        &_i2cDescriptor2,
        &_i2cDescriptor3
    };
}    // namespace

namespace Board
{
    namespace detail
    {
        namespace map
        {
            bool uartChannel(USART_TypeDef* interface, uint8_t& channel)
            {
                for (int i = 0; i < MAX_UART_INTERFACES; i++)
                {
                    if (static_cast<USART_TypeDef*>(uart[i]->interface()) == interface)
                    {
                        channel = i;
                        return true;
                    }
                }

                return false;
            }

            STMPeripheral* uartDescriptor(uint8_t channel)
            {
                if (channel >= MAX_UART_INTERFACES)
                    return nullptr;

                return uart[channel];
            }

            bool i2cChannel(I2C_TypeDef* interface, uint8_t& channel)
            {
                for (int i = 0; i < MAX_I2C_INTERFACES; i++)
                {
                    if (static_cast<I2C_TypeDef*>(i2c[i]->interface()) == interface)
                    {
                        channel = i;
                        return true;
                    }
                }

                return false;
            }

            STMPeripheral* i2cDescriptor(uint8_t channel)
            {
                if (channel >= MAX_I2C_INTERFACES)
                    return nullptr;

                return i2c[channel];
            }

            uint32_t adcChannel(const core::io::mcuPin_t& pin)
            {
                uint8_t index = core::misc::maskToIndex(pin.index);

                if (pin.port == GPIOA)
                {
                    if (index < 8)
                        return index;
                }
                else if (pin.port == GPIOB)
                {
                    switch (index)
                    {
                    case 0:
                        return 8;

                    case 1:
                        return 9;

                    default:
                        return 0xFF;
                    }
                }
                else if (pin.port == GPIOC)
                {
                    if (index < 6)
                        return 10 + index;
                }

                return 0xFF;
            }
        }    // namespace map
    }        // namespace detail
}    // namespace Board