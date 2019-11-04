/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DO_3_Pin GPIO_PIN_2
#define DO_3_GPIO_Port GPIOE
#define DO_2_Pin GPIO_PIN_4
#define DO_2_GPIO_Port GPIOE
#define DO_11_Pin GPIO_PIN_5
#define DO_11_GPIO_Port GPIOE
#define DO_1_Pin GPIO_PIN_6
#define DO_1_GPIO_Port GPIOE
#define DO_10_Pin GPIO_PIN_13
#define DO_10_GPIO_Port GPIOC
#define A_IN_6_Pin GPIO_PIN_1
#define A_IN_6_GPIO_Port GPIOC
#define A_IN_7_Pin GPIO_PIN_2
#define A_IN_7_GPIO_Port GPIOC
#define A_IN_1_Pin GPIO_PIN_1
#define A_IN_1_GPIO_Port GPIOA
#define A_IN_2_Pin GPIO_PIN_2
#define A_IN_2_GPIO_Port GPIOA
#define A_IN_3_Pin GPIO_PIN_3
#define A_IN_3_GPIO_Port GPIOA
#define A_IN_8_Pin GPIO_PIN_4
#define A_IN_8_GPIO_Port GPIOC
#define DI_1_Pin GPIO_PIN_5
#define DI_1_GPIO_Port GPIOC
#define A_IN_4_Pin GPIO_PIN_0
#define A_IN_4_GPIO_Port GPIOB
#define A_IN_5_Pin GPIO_PIN_1
#define A_IN_5_GPIO_Port GPIOB
#define DI_2_Pin GPIO_PIN_7
#define DI_2_GPIO_Port GPIOE
#define DI_9_Pin GPIO_PIN_8
#define DI_9_GPIO_Port GPIOE
#define DI_3_Pin GPIO_PIN_9
#define DI_3_GPIO_Port GPIOE
#define DI_10_Pin GPIO_PIN_10
#define DI_10_GPIO_Port GPIOE
#define DI_4_Pin GPIO_PIN_11
#define DI_4_GPIO_Port GPIOE
#define DI_11_Pin GPIO_PIN_12
#define DI_11_GPIO_Port GPIOE
#define DI_5_Pin GPIO_PIN_13
#define DI_5_GPIO_Port GPIOE
#define DI_12_Pin GPIO_PIN_14
#define DI_12_GPIO_Port GPIOE
#define DI_6_Pin GPIO_PIN_15
#define DI_6_GPIO_Port GPIOE
#define DI_13_Pin GPIO_PIN_12
#define DI_13_GPIO_Port GPIOB
#define DI_7_Pin GPIO_PIN_9
#define DI_7_GPIO_Port GPIOD
#define DI_14_Pin GPIO_PIN_10
#define DI_14_GPIO_Port GPIOD
#define DI_8_Pin GPIO_PIN_11
#define DI_8_GPIO_Port GPIOD
#define LED_DIN_MIDI_Tx_Pin GPIO_PIN_12
#define LED_DIN_MIDI_Tx_GPIO_Port GPIOD
#define LED_DIN_MIDI_Rx_Pin GPIO_PIN_13
#define LED_DIN_MIDI_Rx_GPIO_Port GPIOD
#define LED_USB_MIDI_Tx_Pin GPIO_PIN_14
#define LED_USB_MIDI_Tx_GPIO_Port GPIOD
#define LED_USB_MIDI_Rx_Pin GPIO_PIN_15
#define LED_USB_MIDI_Rx_GPIO_Port GPIOD
#define DI_16_Pin GPIO_PIN_6
#define DI_16_GPIO_Port GPIOC
#define DI_15_Pin GPIO_PIN_8
#define DI_15_GPIO_Port GPIOC
#define DI_18_Pin GPIO_PIN_15
#define DI_18_GPIO_Port GPIOA
#define DI_17_Pin GPIO_PIN_11
#define DI_17_GPIO_Port GPIOC
#define DO_16_Pin GPIO_PIN_0
#define DO_16_GPIO_Port GPIOD
#define DO_9_Pin GPIO_PIN_1
#define DO_9_GPIO_Port GPIOD
#define DO_15_Pin GPIO_PIN_2
#define DO_15_GPIO_Port GPIOD
#define DO_8_Pin GPIO_PIN_3
#define DO_8_GPIO_Port GPIOD
#define DO_14_Pin GPIO_PIN_6
#define DO_14_GPIO_Port GPIOD
#define DO_7_Pin GPIO_PIN_7
#define DO_7_GPIO_Port GPIOD
#define DO_6_Pin GPIO_PIN_4
#define DO_6_GPIO_Port GPIOB
#define DO_13_Pin GPIO_PIN_5
#define DO_13_GPIO_Port GPIOB
#define DO_12_Pin GPIO_PIN_7
#define DO_12_GPIO_Port GPIOB
#define DO_5_Pin GPIO_PIN_8
#define DO_5_GPIO_Port GPIOB
#define DO_4_Pin GPIO_PIN_0
#define DO_4_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
