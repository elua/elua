
/*CODE_BLOCK_BEGIN[CLK001.c]*/

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
** PLATFORM : Infineon XMC4000 Series                                         **
**                                                                            **
** COMPILER : Compiler Independent                                            **
**                                                                            **
** AUTHOR   : App Developer                                                   **
**                                                                            **
** MAY BE CHANGED BY USER [Yes/No]: Yes                                       **
**                                                                            **
** MODIFICATION DATE : Dec 16, 2011                                           **
**                                                                            **
*******************************************************************************/
/**
 * @file   CLK001.c
 *
 * @brief  SCU_Clock_CLK001 App
 *         
 *  CLK001 App is a singleton app which is used by all applications to configure 
 *  the PLL as well as enable the Clock of Peripheral Units.
 *  
 *
 */

/*******************************************************************************
**                      Author(s) Identity                                    **
********************************************************************************
**                                                                            **
** Initials     Name                                                          **
** ---------------------------------------------------------------------------**
** PAE          App Developer                                                 **
*******************************************************************************/

/*******************************************************************************
 ** INCLUDE FILES                                                             **
 ******************************************************************************/
/** Inclusion of header file */
#include <DAVE3.h>

/*******************************************************************************
**                      Private Macro Definitions                             **
*******************************************************************************/
/* MAIN PLL setup parameters */  


#define   CLK001_PLL_K1DIV     0
#define   CLK001_PLL_K2DIV       3      
#define   CLK001_PLL_PDIV         0    
#define   CLK001_PLL_NDIV         39  
#define   CLK001_PLL_K2DIV_STEP_1    19  //PLL output is 24Mhz  
#define   CLK001_PLL_K2DIV_STEP_2    7  //PLL output to 60Mhz  
#define   CLK001_PLL_K2DIV_STEP_3    4  //PLL output to 96Mhz  

/* USB PLL setup parameters */  
#define   CLK001_USBPLL_PDIV      0
#define   CLK001_USBPLL_NDIV      23
#define   CLK001_USBDIV           2
/* OSC_HP setup parameters */
#define  OSC_HP_MODE     0
#define  OSCHPWDGDIV     2

/*******************************************************************************
**                      Private Type Definitions                              **
*******************************************************************************/

/*******************************************************************************
**                 Private Function Declarations:
*******************************************************************************/
static void Delay(void);
static uint32_t (*InitStatusQueryPtr)(void);
static void Delay(void);
static uint32_t Allowed(void);
static uint32_t DisAllowed(void);
/*******************************************************************************
**                      Extern Declarations                                   **
*******************************************************************************/
extern void SystemCoreClockUpdate(void);
/*******************************************************************************
**                      Global Variable Definitions                           **
*******************************************************************************/

/*******************************************************************************
**                      Private Constant Definitions                          **
*******************************************************************************/

/*******************************************************************************
**                 Function like macro definitions                            **
*******************************************************************************/

/*******************************************************************************
**                      Private Function Definitions                          **
*******************************************************************************/ 
static void Delay()
{
	uint32_t i = 0;
	for(i=0; i < 50000;i++)
	{
		__NOP();__NOP();__NOP();__NOP();
	}

}
 

/*******************************************************************************
**                      Public Function Definitions                           **
*******************************************************************************/

/*  Function to initialize the Clock Tree based on UI configuration
 */
