/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : 71x_lib.h
* Author             : MCD Application Team
* Version            : V4.0
* Date               : 10/09/2007
* Description        : This file includes the peripherals header files in the
*                      user application.
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __71x_LIB_H
#define __71x_LIB_H

/* Includes ------------------------------------------------------------------*/
#include "71x_map.h"
#include "71x_conf.h"


#ifdef _ADC12
#include "71x_adc12.h"
#endif

#ifdef _APB
#include "71x_apb.h"
#endif

#ifdef _BSPI
#include "71x_bspi.h"
#endif

#ifdef _CAN
#include "71x_can.h"
#endif

#ifdef _EIC
#include "71x_eic.h"
#endif

#ifdef _EMI
#include "71x_emi.h"
#endif

#ifdef _GPIO
#include "71x_gpio.h"
#endif

#ifdef _I2C
#include "71x_i2c.h"
#endif

#ifdef _PCU
#include "71x_pcu.h"
#endif

#ifdef _RCCU
#include "71x_rccu.h"
#endif

#ifdef _RTC
#include "71x_rtc.h"
#endif

#ifdef _TIM
#include "71x_tim.h"
#endif

#ifdef _UART
#include "71x_uart.h"
#endif

#ifdef _USB
#endif

#ifdef _WDG
#include "71x_wdg.h"
#endif

#ifdef _XTI
#include "71x_xti.h"
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void debug(void);

#endif /*__71x_LIB_H*/

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
