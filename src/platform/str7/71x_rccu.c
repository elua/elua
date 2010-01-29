/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : 71x_rccu.c
* Author             : MCD Application Team
* Version            : V4.0
* Date               : 10/09/2007
* Description        : This file provides all the RCCU firmware functions.
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "71x_rccu.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : RCCU_Div2Config
* Description    : This routine is used to enable or disable the programmable
*                  clock division of the CLOCK1 input clock signal by 2. It 
*                  sets or clears the Div2 flag in the CLK_FLAG register.
* Input          : - NewState: specifies whether the programmable divider can 
*                     divide the CLOCK1 input clock signal by two or not.
*                     it can be: 
*                     ENABLE to enable the division by two of CLK signal. 
*                     DISABLE to disable the division by two of CLK signal.
* Output         : None.
* Return         : None.
*******************************************************************************/
void RCCU_Div2Config (FunctionalState NewState)
{
  /* CLK is divided by 2 */	
  if (NewState == ENABLE)
  {
    RCCU->CFR |= RCCU_Div2_Mask;
  }  
  /* No division of CLK frequency */
  else
  {
    RCCU->CFR &= ~RCCU_Div2_Mask;
  }
}

/*******************************************************************************
* Function Name  : RCCU_Div2Status
* Description    : This routine is used to get the current status of the
*                  programmable clock division by two. It checks the status 
*                  of the Div2 flag in the CLK_FLAG register.
* Input          : None.
* Output         : None.
* Return         : The Div2 Flag status.
*******************************************************************************/
FlagStatus RCCU_Div2Status (void)
{
  if ((RCCU->CFR & RCCU_Div2_Mask) == 0)
  {
    return RESET;
  }
  else
  {
    return SET;	
  }
}

/*******************************************************************************
* Function Name  : RCCU_MCLKConfig
* Description    : This routine is used to configure the MCLK clock divider.
* Input          : - New_Clock: Specifies the MCLK clock divider value. 
*                     it can be: 
*                     RCCU_DEFAULT, RCCU_RCLK_2, RCCU_RCLK_4, RCCU_RCLK_8                   
* Output         : None.
* Return         : None.
*******************************************************************************/
void RCCU_MCLKConfig (RCCU_Clock_Div New_Clock)
{
  PCU->MDIVR = (PCU->MDIVR & ~RCCU_FACT_Mask) | New_Clock;
}

/*******************************************************************************
* Function Name  : RCCU_PCLK1Config
* Description    : Selects the division factor for RCLK to obtain the
*                  PCLK1 clock for the APB1 fast peripherals (PCLK1).
* Input          : - New_Clock : the new PCLK1 clock prescaler division value
*                     it can be: 
*                     RCCU_DEFAULT, RCCU_RCLK_2, RCCU_RCLK_4, RCCU_RCLK_8
* Output         : None.
* Return         : None.
*******************************************************************************/
void RCCU_PCLK1Config (RCCU_Clock_Div New_Clock)
{
  PCU->PDIVR = (PCU->PDIVR & ~RCCU_FACT1_Mask) | New_Clock;
}

/*******************************************************************************
* Function Name  : RCCU_PCLK2Config
* Description    : Selects the division factor for RCLK to obtain the
*                  PCLK2 clock for the APB2 peripherals (PCLK2).
* Input          : - New_Clock : the new PCLK2 clock prescaler division value
*                     it can be: 
*                     RCCU_DEFAULT, RCCU_RCLK_2, RCCU_RCLK_4, RCCU_RCLK_8
* Output         : None.
* Return         : None.
*******************************************************************************/
void RCCU_PCLK2Config (RCCU_Clock_Div New_Clock)
{
  PCU->PDIVR = (PCU->PDIVR & ~RCCU_FACT2_Mask) | (New_Clock << RCCU_FACT2_Index);
}

