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

#include "board/Board.h"
#include "board/Internal.h"
#include "board/arch/arm/st/Internal.h"
#include "board/common/io/Helpers.h"
#include "core/src/general/IO.h"
#include "core/src/general/Atomic.h"
#include "core/src/general/ADC.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Helpers.h"
#include <Target.h>

namespace
{
    class UARTdescriptor0 : public Board::detail::st::Peripheral
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
                .port      = CORE_IO_PIN_PORT_DEF(A),
                .index     = CORE_IO_PIN_INDEX_DEF(10),
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF7_USART1,
            },

            // tx
            {
                .port      = CORE_IO_PIN_PORT_DEF(A),
                .index     = CORE_IO_PIN_INDEX_DEF(9),
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF7_USART1,
            },
        };

        const IRQn_Type _irqn = USART1_IRQn;
    } _uartDescriptor0;

    class UARTdescriptor1 : public Board::detail::st::Peripheral
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
                .port      = CORE_IO_PIN_PORT_DEF(A),
                .index     = CORE_IO_PIN_INDEX_DEF(3),
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF7_USART2,
            },

            // tx
            {
                .port      = CORE_IO_PIN_PORT_DEF(A),
                .index     = CORE_IO_PIN_INDEX_DEF(2),
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF7_USART2,
            },
        };

        const IRQn_Type _irqn = USART2_IRQn;
    } _uartDescriptor1;

    class UARTdescriptor2 : public Board::detail::st::Peripheral
    {
        public:
        UARTdescriptor2() = default;

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
            // rx
            {
                .port      = CORE_IO_PIN_PORT_DEF(B),
                .index     = CORE_IO_PIN_INDEX_DEF(11),
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF7_USART3,
            },

            // tx
            {
                .port      = CORE_IO_PIN_PORT_DEF(B),
                .index     = CORE_IO_PIN_INDEX_DEF(10),
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF7_USART3,
            },
        };

        const IRQn_Type _irqn = USART3_IRQn;
    } _uartDescriptor2;

    class UARTdescriptor3 : public Board::detail::st::Peripheral
    {
        public:
        UARTdescriptor3() = default;

        std::vector<core::io::mcuPin_t> pins() override
        {
            return _pins;
        }

        void* interface() override
        {
            return UART4;
        }

        IRQn_Type irqn() override
        {
            return _irqn;
        }

        void enableClock() override
        {
            __HAL_RCC_UART4_CLK_ENABLE();
        }

        void disableClock() override
        {
            __HAL_RCC_UART4_CLK_DISABLE();
        }

        private:
        std::vector<core::io::mcuPin_t> _pins = {
            // rx
            {
                .port      = CORE_IO_PIN_PORT_DEF(A),
                .index     = CORE_IO_PIN_INDEX_DEF(1),
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF8_UART4,
            },

            // tx
            {
                .port      = CORE_IO_PIN_PORT_DEF(A),
                .index     = CORE_IO_PIN_INDEX_DEF(0),
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF8_UART4,
            },
        };

        const IRQn_Type _irqn = UART4_IRQn;
    } _uartDescriptor3;

    class UARTdescriptor4 : public Board::detail::st::Peripheral
    {
        public:
        UARTdescriptor4() = default;

        std::vector<core::io::mcuPin_t> pins() override
        {
            return _pins;
        }

        void* interface() override
        {
            return UART5;
        }

        IRQn_Type irqn() override
        {
            return _irqn;
        }

        void enableClock() override
        {
            __HAL_RCC_UART5_CLK_ENABLE();
        }

        void disableClock() override
        {
            __HAL_RCC_UART5_CLK_DISABLE();
        }

        private:
        std::vector<core::io::mcuPin_t> _pins = {
            // rx
            {
                .port      = CORE_IO_PIN_PORT_DEF(D),
                .index     = CORE_IO_PIN_INDEX_DEF(2),
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF8_UART5,
            },

            // tx
            {
                .port      = CORE_IO_PIN_PORT_DEF(C),
                .index     = CORE_IO_PIN_INDEX_DEF(12),
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF8_UART5,
            },
        };

        const IRQn_Type _irqn = UART5_IRQn;
    } _uartDescriptor4;

    class UARTdescriptor5 : public Board::detail::st::Peripheral
    {
        public:
        UARTdescriptor5() = default;

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
                .port      = CORE_IO_PIN_PORT_DEF(C),
                .index     = CORE_IO_PIN_INDEX_DEF(7),
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF8_USART6,
            },

            // tx
            {
                .port      = CORE_IO_PIN_PORT_DEF(C),
                .index     = CORE_IO_PIN_INDEX_DEF(6),
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF8_USART6,
            },
        };

        const IRQn_Type _irqn = USART6_IRQn;
    } _uartDescriptor5;

    class I2Cdescriptor0 : public Board::detail::st::Peripheral
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
                .port      = CORE_IO_PIN_PORT_DEF(B),
                .index     = CORE_IO_PIN_INDEX_DEF(7),
                .mode      = core::io::pinMode_t::alternateOD,
                .pull      = core::io::pullMode_t::up,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF4_I2C1,
            },

            // scl
            {
                .port      = CORE_IO_PIN_PORT_DEF(B),
                .index     = CORE_IO_PIN_INDEX_DEF(6),
                .mode      = core::io::pinMode_t::alternateOD,
                .pull      = core::io::pullMode_t::up,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF4_I2C1,
            },
        };

        const IRQn_Type _irqn = static_cast<IRQn_Type>(0);
    } _i2cDescriptor0;

    class I2Cdescriptor1 : public Board::detail::st::Peripheral
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
                .port      = CORE_IO_PIN_PORT_DEF(B),
                .index     = CORE_IO_PIN_INDEX_DEF(11),
                .mode      = core::io::pinMode_t::alternateOD,
                .pull      = core::io::pullMode_t::up,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF4_I2C2,
            },

            // scl
            {
                .port      = CORE_IO_PIN_PORT_DEF(B),
                .index     = CORE_IO_PIN_INDEX_DEF(10),
                .mode      = core::io::pinMode_t::alternateOD,
                .pull      = core::io::pullMode_t::up,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF4_I2C2,
            },
        };

        const IRQn_Type _irqn = static_cast<IRQn_Type>(0);
    } _i2cDescriptor1;

    class I2Cdescriptor2 : public Board::detail::st::Peripheral
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
                .port      = CORE_IO_PIN_PORT_DEF(C),
                .index     = CORE_IO_PIN_INDEX_DEF(9),
                .mode      = core::io::pinMode_t::alternateOD,
                .pull      = core::io::pullMode_t::up,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF4_I2C3,
            },

            // scl
            {
                .port      = CORE_IO_PIN_PORT_DEF(A),
                .index     = CORE_IO_PIN_INDEX_DEF(8),
                .mode      = core::io::pinMode_t::alternateOD,
                .pull      = core::io::pullMode_t::up,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF4_I2C3,
            },
        };

        const IRQn_Type _irqn = static_cast<IRQn_Type>(0);
    } _i2cDescriptor2;

    Board::detail::st::Peripheral* _uartDescriptor[MAX_UART_INTERFACES] = {
        &_uartDescriptor0,
        &_uartDescriptor1,
        &_uartDescriptor2,
        &_uartDescriptor3,
        &_uartDescriptor4,
        &_uartDescriptor5,
    };

    Board::detail::st::Peripheral* _i2cDescriptor[MAX_I2C_INTERFACES] = {
        &_i2cDescriptor0,
        &_i2cDescriptor1,
        &_i2cDescriptor2
    };
}    // namespace

#include "STM32F4Descriptors.cpp.include"