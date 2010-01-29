/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : 71x_tim.c
* Author             : MCD Application Team
* Version            : V4.0
* Date               : 10/09/2007
* Description        : This file provides all the TIM firmware functions.
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "71x_tim.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : TIM_Init
* Description    : This routine is used to Initialize the TIM peripheral
*                  registers to their default values.
* Input          : - TIMx: the Timer to be Initialized.
* Output         : None.
* Return         : None.
*******************************************************************************/
void TIM_Init(TIM_TypeDef *TIMx)
{
  TIMx->CR1 = 0x0000;
  TIMx->CR2 = 0x0000;
  TIMx->SR  = 0x0000;
}

/*******************************************************************************
* Function Name  : TIM_ClockSourceConfig
* Description    : This routine is used to configure the TIM clock source
* Input          : - TIMx: specifies the TIM to be configured.
*                  - Xclock: specifies the TIM source clock. It can be:
*                     TIM_INTERNAL : the TIM is clocked by the APB2 frequency
*                                    divided by the prescaler value.
*                     TIM_EXTERNAL : the TIM is clocked by an external Clock.
* Output         : None.
* Return         : None.
*******************************************************************************/
void TIM_ClockSourceConfig (TIM_TypeDef *TIMx, TIM_Clocks Xclock)
{
  if (Xclock == TIM_EXTERNAL)
  {
    TIMx->CR1 |= TIM_ECKEN_Mask;
  }
  else
  {
    TIMx->CR1 &= ~TIM_ECKEN_Mask;
  }
}

/*******************************************************************************
* Function Name  : TIM_ClockSourceValue
* Description    : This routine is used to get the TIM clock source
* Input          : - TIMx: specifies the TIM to check its source clock.
* Output         : None.
* Return         : The TIM source clock. It can be:
*                   TIM_INTERNAL: The TIM is clocked by the APB2 frequency
*                                 divided by the prescaler value.
*                   TIM_EXTERNAL: The TIM is clocked by an external Clock.
*******************************************************************************/
TIM_Clocks TIM_ClockSourceValue (TIM_TypeDef *TIMx)
{
  if ((TIMx->CR1 & TIM_ECKEN_Mask) == 0)
  {
    return TIM_INTERNAL;	
  }
  else
  {
    return TIM_EXTERNAL;
  }	
}

/*******************************************************************************
* Function Name  : TIM_PrescalerConfig
* Description    : This routine is used to configure the TIM prescaler value
*                  to divide the internal clock.
* Input          : - TIMx: specifies the TIM to be configured.
*                  - Xprescaler: specifies the TIM prescaler value (8bit).
* Output         : None.
* Return         : None.
*******************************************************************************/
void TIM_PrescalerConfig (TIM_TypeDef *TIMx, u8 Xprescaler)
{
  TIMx->CR2 = (TIMx->CR2 & 0xFF00) | Xprescaler;
}

/*******************************************************************************
* Function Name  : TIM_PrescalerValue
* Description    : This routine is used to get the TIM prescaler value 
*                  when the internal clock is used.
* Input          : - TIMx: specifies the timer to get its prescaler value.
* Output         : None.
* Return         : The Current TIM prescaler Value (8bit).
*******************************************************************************/
u8 TIM_PrescalerValue (TIM_TypeDef *TIMx)
{
  return TIMx->CR2 & 0x00FF;
}

/*******************************************************************************
* Function Name  : TIM_ClockLevelConfig
* Description    : This routine is used to configure the TIM clock level
*                  when an external clock source is used.
* Input          : - TIMx: specifies the TIM to be configured.
*                  - Xedge: specifies the active edge of the external clock.
*                    It can be:
*                     TIM_RISING : The rising  edge.
*                     TIM_FALLING: The falling edge.
* Output         : None.
* Return         : None.
*******************************************************************************/
void TIM_ClockLevelConfig (TIM_TypeDef *TIMx, TIM_Clock_Edges Xedge)
{
  if (Xedge == TIM_RISING)
  {
    TIMx->CR1 |= TIM_EXEDG_Mask;
  }
  else
  {
    TIMx->CR1 &= ~TIM_EXEDG_Mask;
  }
}

/*******************************************************************************
* Function Name  : TIM_ClockLevelValue
* Description    : This routine is used to get and return the clock active level
*                  when using an external clock source
* Input          : - TIMx: specifies the TIM to be configured.
* Output         : None.
* Return         : The external clock level of the specified timer.
*                  It can be:            
*                    TIM_RISING  : The rising  edge.
*                    TIM_FALLING : The falling edge.
*******************************************************************************/
TIM_Clock_Edges TIM_ClockLevelValue (TIM_TypeDef *TIMx)
{
  if ((TIMx->CR1 & TIM_EXEDG_Mask) == 0)
  {
    return TIM_FALLING;	
  }	
  else
  {
    return TIM_RISING;	
  }
}

