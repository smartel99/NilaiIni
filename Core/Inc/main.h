/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#define AUDIO_BKND_ERROR_Pin GPIO_PIN_0
#define AUDIO_BKND_ERROR_GPIO_Port GPIOC
#define AUDIO_BKND_ERROR_EXTI_IRQn EXTI0_IRQn
#define AUDIO_RESET_Pin GPIO_PIN_1
#define AUDIO_RESET_GPIO_Port GPIOC
#define BUTTON_Pin GPIO_PIN_0
#define BUTTON_GPIO_Port GPIOA
#define AUDIO_PVDDEn_Pin GPIO_PIN_1
#define AUDIO_PVDDEn_GPIO_Port GPIOA
#define I2S_LRCLK_Pin GPIO_PIN_4
#define I2S_LRCLK_GPIO_Port GPIOA
#define SD_SELECT_Pin GPIO_PIN_4
#define SD_SELECT_GPIO_Port GPIOC
#define LED_Pin GPIO_PIN_6
#define LED_GPIO_Port GPIOC
#define I2S_MCLK_Pin GPIO_PIN_7
#define I2S_MCLK_GPIO_Port GPIOC
#define I2S_SCLK_Pin GPIO_PIN_10
#define I2S_SCLK_GPIO_Port GPIOC
#define I2S_SDIN_Pin GPIO_PIN_12
#define I2S_SDIN_GPIO_Port GPIOC
#define AUDIO_PDN_Pin GPIO_PIN_5
#define AUDIO_PDN_GPIO_Port GPIOB
#define AUDIO_SCL_Pin GPIO_PIN_6
#define AUDIO_SCL_GPIO_Port GPIOB
#define AUDIO_SDA_Pin GPIO_PIN_7
#define AUDIO_SDA_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define SD_SPI_HANDLE   hspi1
#define SD_CS_GPIO_Port SD_SELECT_GPIO_Port
#define SD_CS_Pin       SD_SELECT_Pin
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
