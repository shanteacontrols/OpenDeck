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

#pragma once

#include "stm32f4xx_hal.h"

#define DI_1_PORT               GPIOB
#define DI_1_PIN                GPIO_PIN_15

#define DI_2_PORT               GPIOB
#define DI_2_PIN                GPIO_PIN_14

#define DI_3_PORT               GPIOB
#define DI_3_PIN                GPIO_PIN_13

#define DI_4_PORT               GPIOB
#define DI_4_PIN                GPIO_PIN_12

#define DI_5_PORT               GPIOB
#define DI_5_PIN                GPIO_PIN_11

#define DI_6_PORT               GPIOB
#define DI_6_PIN                GPIO_PIN_10

#define DI_7_PORT               GPIOB
#define DI_7_PIN                GPIO_PIN_1

#define DI_8_PORT               GPIOB
#define DI_8_PIN                GPIO_PIN_0

#define DI_9_PORT               GPIOB
#define DI_9_PIN                GPIO_PIN_9

#define DI_10_PORT              GPIOB
#define DI_10_PIN               GPIO_PIN_8

#define DI_11_PORT              GPIOB
#define DI_11_PIN               GPIO_PIN_7

#define DI_12_PORT              GPIOB
#define DI_12_PIN               GPIO_PIN_6

#define DI_13_PORT              GPIOB
#define DI_13_PIN               GPIO_PIN_5

#define DI_14_PORT              GPIOB
#define DI_14_PIN               GPIO_PIN_4


#define MUX_S0_PORT             GPIOA
#define MUX_S0_PIN              GPIO_PIN_5

#define MUX_S1_PORT             GPIOA
#define MUX_S1_PIN              GPIO_PIN_4

#define MUX_S2_PORT             GPIOA
#define MUX_S2_PIN              GPIO_PIN_7

#define MUX_S3_PORT             GPIOA
#define MUX_S3_PIN              GPIO_PIN_6


#define MUX_1_IN_PORT           GPIOC
#define MUX_1_IN_PIN            GPIO_PIN_5

#define MUX_2_IN_PORT           GPIOC
#define MUX_2_IN_PIN            GPIO_PIN_4


#define UART_0_RX_PORT          GPIOA
#define UART_0_RX_PIN           GPIO_PIN_10

#define UART_0_TX_PORT          GPIOA
#define UART_0_TX_PIN           GPIO_PIN_9


#define BTLDR_BUTTON_PORT       GPIOA
#define BTLDR_BUTTON_PIN        GPIO_PIN_8