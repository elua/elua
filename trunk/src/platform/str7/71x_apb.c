/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : 71x_apb.c
* Author             : MCD Application Team
* Version            : V4.0
* Date               : 10/09/2007
* Description        : This file provides all the APB firmware functions.
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "71x_apb.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : APB_ClockConfig
* Description    : Enables/Disables the peripheral clock gating on the specified
*                  APB bridge.
* Input          : APBx: selects the APB bridge (APB1 or APB2)
*                  NewState:
*                  ENABLE : The peripheral clock is running
*                  DISABLE : The peripheral clock is stopped
*                  Periph: specifies the APB bridge peripheral. 
* Output         : None.                    
* Return         : None.
*******************************************************************************/
void APB_ClockConfig (APB_TypeDef *APBx, FunctionalState NewState,
                      u16 Periph)
{
  if (NewState == ENABLE)
  {
    APBx->CKDIS &= ~Periph;
  }
  else
  {
    APBx->CKDIS |= Periph;
  }
}

/*******************************************************************************
* Function Name  : APB_SwResetConfig
* Description    : Enables/disables the software reset for peripherals on the 
                   specified APB bridge.
* Input          : APBx: selects the APB bridge (APB1 or APB2)
*                  ENABLE : The peripheral is kept under reset
*                  DISABLE : The peripheral is reset by the system-wide reset
*                  Periph: specifies the APB bridge peripheral. 
* Output         : None.
* Return         : None.
*******************************************************************************/
void APB_SwResetConfig (APB_TypeDef *APBx, FunctionalState NewState,
                        u16 Periph)
{
  if (NewState == ENABLE)
  {
    APBx->SWRES |= Periph;
  }
  else
  {
    APBx->SWRES &= ~Periph;
  }
}

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