/*******************************************************************************
* Function Name  : TIM_ICAPModeConfig
* Description    : This routine is used to configure the input capture feature
* Input          : - TIMx: specifies the TIM to be configured.
*                  - Xchannel: specifies the input Capture Channel, it can be:
*                     TIM_CHANNEL_A, TIM_CHANNEL_B
*                  - Xedge: specifies the Active Edge, it can be:
*                     TIM_RISING, TIM_FALLING
* Output         : None.
* Return         : None.
*******************************************************************************/
void TIM_ICAPModeConfig (TIM_TypeDef  *TIMx, TIM_Channels Xchannel,
                         TIM_Clock_Edges  Xedge)
{
  switch (Xchannel)
  {
    case TIM_CHANNEL_A :
      if (Xedge == TIM_RISING)
      {
        TIMx->CR1 |= TIM_IEDGA_Mask;
      }
      else
      {
        TIMx->CR1 &= ~TIM_IEDGA_Mask;
      }
      break;
    case TIM_CHANNEL_B :
      if (Xedge == TIM_RISING)
      {
        TIMx->CR1 |= TIM_IEDGB_Mask;
      }
      else
      {
        TIMx->CR1 &= ~TIM_IEDGB_Mask;
      }
      break;
  }
}

/*******************************************************************************
* Function Name  : TIM_ICAPValue
* Description    : This routine is used to get and return the Input Capture 
*                  value.
* Input          : - TIMx: specifies the TIM to check its Input Capture value.
*                  - Xchannel: specifies the Input Capture channel, it can be:
*                     TIM_Channel_A, TIM_Channel_B 
* Output         : None.
* Return         : The input capture value of the specified timer and channel.
*******************************************************************************/
u16 TIM_ICAPValue (TIM_TypeDef *TIMx, TIM_Channels Xchannel)
{
  if (Xchannel == TIM_CHANNEL_A)
  {
    return TIMx->ICAR;	
  } 	
  else
  {
    return TIMx->ICBR;	
  }
}

/*******************************************************************************
* Function Name  : TIM_OCMPModeConfig
* Description    : This routine is used to configure the output compare mode.
* Input          : - TIMx: specifies the TIM to be configured.
*                  - Xchannel: specifies the output compare channel, it can be:
*                     TIM_CHANNEL_A, TIM_CHANNEL_B
*                  - XpulseLength: specifies the pulse length.
*                  - Xmode: specifies the output compare mode, it can be:
*                      TIM_TIMING, TIM_WAVE 
*                  - Xlevel: specifies the level of the external signal after
*                    the match occurs, it can be:
*                      TIM_HIGH, TIM_LOW 
* Output         : None.
* Return         : None.
*******************************************************************************/
void TIM_OCMPModeConfig (TIM_TypeDef  *TIMx, TIM_Channels Xchannel,
                         u16 XpulseLength, TIM_OC_Modes Xmode,
                         TIM_Logic_Levels Xlevel)
{
  u16 Tmp1 = 0x0000;
  u16 Tmp2 = TIMx->CR2;

  TIMx->CR2 = 0x0000;
  /* Start The TIM Counter */
  TIMx->CR1  = TIM_EN_Mask;
  /* Update the CR2 Register */
  TIMx->CR2  = Tmp2;
  switch (Xmode)
  {
    case TIM_TIMING:
      /* Output Compare Used only for Internal Timing Operation */
       if (Xchannel == TIM_CHANNEL_A)
       {
         Tmp1 &= ~TIM_OCAE_Mask;
       }
       else
       {
       	 Tmp1 &= ~TIM_OCBE_Mask;
       }
      break;
      
    case TIM_WAVE:
      /* Output Compare Used for external wave generation */
       if (Xchannel == TIM_CHANNEL_A)
       {
         Tmp1 = TIM_OCAE_Mask;
       }
       else
       {
       	 Tmp1 = TIM_OCBE_Mask;
       } 
            
      if (Xlevel == TIM_HIGH)
      {
      	if (Xchannel == TIM_CHANNEL_A)
      	{
      	  Tmp1 |= TIM_OLVLA_Mask;
        }
        else
        {
          Tmp1 |= TIM_OLVLB_Mask;	
        }
      }
      else
      {
      	if (Xchannel == TIM_CHANNEL_A)
      	{
      	  Tmp1 &= ~TIM_OLVLA_Mask;	
      	}
      	else
      	{
      	  Tmp1 &= ~TIM_OLVLB_Mask;	
      	}
      }
      break;
      
    default:
      break;
  }
  
  if (Xchannel == TIM_CHANNEL_A)
  {
    TIMx->OCAR = (XpulseLength);
  }
  else
  {
    TIMx->OCBR = (XpulseLength);
  }
  TIMx->CNTR = 0x0000;
  TIMx->CR1 |= Tmp1;
}