/*******************************************************************************
* Function Name  : RCCU_PLL1Config
* Description    : Configures the PLL1 division and multiplication factors.
* Input          : - New_Mul: the PLL1 multiplication factor it can be: 
*                     RCCU_PLL1_Mul_12, RCCU_PLL1_Mul_16,RCCU_PLL1_Mul_20,
*                     RCCU_PLL1_Mul_24
*                  - New_Div: the PLL1 division factor it can be: 
*                     RCCU_Div_1, RCCU_Div_2, RCCU_Div_3, RCCU_Div_4,RCCU_Div_5,
*                     RCCU_Div_6, RCCU_Div_7
* Output         : None.
* Return         : None.
*******************************************************************************/
void RCCU_PLL1Config (RCCU_PLL1_Mul New_Mul, RCCU_PLL_Div New_Div)
{
  /* Turn off the PLL1 by setting bits DX[2:0] */
  RCCU->PLL1CR = 0x0007;
   
  /* Set the PLL1 multiplication factor */
  RCCU->PLL1CR =  New_Mul << RCCU_MX_Index;

  /* Set the PLL1 division factor */
  RCCU->PLL1CR |=  New_Div;

  /* Set the PLL1 FREF_RANGE bit according to the PLL1 input frequency */
  if (RCCU_FrequencyValue(RCCU_CLK2) > 3000000)
  {
    RCCU->PLL1CR |= RCCU_FREFRANGE_Mask;
  }
   
  /* Set the CK2_16 Bit in the CFR to allow PLL1 to be selected */
  RCCU->CFR |= RCCU_CK2_16_Mask;
}

/*******************************************************************************
* Function Name  : RCCU_PLL2Config
* Description    : Configures the PLL2 division and multiplication factors.
* Input          : - New_Mul: the PLL2 multplication factor it can be:
*                     RCCU_PLL2_Mul_12, RCCU_PLL2_Mul_16, RCCU_PLL2_Mul_20,
*                     RCCU_Mul_PLL2_28
*                : - New_Div: the PLL2 division factor it can be:
*                     RCCU_Div_1, RCCU_Div_2, RCCU_Div_3, RCCU_Div_4,
*                     RCCU_Div_5, RCCU_Div_6, RCCU_Div_7
*                  - HCLK_Clock: the clock value present on HCLK pin (in Hz).  
* Output         : None.
* Return         : None.
*******************************************************************************/
void RCCU_PLL2Config (RCCU_PLL2_Mul New_Mul, RCCU_PLL_Div New_Div,
                       u32 HCLK_Clock)
{
  u32 Tmp = (PCU->PLL2CR & ~RCCU_MX_Mask) | (New_Mul << RCCU_MX_Index);
  PCU->PLL2CR = (Tmp & ~RCCU_DX_Mask) | (New_Div  | RCCU_PLLEN_Mask);
  
  /* Set the PLL2 FRQRNG bit according to the PLL2 input frequency */
  if (HCLK_Clock < 3000000)
  {
    PCU->PLL2CR &= ~RCCU_FRQRNG_Mask;
  }
  else
  {
    PCU->PLL2CR |= RCCU_FRQRNG_Mask;
  }
}

/*******************************************************************************
* Function Name  : RCCU_RCLKSourceConfig
* Description    : Selects the clock source for the RCLK.
* Input          : - New_Clock: the RCLK clock source, it can be:
*                     RCCU_PLL1_Output, RCCU_CLOCK2_16, RCCU_CLOCK2, RCCU_CK_AF                               
* Output         : None.
* Return         : None.
*******************************************************************************/
void RCCU_RCLKSourceConfig (RCCU_RCLK_Clocks New_Clock)
{
  switch (New_Clock)
  {
    case RCCU_CLOCK2:
      /* Reset the CSU_CKSEL bit: CLK2 provides the system clock */
      RCCU->CFR &= ~RCCU_CSU_CKSEL_Mask;
      /* Set the CK2_16 Bit in the CFR */
      RCCU->CFR |= RCCU_CK2_16_Mask;
      /* Deselect The CKAF */
      RCCU->CCR   &= ~RCCU_CKAF_SEL_Mask;
      /* Switch off the PLL1 */
      RCCU_PLL1Disable();
      break;
    
    case RCCU_CLOCK2_16:
      /* Reset the CK2_16 Bit in the CFR */
      RCCU->CFR &= ~RCCU_CK2_16_Mask;
      /* Deselect The CKAF */
      RCCU->CCR   &= ~RCCU_CKAF_SEL_Mask;
      break;

    case RCCU_PLL1_Output:
      /* Set the CK2_16 Bit in the CFR */
      RCCU->CFR |= RCCU_CK2_16_Mask;
      
      /* If all DX bit are set the PLL lock flag in meaningless */
      if ((RCCU->PLL1CR & 0x0007) != 7)
      {
      	/* Waits the PLL1 to lock if DX bits are different from '111' */
        while (!(RCCU->CFR & RCCU_LOCK_Mask))
        {}
      }
      /* Deselect The CKAF */
      RCCU->CCR  &= ~RCCU_CKAF_SEL_Mask;
      /* Select The CSU_CKSEL */
      RCCU->CFR |= RCCU_CSU_CKSEL_Mask;
      break;
    
    case RCCU_CK_AF:
      /* Set CKAF_SEL bit */
      RCCU->CCR |= 0x04;
      /* Switch off the PLL1 */
      RCCU_PLL1Disable();
      break;
    
  }
}

