/******************************************************************************
 * @file     system_XMC4500.c
 * @brief    Device specific initialization for the XMC4500-Series according 
 * to CMSIS
 * @version  V2.6
 * @date     14 May 2012
 *
 * @note
 * Copyright (C) 2011-2012 Infineon Technologies AG. All rights reserved.
 *
 * @par
 * Infineon Technologies AG (Infineon) is supplying this software for use with 
 * Infineon's microcontrollers.
 *   
 * This file can be freely distributed within development tools that are 
 * supporting such microcontrollers.
 *  
 *
 * @par
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * INFINEON SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ******************************************************************************/

#include "system_XMC4500.h"
#include <XMC4500.h>

/*----------------------------------------------------------------------------
  Clock Variable definitions
 *----------------------------------------------------------------------------*/
/*!< System Clock Frequency (Core Clock)*/
uint32_t SystemCoreClock;

/* Clock definitions, do not modify! */
#define SCU_CLOCK_CRYSTAL_SOURCE  1
#define SCU_CLOCK_BACK_UP_SOURCE  2

#define HIB_CLOCK_FOSI            1
#define HIB_CLOCK_OSCULP          2

/*
//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
*/



/*--------------------- Watchdog Configuration -------------------------------
//
// <e> Watchdog Configuration
//     <o1.0> Disable Watchdog
//
// </e>
*/
#define WDT_SETUP               1
#define WDTENB_nVal             0x00000001

/*--------------------- CLOCK Configuration -------------------------------
//
// <e> Main Clock Configuration
//     <o1.0..1> CPU clock divider
//                     <0=> fCPU = fSYS 
//                     <1=> fCPU = fSYS / 2
//     <o2.0..1>  Peripheral Bus clock divider
//                     <0=> fPB  = fCPU
//                     <1=> fPB  = fCPU / 2
//     <o3.0..1>  CCU Bus clock divider
//                     <0=> fCCU = fCPU
//                     <1=> fCCU = fCPU / 2
//
// </e>
// 
*/

#define  SCU_CLOCK_SETUP     1
#define  SCU_CPUCLKCR_DIV    0x00000000
#define  SCU_PBCLKCR_DIV     0x00000000
#define  SCU_CCUCLKCR_DIV    0x00000000

/*
 * ************************* MANDATORY CLOCK PARAMETERS ***********************
 */
/* Source for clock generation
 * RANGE: SCU_CLOCK_CRYSTAL (crystal or external clock at crystal input)
 *****************************************************************************/

#define  SCU_PLL_INPUT_CLOCK             SCU_CLOCK_CRYSTAL_SOURCE
#define  SCU_CLOCK_CRYSTAL_FREQUENCY     12000000
#define  SCU_CLOCK_BACK_UP               24000000

/* OSC_HP setup parameters */
#define  SCU_OSC_HP_MODE  0
#define  SCU_OSCHPWDGDIV   2

/* MAIN PLL setup parameters */


#define SCU_PLL_K1DIV  1
#define SCU_PLL_K2DIV  3
#define SCU_PLL_PDIV   1
#define SCU_PLL_NDIV   79

#define SCU_PLL_K2DIV_STEP_1  19  //PLL output is 24Mhz
#define SCU_PLL_K2DIV_STEP_2  7  //PLL output to 60Mhz
#define SCU_PLL_K2DIV_STEP_3  4  //PLL output to 96Mhz



/*--------------------- USB CLOCK Configuration ---------------------------
//
// <e> USB Clock Configuration
//
// </e>
// 
*/

#define SCU_USB_CLOCK_SETUP  1

#define SCU_USBPLL_PDIV      0
#define SCU_USBPLL_NDIV      23
#define SCU_USBDIV           2


/*--------------------- CLOCKOUT Configuration -------------------------------
//
// <e> Clock OUT Configuration
//     <o1.0..1>   Clockout Source Selection
//                     <0=> System Clock
//                     <2=> USB Clock
//                     <3=> Divided value of PLL Clock
//     <o2.0..1>   Clockout Pin Selection
//                     <0=> P1.15
//                     <1=> P0.8
//                     
//
// </e>
// 
*/

#define  SCU_CLOCKOUT_SETUP     0  /* Recommended to keep disabled */
#define  SCU_CLOCKOUT_SOURCE    0x00000000
#define  SCU_CLOCKOUT_PIN       0x00000000