void CLK001_Init(void)
{
  /*<<<DD_CLK001_API_1>>>*/

  if((*InitStatusQueryPtr)())
  {        
     
    /* enable PLL first */
    SCU_PLL->PLLCON0 &= ~(SCU_PLL_PLLCON0_VCOPWD_Msk | SCU_PLL_PLLCON0_PLLPWD_Msk);
    /* Enable OSC_HP */
    SCU_OSC->OSCHPCTRL = (OSC_HP_MODE<<4);   /*enable the OSC_HP*/  
    SCU_OSC->OSCHPCTRL |= (OSCHPWDGDIV<<16); /*value for Kit*/
    /* select external OSC as PLL input */
    SCU_PLL->PLLCON2 &= ~SCU_PLL_PLLCON2_PINSEL_Msk;
    /* restart OSC Watchdog */
    SCU_PLL->PLLCON0 &= ~SCU_PLL_PLLCON0_OSCRES_Msk;

    do 
    {
    ;  /* do nothing */
    }while(!((SCU_PLL->PLLSTAT) & (SCU_PLL_PLLSTAT_PLLHV_Msk | SCU_PLL_PLLSTAT_PLLLV_Msk |SCU_PLL_PLLSTAT_PLLSP_Msk)));     
    /* Setup Main PLL */
    /* select FOFI as system clock */
    if(SCU_CLK->SYSCLKCR != 0X000000)SCU_CLK->SYSCLKCR = 0x00000000; /*Select FOFI*/
    /* Go to bypass the Main PLL */
    SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_VCOBYP_Msk;
    /* disconnect OSC_FI to PLL */
    SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_FINDIS_Msk;      
    /* Setup divider settings for main PLL */
    SCU_PLL->PLLCON1 = ((CLK001_PLL_K1DIV) | (CLK001_PLL_NDIV<<8) | (CLK001_PLL_K2DIV_STEP_1<<16) | (CLK001_PLL_PDIV<<24));
    /* we may have to set OSCDISCDIS */
    SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_OSCDISCDIS_Msk;
    /* connect OSC_FI to PLL */
    SCU_PLL->PLLCON0 &= ~SCU_PLL_PLLCON0_FINDIS_Msk;
    /* restart PLL Lock detection */
    SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_RESLD_Msk;
    /* wait for PLL Lock */
    while (!(SCU_PLL->PLLSTAT & SCU_PLL_PLLSTAT_VCOLOCK_Msk));
    /* Go back to the Main PLL */
    SCU_PLL->PLLCON0 &= ~SCU_PLL_PLLCON0_VCOBYP_Msk;

    /*********************************************************
     here we need to setup the system clock divider
    *********************************************************/

    /* Switch system clock to PLL */
    SCU_CLK->SYSCLKCR |=  0x00010000;
    /*********************************************************
     here the ramp up of the system clock starts
    *********************************************************/
    /* Delay for next K2 step ~50?s */
    /********************************/
    Delay();
    /********************************/
    /* Setup divider settings for main PLL */
    SCU_PLL->PLLCON1 = ((CLK001_PLL_K1DIV) | (CLK001_PLL_NDIV<<8) | (CLK001_PLL_K2DIV_STEP_2<<16) | (CLK001_PLL_PDIV<<24));
    /* Delay for next K2 step ~50?s */ 
    /********************************/
    Delay();
    /********************************/

    /* Setup divider settings for main PLL */
    SCU_PLL->PLLCON1 = ((CLK001_PLL_K1DIV) | (CLK001_PLL_NDIV<<8) | (CLK001_PLL_K2DIV_STEP_3<<16) | (CLK001_PLL_PDIV<<24));
    /* Delay for next K2 step ~50?s */
    /********************************/
    Delay();
     /********************************/

    /* Setup divider settings for main PLL */
    SCU_PLL->PLLCON1 = ((CLK001_PLL_K1DIV) | (CLK001_PLL_NDIV<<8) | (CLK001_PLL_K2DIV<<16) | (CLK001_PLL_PDIV<<24));      
   
    /* Disable USB Clock enabled by Startup code */
    SCU_CLK->CLKSET &= ~SCU_CLK_CLKSET_USBCEN_Msk;

  }
  /* Update the clock variable */
  SystemCoreClockUpdate();
}

/*
 * This routine is called by CMSIS startup to find out if clock tree setup should
 * be done by it. This routine is WEAKLY defined in CStart.
 *
 * In the absence of clock app, the weak definition takes precedence which always
 * permits clock tree setup by CStart.
 *
 * When clock app is defined, this function overrides the CStart definition. Clock
 * tree setup is launched and upon completion, control is ceded back to CStart.
 *
 * CStart abstains from setting up clock tree and instead proceeds with program
 * loading.
 *
 * Return 0 to disallow CStart from performing clock tree setup.
 */
uint32_t AllowPLLInitByStartup(void)
{
	/*Early clock initialization*/
	CLK001_Init();
	/*
	 * Let the CStart know that there is no more a need to perform clock tree
	 * initialization.
	 */
	return 0;
}

/*
 *Clock app APIs are invoked before the BSS and DATA sections are initialized.
 *Global/static variables must not be accessed since they are unreliable at this
 *point in time.
 *
 *Instead, stack must be used. CStart calls SetInitStatus first with 0 before
 *invoking CMSIS startup which in turn calls AllowPLLInitByStartup().
 *
 *After the clock tree is setup, Cstart performs program loading and thereafter
 *calls SetInitStatus with 1. This is needed for clock app to prevent itself
 *from initializing all over again when invoked in a different context (DAVE_Init).
 */
void SetInitStatus(uint32_t Status)
{
if(0 == Status)
	InitStatusQueryPtr = Allowed;
else
	InitStatusQueryPtr = DisAllowed;
}

static uint32_t Allowed(void)
{
	return 1;
}

static uint32_t DisAllowed(void)
{
	return 0;
}

/*CODE_BLOCK_END*/