/*******************************************************************************
* Function Name  : RCCU_RCLKClockSource
* Description    : Gets and returns the current clock source of the RCLK.
* Input          : None.
* Output         : None.
* Return         : The current RCLK clock source which can be:
*                  RCCU_PLL1_Output, RCCU_CLOCK2_16, RCCU_CLOCK2, RCCU_CK_AF
*******************************************************************************/
RCCU_RCLK_Clocks RCCU_RCLKClockSource (void)
{
  /* RCLK = CK_AF */
  if (RCCU->CCR & RCCU_CKAF_SEL_Mask)
  {
    return RCCU_CK_AF;
  }
  
  /* RCLK = CLK2/16 */
  else if ((RCCU->CFR & RCCU_CK2_16_Mask) == 0)
  {
    return RCCU_CLOCK2_16;
  }
  
  /* RCLK = PLL1 output */
  else if (RCCU->CFR & RCCU_CSU_CKSEL_Mask)
  {
    return RCCU_PLL1_Output;
  }
  
  /* RCLK = CLK2 */
  else 
  {
    return RCCU_CLOCK2;
  }
}

/*******************************************************************************
* Function Name  : RCCU_USBCLKConfig
* Description    : Selects the clock source of the USB peripheral.
* Input          : - New_Clock: the USB clock source it can be:
*                     RCCU_PLL2_Output, RCCU_USBCK
* Output         : None.
* Return         : None.
*******************************************************************************/
void RCCU_USBCLKConfig (RCCU_USB_Clocks New_Clock)
{
  /* PLL2 is the clock source to the USB */
  if (New_Clock == RCCU_PLL2_Output)
  {
    PCU->PLL2CR |= RCCU_USBEN_Mask;
  }
  
  /* USBCLK pin is the clock source to the USB */
  else 
  {
    PCU->PLL2CR &= ~RCCU_USBEN_Mask;
  }
}

/*******************************************************************************
* Function Name  : RCCU_USBClockSource
* Description    : Get the clock source of the USB peripheral.
* Input          : None.
* Output         : None.
* Return         : RCCU_USB_Clocks
*                  it can be: RCCU_PLL2_Output, RCCU_USBCK
*******************************************************************************/
RCCU_USB_Clocks RCCU_USBClockSource (void)
{
  if (PCU->PLL2CR & RCCU_USBEN_Mask)
  {
    return RCCU_PLL2_Output;
  }
  else
  {
    return RCCU_USBCK;
  }
}