/*----------------------------------------------------------------------------
  static functions declarations
 *----------------------------------------------------------------------------*/
#if (SCU_CLOCK_SETUP == 1)
static int SystemClockSetup(void);
#endif

#if (SCU_USB_CLOCK_SETUP == 1)
static int USBClockSetup(void);
#endif

/**
  * @brief  Setup the microcontroller system.
  *         Initialize the PLL and update the 
  *         SystemCoreClock variable.
  * @param  None
  * @retval None
  */
void SystemInit(void)
{
/* Setup the WDT */
#if (WDT_SETUP == 1)
WDT->CTR &= ~WDTENB_nVal; 
#endif

#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
SCB->CPACR |= ((3UL << 10*2) |                 /* set CP10 Full Access */
               (3UL << 11*2)  );               /* set CP11 Full Access */
#endif

/* Disable branch prediction - PCON.PBS = 1 */
PREF->PCON |= (PREF_PCON_PBS_Msk);

/* Enable unaligned memory access - SCB_CCR.UNALIGN_TRP = 0 */
SCB->CCR &= ~(SCB_CCR_UNALIGN_TRP_Msk);

/* Setup the clockout */
/* README README README README README README README README README README */
/*
 * Please use the CLOCKOUT feature with diligence. Use this only if you know
 * what you are doing.
 *
 * You must be aware that the settings below can potentially be in conflict
 * with DAVE code generation engine preferences.
 *
 * Even worse, the setting below configures the ports as output ports while in
 * reality, the board on which this chip is mounted may have a source driving
 * the ports.
 *
 * So use this feature only when you are absolutely sure that the port must 
 * indeed be configured as an output AND you are NOT linking this startup code
 * with code that was generated by DAVE code engine.
 */
#if (SCU_CLOCKOUT_SETUP == 1)
SCU_CLK->EXTCLKCR  |= SCU_CLOCKOUT_SOURCE;

if (SCU_CLOCKOUT_PIN) 
 {
  PORT0->IOCR8 = 0x00000088;  /*P0.8 --> ALT1 select +  HWSEL */
  PORT0->HWSEL &= (~PORT0_HWSEL_HW8_Msk);
  PORT0->PDR1 &= (~PORT0_PDR1_PD8_Msk);  /*set to strong driver */
 }
 else 
 {
  PORT1->IOCR12 = 0x88000000; /*P1.15--> ALT1 select */
  PORT1->PDR1 &= (~PORT1_PDR1_PD15_Msk);  /*set to strong driver */
 }
#endif

/* Setup the System clock */ 
#if (SCU_CLOCK_SETUP == 1)
SystemClockSetup();
#endif

/* Setup the USB PL */ 
#if (SCU_USB_CLOCK_SETUP == 1)
USBClockSetup();
#endif

}


/**
  * @brief  Update SystemCoreClock according to Clock Register Values
  * @note   -  
  * @param  None
  * @retval None
  */
void SystemCoreClockUpdate(void)
{
 uint32_t NDIV,PDIV,K2DIV, VCO;

 if (SCU_CLK->SYSCLKCR ==  0x00010000)
 {
  if (SCU_PLL->PLLSTAT & SCU_PLL_PLLSTAT_VCOLOCK_Msk){
    /* check if PLL is locked */
    /* read back divider settings */
    PDIV = ((SCU_PLL->PLLCON1 & SCU_PLL_PLLCON1_PDIV_Msk)>>24)+1;
    NDIV = ((SCU_PLL->PLLCON1 & SCU_PLL_PLLCON1_NDIV_Msk)>>8)+1;
    K2DIV  = ((SCU_PLL->PLLCON1 & SCU_PLL_PLLCON1_K2DIV_Msk)>>16)+1;

    if(SCU_PLL->PLLCON2 & SCU_PLL_PLLCON2_PINSEL_Msk){
     /* the selected clock is the Backup clock fofi */
     VCO = (SCU_CLOCK_BACK_UP/PDIV)*NDIV;
     SystemCoreClock = VCO/K2DIV;
         
     /* In case the sysclock div is used */
     SystemCoreClock = SystemCoreClock/(
                          (SCU_CLK->SYSCLKCR & SCU_CLK_SYSCLKCR_SYSDIV_Msk)+1);
    }
    else
    {
     /* the selected clock is the PLL external oscillator */
     VCO = (SCU_CLOCK_CRYSTAL_FREQUENCY/PDIV)*NDIV;
     SystemCoreClock = VCO/K2DIV;

     /* In case the sysclock div is used */
     SystemCoreClock = SystemCoreClock/(
                          (SCU_CLK->SYSCLKCR & SCU_CLK_SYSCLKCR_SYSDIV_Msk)+1);
    }

   }
 }
 else
 {
 SystemCoreClock = SCU_CLOCK_BACK_UP;
 }
}

