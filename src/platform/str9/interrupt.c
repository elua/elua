/********************************************************************
 * Project:    GNU-Str9-ComStick
 * File:       interrupt.c
 *
 * System:     ARM9TDMI 32 Bit (STR912FW44X)
 * Compiler:   GCC 4.0.3
 *
 * Date:       2006-12-20
 * Author:     Application@Hitex.de
 *
 * Rights:     Hitex Development Tools GmbH
 *             Greschbachstr. 12
 *             D-76229 Karlsruhe
 ********************************************************************
 * Description:
 *
 * This file is part of the GNU Example chain
 * The code is based on usage of the STmicro library functions
 * This is a small implementation of different features
 * The application runs in ARM mode with high optimization level.
 *
 ********************************************************************
 * History:
 *
 *    Revision 1.1    2007/02/17      Gn
 *    Revision 1.0    2006/12/20      Gn
 *    Initial revision
 ********************************************************************
 * This is a preliminary version.
 *
 * WARRANTY:  HITEX warrants that the media on which the SOFTWARE is
 * furnished is free from defects in materials and workmanship under
 * normal use and service for a period of ninety (90) days. HITEX entire
 * liability and your exclusive remedy shall be the replacement of the
 * SOFTWARE if the media is defective. This Warranty is void if failure
 * of the media resulted from unauthorized modification, accident, abuse,
 * or misapplication.
 *
 * DISCLAIMER:  OTHER THAN THE ABOVE WARRANTY, THE SOFTWARE IS FURNISHED
 * "AS IS" WITHOUT WARRANTY OF ANY KIND. HITEX DISCLAIMS ALL OTHER WARRANTIES,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * NEITHER HITEX NOR ITS AFFILIATES SHALL BE LIABLE FOR ANY DAMAGES ARISING
 * OUT OF THE USE OF OR INABILITY TO USE THE SOFTWARE, INCLUDING DAMAGES FOR
 * LOSS OF PROFITS, BUSINESS INTERRUPTION, OR ANY SPECIAL, INCIDENTAL, INDIRECT
 * OR CONSEQUENTIAL DAMAGES EVEN IF HITEX HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGES.
 ********************************************************************/

#define global extern   /* to declare external variables and functions      */

/* switch to SYS mode and enable interrupts */
#define SWITCH_IRQ_TO_SVC asm(" mrs r2, spsr \n stmfd sp!,{r2,r14} \n msr CPSR_c,#0x13 \n stmfd sp!,{lr}" )

/* switch back to IRQ mode with IRQ disabled */
#define SWITCH_SVC_TO_IRQ asm (" ldmfd sp!,{lr} \n msr CPSR_c,#0x12|0x80 \n ldmfd sp!,{r2,r14} \n msr spsr, r2 ")

#define VICVectAddr (*((volatile unsigned long*)0xFFFFF030))

/*******************************************************************************
* Function Name  : Prefetch_Handler
* Description    : This function handles preftetch abort exception.
*******************************************************************************/
void Prefetch_Handler(void)
{
}
/*******************************************************************************
* Function Name  : WDG_IRQHandler
* Description    : This function handles the WDG interrupt request
*******************************************************************************/
void WDG_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : SW_IRQHandler
* Description    : This function handles the SW interrupt request
*******************************************************************************/
void SW_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : ARMRX_IRQHandler
* Description    : This function handles the ARMRX interrupt request
*******************************************************************************/
void ARMRX_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : ARMTX_IRQHandler
* Description    : This function handles the ARMTX interrupt request
*******************************************************************************/
void ARMTX_IRQHandler(void)
{
}

#if 0
/*******************************************************************************
* Function Name  : TIM0_IRQHandler
* Description    : This function handles the TIM0 interrupt request
*******************************************************************************/
void TIM0_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : TIM1_IRQHandler
* Description    : This function handles the TIM1 interrupt request
*******************************************************************************/
void TIM1_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : TIM2_IRQHandler
* Description    : This function handles the TIM2 interrupt request
*******************************************************************************/
void TIM2_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : TIM3_IRQHandler
* Description    : This function handles the TIM3 interrupt request
*******************************************************************************/
void TIM3_IRQHandler(void)
{
}
#endif

/*******************************************************************************
* Function Name  : USBHP_IRQHandler
* Description    : This function handles the USBHP interrupt request
*******************************************************************************/
void USBHP_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : USBLP_IRQHandler
* Description    : This function handles the USBLP interrupt request
*******************************************************************************/
void USBLP_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : SCU_IRQHandler
* Description    : This function handles the SCU interrupt request
*******************************************************************************/
void SCU_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : ENET_IRQHandler
* Description    : This function handles the DENET interrupt request
*******************************************************************************/
void ENET_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : DMA_IRQHandler
* Description    : This function handles the DMA interrupt request
*******************************************************************************/
void DMA_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : CAN_IRQHandler
* Description    : This function handles the CAN interrupt request
*******************************************************************************/
void CAN_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : MC_IRQHandler
* Description    : This function handles the MC interrupt request
*******************************************************************************/
void MC_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : ADC_IRQHandler
* Description    : This function handles the ADC interrupt request
*******************************************************************************/
extern void ADC_IRQHandler(void);

/*******************************************************************************
* Function Name  : UART0_IRQHandler
* Description    : This function handles the UART0 interrupt request
*******************************************************************************/
void UART0_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : UART1_IRQHandler
* Description    : This function handles the UART1 interrupt request
*******************************************************************************/
void UART1_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : UART2_IRQHandler
* Description    : This function handles the UART2 interrupt request
*******************************************************************************/
void UART2_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : I2C0_IRQHandler
* Description    : This function handles the I2C0 interrupt request
*******************************************************************************/
void I2C0_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : I2C1_IRQHandler
* Description    : This function handles the I2C1 interrupt request
*******************************************************************************/
void I2C1_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : SSP0_IRQHandler
* Description    : This function handles the SSP0 interrupt request
*******************************************************************************/
void SSP0_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : SSP1_IRQHandler
* Description    : This function handles the SSP1 interrupt request
*******************************************************************************/
void SSP1_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : LVD_IRQHandler
* Description    : This function handles the LVD interrupt request
*******************************************************************************/
void LVD_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : RTC_IRQHandler
* Description    : This function handles the RTC interrupt request
*******************************************************************************/
void RTC_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : WIU_IRQHandler
* Description    : This function handles the WIU interrupt request
*******************************************************************************/
/*void WIU_IRQHandler(void)
{
}*/

/*******************************************************************************
* Function Name  : EXTIT0_IRQHandler
* Description    : This function handles the EXTIT0 interrupt request
*******************************************************************************/
void EXTIT0_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : EXTIT1_IRQHandler
* Description    : This function handles the EXTIT1 interrupt request
*******************************************************************************/
void EXTIT1_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : EXTIT2_IRQHandler
* Description    : This function handles the EXTIT2 interrupt request
*******************************************************************************/
void EXTIT2_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : EXTIT3_IRQHandler
* Description    : This function handles the EXTIT3 interrupt request
*******************************************************************************/
void EXTIT3_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : USBWU_IRQHandler
* Description    : This function handles the USBWU interrupt request
*******************************************************************************/
void USBWU_IRQHandler(void)
{
}
/*******************************************************************************
* Function Name  : PFQBC_IRQHandler
* Description    : This function handles the PFQBC interrupt request
*******************************************************************************/
void PFQBC_IRQHandler(void)
{
}