/*******************************************************************************
* Function Name  : RCCU_FrequencyValue
* Description    : Computes and Returns any internal RCCU clock frequency
*                  value passed in parametres.
* Input          : - Internal_Clk: the RCCU internal clock to compute the
*                    frequency.
*                     it can be:
*                     RCCU_CLK2, RCCU_RCLK, RCCU_MCLK, RCCU_PCLK2, RCCU_PCLK1
* Output         : None.
* Return         : The frequency value of the specified clock in Hz of the 
*                  internal clock passed in parameter.
*******************************************************************************/
u32 RCCU_FrequencyValue (RCCU_Clocks Internal_Clk)
{
  u32 Tmp;
  u8 Div = 0;
  u8 Mul = 0;
  RCCU_RCLK_Clocks CurrentRCLK;
 
  if (RCCU_Div2Status() == SET)
  {
    Tmp = RCCU_Main_Osc / 2;	
  }
  else
  {
    Tmp = RCCU_Main_Osc;
  }

  if (Internal_Clk == RCCU_CLK2)
  {
    Div = 1;
    Mul = 1;
  }
  
  else
  {
    CurrentRCLK = RCCU_RCLKClockSource ();
    switch (CurrentRCLK)
    {
      case RCCU_CLOCK2_16 :
        Div = 16;
        Mul = 1;
        break;
        
      case RCCU_CLOCK2 :
        Div = 1;
        Mul = 1;
        break;
        
      case RCCU_PLL1_Output :
      
       if ((RCCU->PLL1CR & RCCU_DX_Mask)&&(RCCU->PLL1CR & RCCU_FREEN_Mask))
        {
          if (RCCU->PLL1CR & 0x10)
          {
            if (RCCU->PLL1CR & RCCU_FREFRANGE_Mask)
            {
              Tmp = 250000;	
            }
            else
            {
              Tmp = 125000;	
            }
          }
          else
          {
            if (RCCU->PLL1CR & RCCU_FREFRANGE_Mask)
            {
              Tmp = 500000;	
            }
            else
            {
              Tmp = 250000;	
            }
          }
          
          Div = 1;
          Mul = 1;
        }
        
       else
       {
         Mul = (RCCU->PLL1CR & RCCU_MX_Mask) >> RCCU_MX_Index;
        
         switch (Mul)
         {
           case 0:
             Mul = 20;
             break;
            
           case 1:
             Mul = 12;
             break;
            
           case 2:
             Mul = 24;
             break;
            
           case 3:
             Mul = 16;
             break;
         }
        
        Div = (RCCU->PLL1CR & RCCU_DX_Mask) + 1;
        }
        break;
        
      case RCCU_CK_AF :
        Mul = 1;
        Div = 1;
        Tmp = RCCU_RTC_Osc;
        break;
    }
  }

  switch (Internal_Clk)
  {
    case RCCU_MCLK :
      Div <<= PCU->MDIVR & RCCU_FACT_Mask;
      break;
      
    case RCCU_PCLK2 :
      Div <<= (PCU->PDIVR & RCCU_FACT2_Mask) >> RCCU_FACT2_Index;
      break;

    case RCCU_PCLK1 :
      Div <<=  PCU->PDIVR & 0x3;
      break;
  }

  return (Tmp * Mul) / Div;
}

/*******************************************************************************
* Function Name  : RCCU_ITConfig
* Description    : Configures the RCCU interrupts
* Input          : - RCCU_IT: the RCCU interrption source it can be:
*                     RCCU_CK2_16_IT, RCCU_CKAF_IT, RCCU_PLL1_LOCK_IT,
*                     RCCU_STOP_IT
*                  - NewState: the new status of the RCCU interrupt.
*                     it can be:
*                     ENABLE: to enable the specified interrupt.
*                     DISABLE: to disable the specified interrupt.
* Output         : None.
* Return         : None.
*******************************************************************************/
void RCCU_ITConfig (RCCU_Interrupts RCCU_IT, FunctionalState NewState)
{
  if (NewState == ENABLE)
  {
    RCCU->CCR |= RCCU_IT;
  }
  else
  {
    RCCU->CCR &= ~RCCU_IT;
  }
}

/*******************************************************************************
* Function Name  : RCCU_FlagStatus
* Description    : Checks whether the specified interrupt is enabled or
*                  disabled.
* Input          : - RCCU flag: the flag to see its status it can be:
*                     RCCU_CK2_16_I, RCCU_CKAF_I, RCCU_PLL1_LOCK_I,
*                     RCCU_CKAF_ST, RCCU_PLL1_LOCK, RCCU_STOP_I
* Output         : None.
* Return         : The flag status: SET or RESET
*******************************************************************************/
FlagStatus RCCU_FlagStatus (RCCU_Flags RCCU_flag)
{
  if (RCCU->CFR & RCCU_flag)
  {
    return SET;
  }
  else
  {
    return RESET;
  }
}

