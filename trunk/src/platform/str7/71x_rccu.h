/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : 71x_rccu.h
* Author             : MCD Application Team
* Version            : V4.0
* Date               : 10/09/2007
* Description        : This file contains all the functions prototypes for the
*                      RCCU firmware library.
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __71x_RCCU_H
#define __71x_RCCU_H

/* Includes ------------------------------------------------------------------*/
#include "71x_map.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  RCCU_DEFAULT = 0x00,
  RCCU_RCLK_2  = 0x01,
  RCCU_RCLK_4  = 0x02,
  RCCU_RCLK_8  = 0x03
} RCCU_Clock_Div;

typedef enum
{
  RCCU_PLL1_Output,
  RCCU_CLOCK2_16,
  RCCU_CLOCK2,
  RCCU_CK_AF
} RCCU_RCLK_Clocks;

typedef enum
{
  RCCU_PLL1_Mul_12 = 0x01,
  RCCU_PLL1_Mul_16 = 0x03,
  RCCU_PLL1_Mul_20 = 0x00,
  RCCU_PLL1_Mul_24 = 0x02
} RCCU_PLL1_Mul;

typedef enum
{
  RCCU_PLL2_Mul_12 = 0x01,
  RCCU_PLL2_Mul_16 = 0x03,
  RCCU_PLL2_Mul_20 = 0x00,
  RCCU_PLL2_Mul_28 = 0x02
} RCCU_PLL2_Mul;

typedef enum
{
  RCCU_Div_1 = 0x00,
  RCCU_Div_2 = 0x01,
  RCCU_Div_3 = 0x02,
  RCCU_Div_4 = 0x03,
  RCCU_Div_5 = 0x04,
  RCCU_Div_6 = 0x05,
  RCCU_Div_7 = 0x06
} RCCU_PLL_Div;

typedef enum
{
  RCCU_PLL2_Output = 0x01,
  RCCU_USBCK       = 0x00
} RCCU_USB_Clocks;

typedef enum
{
  RCCU_CLK2,
  RCCU_RCLK,
  RCCU_MCLK,
  RCCU_PCLK2,
  RCCU_PCLK1
} RCCU_Clocks;

typedef enum
{
  RCCU_PLL1_LOCK_IT = 0x0080,
  RCCU_CKAF_IT      = 0x0100,
  RCCU_CK2_16_IT    = 0x0200,
  RCCU_STOP_IT      = 0x0400
} RCCU_Interrupts;

typedef enum
{
  RCCU_PLL1_LOCK   = 0x0002,
  RCCU_CKAF_ST     = 0x0004,
  RCCU_PLL1_LOCK_I = 0x0800,
  RCCU_CKAF_I      = 0x1000,
  RCCU_CK2_16_I    = 0x2000,
  RCCU_STOP_I      = 0x4000
} RCCU_Flags;

typedef enum
{
  RCCU_ExternalReset = 0x00000000,
  RCCU_SoftwareReset = 0x00000020,
  RCCU_WDGReset      = 0x00000040,
  RCCU_RTCAlarmReset = 0x00000080,
  RCCU_LVDReset      = 0x00000200,
  RCCU_WKPReset      = 0x00000400
}RCCU_ResetSources;

typedef enum
{
  RCCU_PLL1FreeRunning125,
  RCCU_PLL1FreeRunning250,
  RCCU_PLL1FreeRunning500
}RCCU_PLL1FreeRunningModes;

/* Exported constants --------------------------------------------------------*/
#define RCCU_Div2_Mask            0x00008000
#define RCCU_Div2_Index           0x0F
#define RCCU_FACT_Mask            0x0003

#define RCCU_FACT1_Mask           0x0003

#define RCCU_FACT2_Mask           0x0300
#define RCCU_FACT2_Index          0x08

#define RCCU_MX_Mask              0x00000030
#define RCCU_MX_Index             0x04

#define RCCU_DX_Mask              0x00000007

#define RCCU_FREFRANGE_Mask       0x00000040

#define RCCU_FRQRNG_Mask          0x00000040

#define RCCU_FREEN_Mask           0x00000080

#define RCCU_PLLEN_Mask           0x00000080

#define RCCU_CSU_CKSEL_Mask       0x00000001

#define RCCU_CK2_16_Mask          0x00000008

#define RCCU_CKAF_SEL_Mask        0x00000004

#define RCCU_LOCK_Mask            0x00000002

#define RCCU_USBEN_Mask           0x0100
#define RCCU_USBEN_Index          0x08
#define RCCU_ResetSources_Mask    0x000006E0

/* RTC Oscillator Frequency value = 32 768 Hz */
#define RCCU_RTC_Osc              32768

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void RCCU_Div2Config (FunctionalState NewState);
FlagStatus RCCU_Div2Status (void);
void RCCU_MCLKConfig (RCCU_Clock_Div New_Clock);
void RCCU_PCLK1Config (RCCU_Clock_Div New_Clock);
void RCCU_PCLK2Config (RCCU_Clock_Div New_Clock);
void RCCU_PLL1Config (RCCU_PLL1_Mul New_Mul, RCCU_PLL_Div New_Div);
void RCCU_PLL2Config (RCCU_PLL2_Mul New_Mul, RCCU_PLL_Div New_Div,
                      u32 HCLK_Clock);
void RCCU_RCLKSourceConfig (RCCU_RCLK_Clocks New_Clock);
RCCU_RCLK_Clocks RCCU_RCLKClockSource (void);
void RCCU_USBCLKConfig (RCCU_USB_Clocks New_Clock);
RCCU_USB_Clocks RCCU_USBClockSource (void);
u32 RCCU_FrequencyValue (RCCU_Clocks Internal_Clk);
void RCCU_ITConfig (RCCU_Interrupts RCCU_IT, FunctionalState NewState);
FlagStatus RCCU_FlagStatus (RCCU_Flags RCCU_flag);
void RCCU_FlagClear (RCCU_Flags RCCU_flag);
RCCU_ResetSources RCCU_ResetSource (void);
void RCCU_PLL1FreeRunningModeConfig(RCCU_PLL1FreeRunningModes
                                    NewPll1FreeRunningMode);
void RCCU_PLL1Disable(void);
void RCCU_PLL2Disable(void);
void RCCU_GenerateSWReset(void);

#endif /*__71x_RCCU_H*/

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
