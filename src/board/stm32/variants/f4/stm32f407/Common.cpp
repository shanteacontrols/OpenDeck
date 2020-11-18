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
    TIM_HandleTypeDef htim7;
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
                .port      = GPIOB,
                .index     = GPIO_PIN_11,
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF7_USART3,
            },

            {
                .port      = GPIOB,
                .index     = GPIO_PIN_10,
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF7_USART3,
            },
        };

        const IRQn_Type _irqn = USART3_IRQn;
    } _uartDescriptor3;

    class UARTdescriptor4 : public Board::detail::map::STMPeripheral
    {
        public:
        UARTdescriptor4() = default;

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
            {
                .port      = GPIOA,
                .index     = GPIO_PIN_1,
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF8_UART4,
            },

            {
                .port      = GPIOA,
                .index     = GPIO_PIN_0,
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF8_UART4,
            },
        };

        const IRQn_Type _irqn = UART4_IRQn;
    } _uartDescriptor4;

    class UARTdescriptor5 : public Board::detail::map::STMPeripheral
    {
        public:
        UARTdescriptor5() = default;

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
            {
                .port      = GPIOD,
                .index     = GPIO_PIN_2,
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF8_UART5,
            },

            {
                .port      = GPIOC,
                .index     = GPIO_PIN_12,
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF8_UART5,
            },
        };

        const IRQn_Type _irqn = UART5_IRQn;
    } _uartDescriptor5;

    class UARTdescriptor6 : public Board::detail::map::STMPeripheral
    {
        public:
        UARTdescriptor6() = default;

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
                .port      = GPIOC,
                .index     = GPIO_PIN_7,
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF8_USART6,
            },

            {
                .port      = GPIOC,
                .index     = GPIO_PIN_6,
                .mode      = core::io::pinMode_t::alternatePP,
                .pull      = core::io::pullMode_t::none,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF8_USART6,
            },
        };

        const IRQn_Type _irqn = USART6_IRQn;
    } _uartDescriptor6;

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
                .index     = GPIO_PIN_11,
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
                .port      = GPIOC,
                .index     = GPIO_PIN_9,
                .mode      = core::io::pinMode_t::alternateOD,
                .pull      = core::io::pullMode_t::up,
                .speed     = core::io::gpioSpeed_t::veryHigh,
                .alternate = GPIO_AF4_I2C3,
            },

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
    } _i2cDescriptor3;

    Board::detail::map::STMPeripheral* uart[MAX_UART_INTERFACES] = {
        &_uartDescriptor1,
        &_uartDescriptor2,
        &_uartDescriptor3,
        &_uartDescriptor4,
        &_uartDescriptor5,
        &_uartDescriptor6,
    };

    Board::detail::map::STMPeripheral* i2c[MAX_I2C_INTERFACES] = {
        &_i2cDescriptor1,
        &_i2cDescriptor2,
        &_i2cDescriptor3
    };
}    // namespace

extern "C" void TIM7_IRQHandler(void)
{
    __HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE);
    Board::detail::isrHandling::mainTimer();
}

#ifdef FW_APP
//not needed in bootloader
extern "C" void USART1_IRQHandler(void)
{
    Board::detail::isrHandling::uart(0);
}

extern "C" void USART2_IRQHandler(void)
{
    Board::detail::isrHandling::uart(1);
}

extern "C" void USART3_IRQHandler(void)
{
    Board::detail::isrHandling::uart(2);
}

extern "C" void UART4_IRQHandler(void)
{
    Board::detail::isrHandling::uart(3);
}

extern "C" void UART5_IRQHandler(void)
{
    Board::detail::isrHandling::uart(4);
}

extern "C" void USART6_IRQHandler(void)
{
    Board::detail::isrHandling::uart(5);
}

extern "C" void ADC_IRQHandler(void)
{
    Board::detail::isrHandling::adc(hadc1.Instance->DR);
}
#endif

namespace Board
{
    namespace detail
    {
        namespace setup
        {
            void clocks()
            {
                RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
                RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

                /* Configure the main internal regulator output voltage */
                __HAL_RCC_PWR_CLK_ENABLE();
                __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

                /* Initializes the CPU, AHB and APB busses clocks */
                RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
                RCC_OscInitStruct.HSEState       = RCC_HSE_BYPASS;
                RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
                RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
                RCC_OscInitStruct.PLL.PLLM       = 4;
                RCC_OscInitStruct.PLL.PLLN       = 168;
                RCC_OscInitStruct.PLL.PLLP       = RCC_PLLP_DIV2;
                RCC_OscInitStruct.PLL.PLLQ       = 7;

                if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
                    Board::detail::errorHandler();

                /* Initializes the CPU, AHB and APB busses clocks */
                RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
                RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
                RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV2;
                RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
                RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

                if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
                    Board::detail::errorHandler();
            }

            void timers()
            {
                htim7.Instance               = TIM7;
                htim7.Init.Prescaler         = 0;
                htim7.Init.CounterMode       = TIM_COUNTERMODE_UP;
                htim7.Init.Period            = 41999;
                htim7.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
                htim7.Init.RepetitionCounter = 0;
                htim7.Init.AutoReloadPreload = 0;
                HAL_TIM_Base_Init(&htim7);

                HAL_TIM_Base_Start_IT(&htim7);
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
                return TIM7;
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