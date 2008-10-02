/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : 71x_gpio.h
* Author             : MCD Application Team
* Version            : V4.0
* Date               : 10/09/2007
* Description        : This file contains all the functions prototypes for the
*                      GPIO firmware library.
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __71x_GPIO_H
#define __71x_GPIO_H

/* Includes ------------------------------------------------------------------*/
#include "71x_map.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  GPIO_HI_AIN_TRI,  /* HIGH IMPEDANCE ANALOG INPUT TRISTATE   */
  GPIO_IN_TRI_TTL,  /* TRISTATE TTL INPUT                     */
  GPIO_IN_TRI_CMOS, /* TRISTATE CMOS INPUT                    */
  GPIO_IPUPD_WP,    /* PULL-UP/PULL-DOWN WEAK PUSH PULL INPUT */
  GPIO_OUT_OD,      /* OPEN DRAIN OUTPUT                      */
  GPIO_OUT_PP,      /* PUSH PULL OUTPUT                       */
  GPIO_AF_OD,       /* OPEN DRAIN OUTPUT ALTERNATE FUNCTION   */
  GPIO_AF_PP        /* PUSH-PULL OUTPUT ALTERNATE FUNCTION    */
} GpioPinMode_TypeDef;

/* Exported constants --------------------------------------------------------*/
#define GPIO_LSB             0x00
#define GPIO_MSB             0x08

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void GPIO_Config (GPIO_TypeDef *GPIOx, u16 Port_Pins,
                  GpioPinMode_TypeDef GPIO_Mode);
u8 GPIO_BitRead(GPIO_TypeDef *GPIOx, u8 Port_Pin);
u8 GPIO_ByteRead(GPIO_TypeDef *GPIOx, u8 Port_Byte);
u16 GPIO_WordRead(GPIO_TypeDef *GPIOx);
void GPIO_BitWrite(GPIO_TypeDef *GPIOx, u8 Port_Pin, u8 Port_Val);
void GPIO_ByteWrite(GPIO_TypeDef *GPIOx, u8 Port_Byte, u8 Port_Val);
void GPIO_WordWrite(GPIO_TypeDef *GPIOx, u16 Port_Val);

#endif /*__71x_GPIO_H*/

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