/*******************************************************************************
* Function Name  : RCCU_FlagClear
* Description    : Clears the specified interrupt flag in the RCCU registers 
*                  passed in parameter.
* Input          : - RCCU_Flags: the flag wich will be cleared it can be:
*                     RCCU_PLL1_LOCK_I, RCCU_CKAF_I, RCCU_CK2_16_I, RCCU_STOP_I
* Output         : None.
* Return         : None.
*******************************************************************************/
void RCCU_FlagClear (RCCU_Flags RCCU_flag)
{
  RCCU->CFR |= RCCU_flag;
}

/*******************************************************************************
* Function Name  : RCCU_ResetSources
* Description    : Returns the system reset source.
* Input          : None.
* Output         : None.
* Return         : The reset source.
*******************************************************************************/
RCCU_ResetSources RCCU_ResetSource (void)
{
  switch (RCCU->CFR & RCCU_ResetSources_Mask)
  {
    case 0x00000020:
      return RCCU_SoftwareReset;
    case 0x00000040:
      return RCCU_WDGReset;
    case 0x00000080:
      return RCCU_RTCAlarmReset;
    case 0x00000200:
      return RCCU_LVDReset;
    case 0x00000400:
      return RCCU_WKPReset;
    default :
      return RCCU_ExternalReset;
  }
}

/*******************************************************************************
* Function Name  : RCCU_PLL1FreeRunningModeConfig
* Description    : This function configures the PLL1 in free runnig mode.
* Input          : - NewPll1FreeRunningMode: the new free running mode 
*                     it can be:
*                      RCCU_PLL1FreeRunning125, RCCU_PLL1FreeRunning250,
*                      RCCU_PLL1FreeRunning500 
* Output         : None.
* Return         : None.
*******************************************************************************/
void RCCU_PLL1FreeRunningModeConfig(RCCU_PLL1FreeRunningModes
                                    NewPll1FreeRunningMode)
{
  /* bits DX[2:0] (RCCU_PLL1CR) are set to 111, the PLL1 is switched off */
  RCCU->PLL1CR = RCCU_DX_Mask; 
                                    

  switch (NewPll1FreeRunningMode)
  {
    case RCCU_PLL1FreeRunning250:
    {
         RCCU->PLL1CR |= RCCU_FREEN_Mask | RCCU_FREFRANGE_Mask 
                         | (3 << RCCU_MX_Index);              
      break;
    }
    case RCCU_PLL1FreeRunning500:
    {
       RCCU->PLL1CR |= RCCU_FREEN_Mask | RCCU_FREFRANGE_Mask 
                       | (2 << RCCU_MX_Index);               
      break;
    }
    default:
    {
        RCCU->PLL1CR |= RCCU_FREEN_Mask | (3 << RCCU_MX_Index);                
      break;
    }
  }
}

/*******************************************************************************
* Function Name  : RCCU_PLL1Disable
* Description    : This function switch off the PLL1.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void RCCU_PLL1Disable(void)
{
  /* Stop the PLL1 DX[0..2]=111 and FREEN=0 */
  RCCU->PLL1CR = RCCU_DX_Mask;
}

/*******************************************************************************
* Function Name  : RCCU_PLL2Disable
* Description    : This function switch off the PLL2.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void RCCU_PLL2Disable(void)
{
  /* Stop the PLL2 DX[0..2]=111 */
  PCU->PLL2CR |= RCCU_DX_Mask;
}

/*******************************************************************************
* Function Name  : RCCU_GenerateSWReset
* Description    : This function generates software reset.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void RCCU_GenerateSWReset(void)
{
  /* SRESEN = 1  and EN_HALT = 1 */	
  RCCU->CCR |=0x808;   
  
  /* HALT =1   ==> software reset generation */
  RCCU->SMR |=0x2;  
}

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
