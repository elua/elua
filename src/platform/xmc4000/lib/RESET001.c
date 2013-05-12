
/*CODE_BLOCK_BEGIN[RESET001.c]*/

/*******************************************************************************
 Copyright (c) 2011, Infineon Technologies AG                                 **
 All rights reserved.                                                         **
                                                                              **
 Redistribution and use in source and binary forms, with or without           **
 modification,are permitted provided that the following conditions are met:   **
                                                                              **
 *Redistributions of source code must retain the above copyright notice,      **
 this list of conditions and the following disclaimer.                        **
 *Redistributions in binary form must reproduce the above copyright notice,   **
 this list of conditions and the following disclaimer in the documentation    **
 and/or other materials provided with the distribution.                       **
 *Neither the name of the copyright holders nor the names of its contributors **
 may be used to endorse or promote products derived from this software without** 
 specific prior written permission.                                           **
                                                                              **
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"  **
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE    **
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE   **
 ARE  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE   **
 LIABLE  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR         **
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF         **
 SUBSTITUTE GOODS OR  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    **
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN      **
 CONTRACT, STRICT LIABILITY,OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)       **
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE   **
 POSSIBILITY OF SUCH DAMAGE.                                                  **
                                                                              **
 To improve the quality of the software, users are encouraged to share        **
 modifications, enhancements or bug fixes with Infineon Technologies AG       **
 dave@infineon.com).                                                          **
                                                                              **
********************************************************************************
**                                                                            **
**                                                                            **
** PLATFORM : Infineon XMC4000 Series   			              **
**                                                                            **
** COMPILER : Compiler Independent                                            **
**                                                                            **
** AUTHOR : $Author:                                                          **
**                                                                            **
** MAY BE CHANGED BY USER [yes/no]: Yes                                       **
**                                                                            **
** MODIFICATION DATE : Aug 19, 2011                                           **
**                                                                            **
*******************************************************************************/

/******************************************************************************
**                      Author(s) Identity                                   **
*******************************************************************************
**                                                                           **
** Initials     Name                                                         **
** --------------------------------------------------------------------------**
** BT         Bini Thaliyath                                                 **
**                                                                           **
*******************************************************************************/
/**
 * @file RESET001.c
 *
 * @brief  Reset App implementation.
 *
 */
/*******************************************************************************
 ** INCLUDE FILES                                                             **
 ******************************************************************************/


/** Inclusion of header file */
#include <DAVE3.h>


/*******************************************************************************
**                      Private Macro Definitions                             **
*******************************************************************************/
/* Invalid Input */
#define RCU_INVALID_INPUT  0x0000000FU
/** To check bit value is 1 */ 
#define RCU_SET   1U
/** To check bit value is 0 */
#define RCU_RESET   0U
/** Clear BitMask  */
#define CLEAR_BITMASK	0xF0000000U
/** Reset BitMask */
#define RESET_BITMASK   0x0FFFFFFFU

/** @ingroup RESET001 Func
 * @{
 */

/*******************************************************************************
**                      Public Function Definitions                           **
*******************************************************************************/


/** @ingroup RESET001 Func
 * @{
 */
 
 /* This function is to enable reset of peripheral by software  */
 
 void RESET001_AssertReset(RESET001_ResetnType Resetn)
 {
  uint32_t* RCUControlReg = 0 ;
  uint32_t Temp = 0 ;
  /* <<<DD_RESET001_API_1>>> */
  SCU_RESET_TypeDef* RCUCtrlReg = SCU_RESET;
  Temp = ((uint32_t)Resetn & CLEAR_BITMASK) >> 28;
  RCUControlReg = (uint32_t*)((uint32_t)(&RCUCtrlReg->PRSET0) + ( 0x0000000CU * Temp));
  *RCUControlReg = ((uint32_t)Resetn & RESET_BITMASK );
}

/* API to get the  reason of last reset */	

void RESET001_DeassertReset(RESET001_ResetnType Resetn)
{
  uint32_t* RCUControlReg = 0;
  uint32_t Temp = 0;
  /* <<<DD_RESET001_API_2>>> */
  SCU_RESET_TypeDef* RCUCtrlReg = SCU_RESET;  
  Temp = ((uint32_t)Resetn & CLEAR_BITMASK) >> 28;
  RCUControlReg = (uint32_t*)((uint32_t)(&RCUCtrlReg->PRCLR0) + ( 0x0000000CU * Temp));
  *RCUControlReg = ((uint32_t)Resetn & RESET_BITMASK) ;
}
	
/* API to get the  reason of last reset. */
RESET001_InfoType RESET001_GetResetInfo()
{
  SCU_RESET_TypeDef* RCUCtrlReg = SCU_RESET;
 /* <<<DD_RESET001_API_4>>> */
  return (RCUCtrlReg->RSTSTAT  &  SCU_RESET_RSTSTAT_RSTSTAT_Msk);
}
		
/* Get the reset status of the peripheral */		
status_t RESET001_GetStatus(RESET001_ResetnType Resetn)
{
  uint32_t status = RCU_INVALID_INPUT;
  uint32_t* RCUControlReg = 0;
  uint32_t Temp = 0 ;
  uint32_t Getstatus = 0;
  /* <<<DD_RESET001_API_3>>> */
  SCU_RESET_TypeDef* RCUCtrlReg = SCU_RESET;
  Temp = ((uint32_t)Resetn & CLEAR_BITMASK) >> 28;	
  RCUControlReg = (uint32_t*)((uint32_t)(&RCUCtrlReg->PRSTAT0) + ( 0x0000000CU * Temp));
  Getstatus	 = (*RCUControlReg) & (uint32_t)Resetn & RESET_BITMASK ;  			 
  if(Getstatus != 0)
  {
	  status = RCU_SET;
  }
  else
  {
	  status = RCU_RESET;
  }
  return status;
}
 
 
/**
*@}
*/





/*CODE_BLOCK_END*/