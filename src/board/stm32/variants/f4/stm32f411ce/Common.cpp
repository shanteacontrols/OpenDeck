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
#include "Pins.h"
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
    TIM_HandleTypeDef htim5;
    ADC_HandleTypeDef hadc1;

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

extern "C" void TIM5_IRQHandler(void)
{
    __HAL_TIM_CLEAR_IT(&htim5, TIM_IT_UPDATE);
    Board::detail::isrHandling::mainTimer();
}

#if defined(FW_APP) || defined(FW_CDC)
//not needed in bootloader
#ifdef USE_UART
extern "C" void USART1_IRQHandler(void)
{
    Board::detail::isrHandling::uart(0);
}

extern "C" void USART2_IRQHandler(void)
{
    Board::detail::isrHandling::uart(1);
}

extern "C" void USART6_IRQHandler(void)
{
    Board::detail::isrHandling::uart(5);
}
#endif

#ifdef FW_APP
extern "C" void ADC_IRQHandler(void)
{
    Board::detail::isrHandling::adc(hadc1.Instance->DR);
}
#endif
#endif

namespace Board
{
    namespace detail
    {
        namespace setup
        {
            void timers()
            {
                htim5.Instance               = TIM5;
                htim5.Init.Prescaler         = 0;
                htim5.Init.CounterMode       = TIM_COUNTERMODE_UP;
                htim5.Init.Period            = 41999;
                htim5.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
                htim5.Init.RepetitionCounter = 0;
                htim5.Init.AutoReloadPreload = 0;
                HAL_TIM_Base_Init(&htim5);

                HAL_TIM_Base_Start_IT(&htim5);
            }

            void adc()
            {
                ADC_ChannelConfTypeDef sConfig = { 0 };

                hadc1.Instance                   = ADC1;
                hadc1.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;
                hadc1.Init.Resolution            = ADC_RESOLUTION_12B;
                hadc1.Init.ScanConvMode          = DISABLE;
                hadc1.Init.ContinuousConvMode    = DISABLE;
                hadc1.Init.DiscontinuousConvMode = DISABLE;
                hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
                hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
                hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
                hadc1.Init.NbrOfConversion       = 1;
                hadc1.Init.DMAContinuousRequests = DISABLE;
                hadc1.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
                HAL_ADC_Init(&hadc1);

                for (int i = 0; i < MAX_ADC_CHANNELS; i++)
                {
                    sConfig.Channel      = map::adcChannel(i);
                    sConfig.Rank         = 1;
                    sConfig.SamplingTime = ADC_SAMPLETIME_112CYCLES;
                    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
                }

                //set first channel
                core::adc::setChannel(map::adcChannel(0));

                HAL_ADC_Start_IT(&hadc1);
            }
        }    // namespace setup

        namespace map
        {
            ADC_TypeDef* adcInterface()
            {
                return ADC1;
            }

            TIM_TypeDef* mainTimerInstance()
            {
                return TIM5;
            }

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

            uint32_t adcChannel(core::io::mcuPin_t pin)
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