/*******************************************************************************
* Function Name  : TIM_OPModeConfig
* Description    : This routine is used to configure the one pulse mode.
* Input          : - TIMx: specifies the TIM to be configured.
*                  - XpulseLength: specifies the pulse length.
*                  - XLevel1: specifies the output level on the OCMPA pin during
*                    the pulse it can be:
*                     TIM_HIGH, TIM_LOW
*                  - XLevel2: specifies the output level on the OCMPB pin after
*                    the pulse it can be:
*                     TIM_HIGH, TIM_LOW
*                  - Xedge:  specifies the edge to be detected by the input 
*                    capture A pin it can be:
*                     TIM_RISING, TIM_FALLING
* Output         : None.
* Return         : None.
*******************************************************************************/
void TIM_OPModeConfig (TIM_TypeDef *TIMx, u16 XpulseLength,
                       TIM_Logic_Levels XLevel1, TIM_Logic_Levels XLevel2,
                       TIM_Clock_Edges Xedge)
{
  u16 Tmp = 0;

  /* Set the Level During the pulse */
  if (XLevel1 == TIM_HIGH)
  {
    Tmp |= TIM_OLVLB_Mask;
  }
  
  /* Set the Level after After the pulse */
  if (XLevel2 == TIM_HIGH)
  {
    Tmp |= TIM_OLVLA_Mask;
  }
  
  /* Set the Activation Edge on the INCAP 1 */  /* to be verified*/
  if (Xedge == TIM_RISING)
  {
    Tmp |= TIM_IEDGA_Mask;
  }
  
  /* Set the Output Compare Function */
  Tmp |= TIM_OCAE_Mask;
  
  /* Set the One pulse mode */
  Tmp |= TIM_OPM_Mask;
  
  /* Update the CR1 register Value */
  TIMx->CR1 = Tmp;
  
  /* Set the Pulse length */
  TIMx->OCAR = XpulseLength;
}

/*******************************************************************************
* Function Name  : TIM_PWMOModeConfig
* Description    : This routine is used to configure the PWM output mode.
* Input          : - TIMx: specifies the TIM to be configured.
*                  - XDutyCycle: specifies the PWM signal duty cycle.
*                  - XLevel1: specifies the PWM signal level during the duty
*                    cycle, it can be:
*                     TIM_HIGH, TIM_LOW
*                  - XFullperiod: specifies the PWM signal full period.
*                  - XLevel2: specifies the PWM signal level out of the duty
*                    cycle, it can be:
*                     TIM_HIGH, TIM_LOW
* Output         : None.
* Return         : None.
*******************************************************************************/
void TIM_PWMOModeConfig (TIM_TypeDef *TIMx, u16 XDutyCycle,
                         TIM_Logic_Levels XLevel1, u16 XFullperiod,
                         TIM_Logic_Levels XLevel2)
{
  vu16 Tmp = TIMx->CR1;
  
  /* Set the level during the pulse */
  if (XLevel1 == TIM_HIGH)
  {
    Tmp |= TIM_OLVLB_Mask;	
  }
  else
  {
    Tmp &= ~TIM_OLVLB_Mask;	
  }
  
  /* Set the level after the pulse */
  if (XLevel2 == TIM_HIGH)
  {
    Tmp |= TIM_OLVLA_Mask;
  }
  else
  {
    Tmp &= ~TIM_OLVLA_Mask;	
  }

  /* Set the OCAE */
  Tmp |= TIM_OCAE_Mask;
  
  /* Set the PWM Bit */
  Tmp |= TIM_PWM_Mask;
  
  /* Update the CR1 */
  TIMx->CR1 = Tmp;
  
  /* Set the Duty Cycle value */
  if (XDutyCycle < 5)
  {
    XDutyCycle = 4;
  }
  
  TIMx->OCAR = XDutyCycle - 4;
  
  /* Set the Full Period */
  TIMx->OCBR = XFullperiod - 4;
}

