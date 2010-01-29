/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : 71x_apb.h
* Author             : MCD Application Team
* Version            : V4.0
* Date               : 10/09/2007
* Description        : This file contains all the functions prototypes for the
*                      APB bridge firmware library.
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __71x_APB_H
#define __71x_APB_H

/* Includes ------------------------------------------------------------------*/
#include "71x_map.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* APB1 Peripherals */ 
#define I2C0_Periph     0x0001 
#define I2C1_Periph     0x0002 
#define UART0_Periph    0x0008
#define UART1_Periph    0x0010 
#define UART2_Periph    0x0020 
#define UART3_Periph    0x0040 
#define USB_Periph      0x0080 
#define CAN_Periph      0x0100 
#define BSPI0_Periph    0x0200 
#define BSPI1_Periph    0x0400 
#define HDLC_Periph     0x2000 
#define APB1_ALL_Periph 0x27FB

/* APB2 Peripherals */ 
#define XTI_Periph      0x0001 
#define GPIO0_Periph    0x0004 
#define GPIO1_Periph    0x0008 
#define GPIO2_Periph    0x0010 
#define ADC12_Periph    0x0040 
#define CKOUT_Periph    0x0080 
#define TIM0_Periph     0x0100 
#define TIM1_Periph     0x0200 
#define TIM2_Periph     0x0400 
#define TIM3_Periph     0x0800 
#define RTC_Periph      0x1000 
#define EIC_Periph      0x4000
#define APB2_ALL_Periph 0x5FDD

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void APB_ClockConfig (APB_TypeDef *APBx, FunctionalState NewState,
                      u16 Periph);
void APB_SwResetConfig (APB_TypeDef *APBx, FunctionalState NewState,
                        u16 Periph);

#endif  /* __71x_APB_H*/

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
