/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : 71x_tim.h
* Author             : MCD Application Team
* Version            : V4.0
* Date               : 10/09/2007
* Description        : This file contains all the functions prototypes for the
*                      TIM firmware library.
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __71x_TIM_H
#define __71x_TIM_H

/* Includes ------------------------------------------------------------------*/
#include "71x_map.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  TIM_EXTERNAL,
  TIM_INTERNAL
}TIM_Clocks;

typedef enum
{
  TIM_RISING,
  TIM_FALLING
}TIM_Clock_Edges;

typedef enum
{
  TIM_CHANNEL_A,
  TIM_CHANNEL_B
}TIM_Channels;

typedef enum
{
  TIM_WITH_IT,
  TIM_WITHOUT_IT
}TIM_IT_Mode;

typedef enum
{
  TIM_TIMING,
  TIM_WAVE
}TIM_OC_Modes;

typedef enum
{
  TIM_HIGH,
  TIM_LOW
}TIM_Logic_Levels;

typedef enum
{
  TIM_START,
  TIM_STOP,
  TIM_CLEAR
}TIM_CounterOperations;

typedef enum
{
  TIM_ICFA = 0x8000,
  TIM_OCFA = 0x4000,
  TIM_TOF  = 0x2000,
  TIM_ICFB = 0x1000,
  TIM_OCFB = 0x0800
}TIM_Flags;


typedef struct
{
  u16 Pulse;
  u16 Period;
}PWMI_parameters;

/* Exported constants --------------------------------------------------------*/
#define TIM_ECKEN_Mask       0x0001
#define TIM_EXEDG_Mask       0x0002

#define TIM_IEDGA_Mask       0x0004
#define TIM_IEDGB_Mask       0x0008

#define TIM_PWM_Mask         0x0010

#define TIM_OPM_Mask         0x0020

#define TIM_OCAE_Mask        0x0040
#define TIM_OCBE_Mask        0x0080

#define TIM_OLVLA_Mask       0x0100
#define TIM_OLVLB_Mask       0x0200

#define TIM_FOLVA_Mask       0x0400
#define TIM_FOLVB_Mask       0x0800

#define TIM_PWMI_Mask        0x4000

#define TIM_EN_Mask          0x8000

#define TIM_OCBIE_Mask       0x0800  
#define TIM_ICBIE_Mask       0x1000  
#define TIM_TOIE_Mask        0x2000  
#define TIM_OCAIE_Mask       0x4000
#define TIM_ICAIE_Mask       0x8000  

#define TIM_ICA_IT           0x8000 /* Input Capture Channel A  */
#define TIM_OCA_IT           0x4000 /* Output Compare Channel A */
#define TIM_TO_IT            0x2000 /* Timer OverFlow           */
#define TIM_ICB_IT           0x1000 /* Input Capture Channel B  */
#define TIM_OCB_IT           0x0800 /* Output Compare Channel B */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void TIM_Init(TIM_TypeDef *TIMx);
void TIM_ClockSourceConfig (TIM_TypeDef *TIMx, TIM_Clocks Xclock);
TIM_Clocks TIM_ClockSourceValue (TIM_TypeDef *TIMx);
void TIM_PrescalerConfig (TIM_TypeDef *TIMx, u8 Xprescaler);
u8 TIM_PrescalerValue (TIM_TypeDef *TIMx);
void TIM_ClockLevelConfig (TIM_TypeDef *TIMx, TIM_Clock_Edges Xedge);
TIM_Clock_Edges TIM_ClockLevelValue (TIM_TypeDef *TIMx);
void TIM_ICAPModeConfig (TIM_TypeDef  *TIMx, TIM_Channels Xchannel,
                         TIM_Clock_Edges  Xedge);
u16 TIM_ICAPValue (TIM_TypeDef *TIMx, TIM_Channels Xchannel);
void TIM_OCMPModeConfig (TIM_TypeDef  *TIMx, TIM_Channels Xchannel,
                         u16 XpulseLength, TIM_OC_Modes Xmode,
                         TIM_Logic_Levels Xlevel );
void TIM_OPModeConfig (TIM_TypeDef  *TIMx, u16 XpulseLength,
                       TIM_Logic_Levels XLevel1, TIM_Logic_Levels XLevel2,
                       TIM_Clock_Edges  Xedge );
void TIM_PWMOModeConfig (TIM_TypeDef  *TIMx, u16 XDutyCycle,
                         TIM_Logic_Levels XLevel1, u16 XFullperiod,
                         TIM_Logic_Levels XLevel2);
void TIM_PWMIModeConfig (TIM_TypeDef *TIMx, TIM_Clock_Edges Xedge);
PWMI_parameters TIM_PWMIValue (TIM_TypeDef *TIMx);
void TIM_CounterConfig (TIM_TypeDef *TIMx, TIM_CounterOperations Xoperation);
void TIM_ITConfig (TIM_TypeDef *TIMx, u16 New_IT,
                   FunctionalState NewState);
FlagStatus TIM_FlagStatus (TIM_TypeDef *TIMx, TIM_Flags Xflag);
void TIM_FlagClear (TIM_TypeDef *TIMx, TIM_Flags Xflag);
u16 TIM_CounterValue(TIM_TypeDef *TIMx);

#endif /*__71x_TIM_H*/

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