/*******************************************************************************
* Function Name  : TIM_PWMIModeConfig
* Description    : This routine is used to configure the PWM input mode.
* Input          : - TIMx: specifies the TIM to be configured.
*                : - Xedge: specifies the first edge of the external PWM signal.
*                    It can be:
*                     TIM_RISING, TIM_FALLING
* Output         : None.
* Output         : None.
*******************************************************************************/
void TIM_PWMIModeConfig (TIM_TypeDef  *TIMx,  TIM_Clock_Edges  Xedge)
{
  vu16 Tmp = TIMx->CR1;
  
  if (Xedge == TIM_RISING)
  {
    Tmp |= TIM_IEDGA_Mask;	
  }
  else
  {
    Tmp &= ~TIM_IEDGA_Mask;
  }
  
  /* Set the first edge Level */
  /* Set the Second edge Level (Opposit of the first level)*/
  if (Xedge == TIM_FALLING)
  {
    Tmp |= TIM_IEDGB_Mask;
  }
  else
  {
    Tmp &= ~TIM_IEDGB_Mask;	
  }

  /* Set the PWM I Bit */
  Tmp |= TIM_PWMI_Mask;

  /* Update the CR1 */
  TIMx->CR1 = Tmp;
}

/*******************************************************************************
* Function Name  : TIM_PWMIValue
* Description    : This routine is used to get the PWMI values.
* Input          : - TIMx: specifies the TIM to get its PWM parameters values.
* Output         : None.
* Return         : The PWM input parameters: pulse and period.
*******************************************************************************/
PWMI_parameters TIM_PWMIValue (TIM_TypeDef  *TIMx)
{
  PWMI_parameters Tmp;
  
  Tmp.Pulse  = TIMx->ICBR;
  Tmp.Period = TIMx->ICAR;
  return Tmp;
}

/*******************************************************************************
* Function Name  : TIM_CounterConfig
* Description    : This routine is used to start/stop and clear the selected 
*                  timer counter.
* Input          : - TIMx: specifies the TIM to be configured.
*                  - Xoperation: specifies the operation of the counter.
*                    It can be:
*                     TIM_START, TIM_STOP, TIM_CLEAR
* Output         : None.
* Return         : None.
*******************************************************************************/
void TIM_CounterConfig (TIM_TypeDef  *TIMx, TIM_CounterOperations Xoperation)
{
  switch (Xoperation)
  {
    case TIM_START :
      TIMx->CR1 |= TIM_EN_Mask;
      break;
      
    case TIM_STOP :
      TIMx->CR1 &= ~TIM_EN_Mask;
      break;
      
    case TIM_CLEAR :
      TIMx->CNTR = 0x1234;
      break;
      
    default:
      break;  
  }
}

/*******************************************************************************
* Function Name  : TIM_ITConfig
* Description    : This routine is used to configure the TIM interrupt.
* Input          : - TIMx: specifies the TIM to be configured.
*                  - New_IT: specifies the TIM interrupt to be configured.
*                    You can specify one or more TIM interrupts to be configured
*                    using the logical operator 'OR'. It can be:
*                     TIM_OCBIE, TIM_ICBIE, TIM_TOE, TIM_OCAIE, TIM_ICAIE
*                  - NewState: specifies the TIM interrupt state whether it
*                    would be enabled or disabled, it can be:
*                     ENABLE: the corresponding TIM interrupt(s) will be enabled.
*                     DISABLE: the corresponding TIM interrupt(s) will be
*                     disabled.
* Output         : None.
* Return         : None.
*******************************************************************************/
void TIM_ITConfig (TIM_TypeDef *TIMx, u16 New_IT, FunctionalState NewState)
{
   if (NewState == ENABLE)
   {
    TIMx->CR2 |= New_IT;
   }
   else
   {
    TIMx->CR2 &= ~New_IT;
   }
}

/*******************************************************************************
* Function Name  : TIM_FlagStatus
* Description    : Specifies the TIM to check a flag.
* Input          : - TIMx: specifies the TIM to check a flag.
*                  - Xflag: specifies the TIM flag to be tested. It can be:
*                     TIM_ICFA, TIM_OCFA, TIM_TOF,TIM_ICFB, TIM_OCFB
* Output         : None.
* Return         : The flag status passed in parameter.
*******************************************************************************/
FlagStatus TIM_FlagStatus (TIM_TypeDef *TIMx, TIM_Flags Xflag)
{
  if ((TIMx->SR & Xflag) == 0)
  {
    return RESET;	 
  }
  else
  {
    return SET;	
  }
}

/*******************************************************************************
* Function Name  : TIM_FlagClear
* Description    : This routine is used to clear the TIM flags.
* Input          : - TIMx: specifies the TIM to clear a flag.
* Output         : None.
* Return         : None.
*******************************************************************************/
void TIM_FlagClear (TIM_TypeDef *TIMx, TIM_Flags Xflag)
{
  TIMx->SR &= ~Xflag;
}

/*******************************************************************************
* Function Name  : TIM_CounterValue
* Description    : This routine returns the timer counter value.
* Input          : - TIMx: specifies the TIM to get its counter value.
* Output         : None.
* Return         : The selected timer counter value.
*******************************************************************************/
u16 TIM_CounterValue(TIM_TypeDef *TIMx)
{
  return TIMx->CNTR;
}

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
