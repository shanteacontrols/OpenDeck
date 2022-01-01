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
#include <MCU.h>

namespace
{
    class UARTdescriptor0 : public Board::detail::map::STMPeripheral
    {
        public:
        UARTdescriptor0() = default;

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
            // rx
            {
                .port      = GPIOA,
                .index     = GPIO_PIN_10,
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF7_USART1,
            },

            // tx
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
    } _uartDescriptor0;

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
            // rx
            {
                .port      = GPIOA,
                .index     = GPIO_PIN_3,
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF7_USART2,
            },

            // tx
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
            // rx
            {
                .port      = GPIOA,
                .index     = GPIO_PIN_12,
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF8_USART6,
            },

            // tx
            {
                .port      = GPIOA,
                .index     = GPIO_PIN_11,
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF8_USART6,
            },
        };

        const IRQn_Type _irqn = USART6_IRQn;
    } _uartDescriptor2;

    class I2Cdescriptor0 : public Board::detail::map::STMPeripheral
    {
        public:
        I2Cdescriptor0() = default;

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
            // sda
            {
                .port      = GPIOB,
                .index     = GPIO_PIN_7,
                .mode      = core::io::pinMode_t::alternateOD,
                .pull      = core::io::pullMode_t::up,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF4_I2C1,
            },

            // scl
            {
                .port      = GPIOB,
                .index     = GPIO_PIN_6,
                .mode      = core::io::pinMode_t::alternateOD,
                .pull      = core::io::pullMode_t::up,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF4_I2C1,
            },
        };

        const IRQn_Type _irqn = static_cast<IRQn_Type>(0);
    } _i2cDescriptor0;

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
            // sda
            {
                .port      = GPIOB,
                .index     = GPIO_PIN_3,
                .mode      = core::io::pinMode_t::alternateOD,
                .pull      = core::io::pullMode_t::up,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF4_I2C2,
            },

            // scl
            {
                .port      = GPIOB,
                .index     = GPIO_PIN_10,
                .mode      = core::io::pinMode_t::alternateOD,
                .pull      = core::io::pullMode_t::up,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF4_I2C2,
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
            // sda
            {
                .port      = GPIOB,
                .index     = GPIO_PIN_4,
                .mode      = core::io::pinMode_t::alternateOD,
                .pull      = core::io::pullMode_t::up,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF4_I2C3,
            },

            // scl
            {
                .port      = GPIOA,
                .index     = GPIO_PIN_8,
                .mode      = core::io::pinMode_t::alternateOD,
                .pull      = core::io::pullMode_t::up,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF4_I2C3,
            },
        };

        const IRQn_Type _irqn = static_cast<IRQn_Type>(0);
    } _i2cDescriptor2;

    Board::detail::map::STMPeripheral* _uartDescriptor[MAX_UART_INTERFACES] = {
        &_uartDescriptor0,
        &_uartDescriptor1,
        &_uartDescriptor2,
    };

    Board::detail::map::STMPeripheral* _i2cDescriptor[MAX_I2C_INTERFACES] = {
        &_i2cDescriptor0,
        &_i2cDescriptor1,
        &_i2cDescriptor2
    };
}    // namespace

#include "STM32F4Descriptors.cpp.include"