/**
  * @brief  -
  * @note   -  
  * @param  None
  * @retval None
  */
#if (SCU_CLOCK_SETUP == 1)
static int SystemClockSetup(void)
{

 if(AllowPLLInitByStartup()){
  /* check if PLL is switched on */
  if ((SCU_PLL->PLLCON0 &(SCU_PLL_PLLCON0_VCOPWD_Msk | 
                                             SCU_PLL_PLLCON0_PLLPWD_Msk)) != 0)
  {
   /* Enable PLL first */
   SCU_PLL->PLLCON0 &= ~(SCU_PLL_PLLCON0_VCOPWD_Msk | 
                                                   SCU_PLL_PLLCON0_PLLPWD_Msk);
  }

  /* Enable OSC_HP if not already on*/
  if (SCU_PLL_INPUT_CLOCK == SCU_CLOCK_CRYSTAL_SOURCE)
   {
    if (SCU_OSC->OSCHPCTRL & SCU_OSC_OSCHPCTRL_MODE_Msk)
    {
     
     /* Enable the OSC_HP*/     
     SCU_OSC->OSCHPCTRL = (SCU_OSC_HP_MODE<<4);   
  
     /* setup OSC WDG devider */
     SCU_OSC->OSCHPCTRL |= (SCU_OSCHPWDGDIV<<16);
  
     /* select external OSC as PLL input */
     SCU_PLL->PLLCON2 &= ~SCU_PLL_PLLCON2_PINSEL_Msk;

     /* restart OSC Watchdog */
     SCU_PLL->PLLCON0 &= ~SCU_PLL_PLLCON0_OSCRES_Msk;  
  
     /* Timeout for wait loo ~150ms */
     /********************************/

     /* Set reload register */
     SysTick->LOAD  = ((5000000+100) & SysTick_LOAD_RELOAD_Msk) - 1;

     /* Load the SysTick Counter Value */
     SysTick->VAL   = 0;

     /* Enable SysTick IRQ and SysTick Timer */
     SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
                     
     do 
     {
       ;/* wait for ~150ms  */
     }while((((SCU_PLL->PLLSTAT) & (SCU_PLL_PLLSTAT_PLLHV_Msk | 
               SCU_PLL_PLLSTAT_PLLLV_Msk |SCU_PLL_PLLSTAT_PLLSP_Msk)) 
                                             != 0x380)&&(SysTick->VAL >= 500));

     /* Stop SysTick Timer */
     SysTick->CTRL  &= ~SysTick_CTRL_ENABLE_Msk;

     if (((SCU_PLL->PLLSTAT) & (SCU_PLL_PLLSTAT_PLLHV_Msk | 
           SCU_PLL_PLLSTAT_PLLLV_Msk |SCU_PLL_PLLSTAT_PLLSP_Msk)) != 0x380)
      return(0);/* Return Error */
     }
    }


   if (!(SCU_PLL->PLLSTAT & SCU_PLL_PLLSTAT_VCOLOCK_Msk))
   {
    /* Systen is still running from internal clock */
    /* Setup Main PLL */
    /* select FOFI as system clock */
    if((SCU_CLK->SYSCLKCR & SCU_CLK_SYSCLKCR_SYSSEL_Msk) != 0x0)
     SCU_CLK->SYSCLKCR &= ~SCU_CLK_SYSCLKCR_SYSSEL_Msk; /*Select FOFI*/

     /* Go to bypass the Main PLL */
    SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_VCOBYP_Msk;

    /* Disconnect OSC_HP to PLL */
    SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_FINDIS_Msk;

    /* Setup devider settings for main PLL */
    SCU_PLL->PLLCON1 = ((SCU_PLL_K1DIV) | (SCU_PLL_NDIV<<8) | 
                               (SCU_PLL_K2DIV_STEP_1<<16) |(SCU_PLL_PDIV<<24));

    /* We may have to set OSCDISCDIS */
    SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_OSCDISCDIS_Msk;

    /* Connect OSC_HP to PLL */
    SCU_PLL->PLLCON0 &= ~SCU_PLL_PLLCON0_FINDIS_Msk;

    /* Restart PLL Lock detection */
    SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_RESLD_Msk;

     /* Wait for PLL Lock */
     /* setup time out loop */
     /* Timeout for wait loo ~150ms */
     /********************************/
    
    /* set reload register */
    SysTick->LOAD  = ((5000000+100) & SysTick_LOAD_RELOAD_Msk) - 1;

    /* Load the SysTick Counter Value */
    SysTick->VAL   = 0;

    /* Enable SysTick IRQ and SysTick Timer */
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
                     

    while ((!(SCU_PLL->PLLSTAT & SCU_PLL_PLLSTAT_VCOLOCK_Msk))&&
                                                        (SysTick->VAL >= 500));
    /* Stop SysTick Timer */
    SysTick->CTRL  &= ~SysTick_CTRL_ENABLE_Msk;

     if ((SCU_PLL->PLLSTAT & SCU_PLL_PLLSTAT_VCOLOCK_Msk)==
                                                   SCU_PLL_PLLSTAT_VCOLOCK_Msk)
      {
       /* Go back to the Main PLL */
       SCU_PLL->PLLCON0 &= ~SCU_PLL_PLLCON0_VCOBYP_Msk;
      }
      else
      { 
       return(0);
      }
  
     /*********************************************************
     Here we need to setup the system clock divider
     *********************************************************/
  
    SCU_CLK->CPUCLKCR = SCU_CPUCLKCR_DIV;
    SCU_CLK->PBCLKCR = SCU_PBCLKCR_DIV;  
    SCU_CLK->CCUCLKCR = SCU_CCUCLKCR_DIV;
  
    /* Switch system clock to PLL */
    SCU_CLK->SYSCLKCR |=  0x00010000; 
                                  
    /*********************************************************
    Here the ramp up of the system clock starts
    *********************************************************/
    /* Delay for next K2 step ~50us */
    /********************************/

    /* Set reload register */
    SysTick->LOAD  = ((1250+100) & SysTick_LOAD_RELOAD_Msk) - 1;

    /* Load the SysTick Counter Value */
    SysTick->VAL   = 0; 

    /* Enable SysTick IRQ and SysTick Timer */
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    /* Wait for ~50us  */
    while (SysTick->VAL >= 100);                   
    /* Stop SysTick Timer */
    SysTick->CTRL  &= ~SysTick_CTRL_ENABLE_Msk;                 

    /********************************/
  
    /* Setup devider settings for main PLL */
    SCU_PLL->PLLCON1 = ((SCU_PLL_K1DIV) | (SCU_PLL_NDIV<<8) | 
                              (SCU_PLL_K2DIV_STEP_2<<16) | (SCU_PLL_PDIV<<24));

    /* Delay for next K2 step ~50us */
    /********************************/
    SysTick->LOAD  = ((3000+100) & SysTick_LOAD_RELOAD_Msk) - 1;

    /* Load the SysTick Counter Value */
    SysTick->VAL   = 0; 

    /* Enable SysTick IRQ and SysTick Timer */
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
                                         
    /* Wait for ~50us  */  
    while (SysTick->VAL >= 100);  

    /* Stop SysTick Timer */
    SysTick->CTRL  &= ~SysTick_CTRL_ENABLE_Msk;
    /********************************/
  
    /* Setup devider settings for main PLL */
    SCU_PLL->PLLCON1 = ((SCU_PLL_K1DIV) | (SCU_PLL_NDIV<<8) | 
                              (SCU_PLL_K2DIV_STEP_3<<16) | (SCU_PLL_PDIV<<24));
  
     /* Delay for next K2 step ~50us */
     /********************************/
     SysTick->LOAD  = ((4800+100) & SysTick_LOAD_RELOAD_Msk) - 1;

     /* Load the SysTick Counter Value */
     SysTick->VAL   = 0;

     /* Enable SysTick IRQ and SysTick Timer */
     SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
                                         
     /* Wait for ~50us  */  
     while (SysTick->VAL >= 100);                   

     /* Stop SysTick Timer */
     SysTick->CTRL  &= ~SysTick_CTRL_ENABLE_Msk;                 
     /********************************/
  
     /* Setup devider settings for main PLL */
     SCU_PLL->PLLCON1 = ((SCU_PLL_K1DIV) | (SCU_PLL_NDIV<<8) | 
                                     (SCU_PLL_K2DIV<<16) | (SCU_PLL_PDIV<<24));

     /* Clear request for System OCS Watchdog Trap and System VCO Lock Trap  */  
     SCU_TRAP->TRAPCLR = SCU_TRAP_TRAPCLR_SOSCWDGT_Msk | 
                                                 SCU_TRAP_TRAPCLR_SVCOLCKT_Msk;
  }
 }
   return(1);
}
#endif

/**
  * @brief  -
  * @note   -  
  * @param  None
  * @retval None
  */
#if(SCU_USB_CLOCK_SETUP == 1)
static int USBClockSetup(void)
{
 /* Enable PLL first */
 SCU_PLL->USBPLLCON &= ~(SCU_PLL_USBPLLCON_VCOPWD_Msk | 
                                                 SCU_PLL_USBPLLCON_PLLPWD_Msk);

  /* Enable OSC_HP if not already on*/
  if (SCU_OSC->OSCHPCTRL & SCU_OSC_OSCHPCTRL_MODE_Msk)
   {
    SCU_OSC->OSCHPCTRL = (SCU_OSC_HP_MODE<<4);   /*enable the OSC_HP*/

    /* setup OSC WDG devider */
    SCU_OSC->OSCHPCTRL |= (SCU_OSCHPWDGDIV<<16);

    /* select external OSC as PLL input */
    SCU_PLL->PLLCON2 &= ~SCU_PLL_PLLCON2_PINSEL_Msk;

    /* restart OSC Watchdog */
    SCU_PLL->PLLCON0 &= ~SCU_PLL_PLLCON0_OSCRES_Msk;

    /* Timeout for wait loo ~150ms */
    /********************************/

    /* Set reload register */
    SysTick->LOAD  = ((5000000+100) & SysTick_LOAD_RELOAD_Msk) - 1;

    /* Load the SysTick Counter Value */
    SysTick->VAL   = 0;

    /* Enable SysTick IRQ and SysTick Timer */
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
                                           
    do
    {
         ;/* wait for ~150ms  */
    }while((((SCU_PLL->PLLSTAT) & (SCU_PLL_PLLSTAT_PLLHV_Msk | 
              SCU_PLL_PLLSTAT_PLLLV_Msk |SCU_PLL_PLLSTAT_PLLSP_Msk)) != 0x380)
              &&(SysTick->VAL >= 500));

    /* Stop SysTick Timer */
    SysTick->CTRL  &= ~SysTick_CTRL_ENABLE_Msk;                 

    if (((SCU_PLL->PLLSTAT) & (SCU_PLL_PLLSTAT_PLLHV_Msk | 
          SCU_PLL_PLLSTAT_PLLLV_Msk |SCU_PLL_PLLSTAT_PLLSP_Msk)) != 0x380)
      return(0);/* Return Error */

   }

/* Setup USB PLL */
   /* Go to bypass the Main PLL */
   SCU_PLL->USBPLLCON |= SCU_PLL_USBPLLCON_VCOBYP_Msk;
   /* disconnect OSC_FI to PLL */
   SCU_PLL->USBPLLCON |= SCU_PLL_USBPLLCON_FINDIS_Msk;
   /* Setup devider settings for main PLL */
   SCU_PLL->USBPLLCON = ((SCU_USBPLL_NDIV<<8) | (SCU_USBPLL_PDIV<<24));
   /* Setup USBDIV settings USB clock */
   SCU_CLK->USBCLKCR = SCU_USBDIV;
   /* we may have to set OSCDISCDIS */
   SCU_PLL->USBPLLCON |= SCU_PLL_USBPLLCON_OSCDISCDIS_Msk;
   /* connect OSC_FI to PLL */
   SCU_PLL->USBPLLCON &= ~SCU_PLL_USBPLLCON_FINDIS_Msk;
   /* restart PLL Lock detection */
   SCU_PLL->USBPLLCON |= SCU_PLL_USBPLLCON_RESLD_Msk;
   /* wait for PLL Lock */
   while (!(SCU_PLL->USBPLLSTAT & SCU_PLL_USBPLLSTAT_VCOLOCK_Msk));

   return(1);
 
 }
#endif
