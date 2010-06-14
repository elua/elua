/*****************************************************************************
 *   target.h:  Header file for NXP LPC23xx/24xx Family Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.09.20  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#ifndef __TARGET_H 
#define __TARGET_H

#ifdef __cplusplus
   extern "C" {
#endif

/* Only choose one of them below, by default, it's Keil MCB2300 */
/*#define ENG_BOARD_LPC24XX				0
#define KEIL_BOARD_LPC23XX				1
#define EA_BOARD_LPC24XX				0
#define IAR_BOARD_LPC23XX				0*/

/* On EA and IAR boards, they use Micrel PHY.
   on ENG and KEIL boards, they use National PHY */
/*#define NATIONAL_PHY			1
#define MICREL_PHY				2*/

/* If USB device is used, CCO will be 288Mhz( divided by 6) or 384Mhz( divided by 8)
to get precise USB clock 48Mhz. If USB is not used, you set any clock you want
but make sure the divider of the CCO should be an even number. If you want to 
use USB, change "define USE_USB" from 0 to 1 */
 
#define	USE_USB					0

/* PLL Setting Table Matrix */
/* 	
	Main Osc.	CCLKCFG		Fcco		Fcclk 		M 	N
	12Mhz		29		300Mhz		10Mhz			24	1
	12Mhz		35		360Mhz		10Mhz			14	0					
	12Mhz		27		336Mhz		12Mhz			13	0		
	12Mhz		17		360Mhz		20Mhz			14	0
	12Mhz		13		336Mhz		24Mhz			13	0
	12Mhz		11		300Mhz		25Mhz			24	1   
	12Mhz		9		  300Mhz		30Mhz			24	1
	12Mhz		11		360Mhz		30Mhz			14	0
	12Mhz		9		  320Mhz		32Mhz			39	2
	12Mhz		9		  350Mhz		35Mhz			174	11
	12Mhz		7		  312Mhz		39Mhz			12	0 
	12Mhz		7		  360Mhz		45Mhz			14	0  
	12Mhz		5		  300Mhz		50Mhz			24	1
	12Mhz		5		  312Mhz		52Mhz			12	0
	12Mhz		5		  336Mhz		56Mhz			13	0				
	12Mhz		3		  300Mhz		75Mhz			24	1
	12Mhz		3		  312Mhz		78Mhz			12	0  
	12Mhz		3		  320Mhz		80Mhz			39	2
	12Mhz		3		  336Mhz		84Mhz			13	0 
*/

/* These are limited number of Fcco configuration for
USB communication as the CPU clock and USB clock shares
the same PLL. The USB clock needs to be multiple of
48Mhz. */
#if USE_USB		/* 1 is USB, 0 is non-USB related */  
/* Fcck = 48Mhz, Fosc = 288Mhz, and USB 48Mhz */
#define PLL_MValue			11
#define PLL_NValue			0
#define CCLKDivValue		5
#define USBCLKDivValue		5

/* System configuration: Fosc, Fcclk, Fcco, Fpclk must be defined */
/* PLL input Crystal frequence range 4KHz~20MHz. */
#define Fosc	12000000
/* System frequence,should be less than 80MHz. */
#define Fcclk	48000000
#define Fcco	288000000

#else // #if USE_USB


// [TODO]: use the PLL calculator XLS to increase frequency a bit
/* Fcck = 60Mhz, Fosc = 360Mhz, USB can't be divided into 48Mhz
in this case, so USBCLKDivValue is not needed. */
#if 0 // 60MHz
#define PLL_MValue			14
#define PLL_NValue			0
#define CCLKDivValue		5
#define Fcclk           60000000ULL
#else // 72MHz
#define PLL_MValue      14
#define PLL_NValue      0
#define CCLKDivValue    4
#define Fcclk           72000000ULL
#endif

/* System configuration: Fosc, Fcclk, Fcco, Fpclk must be defined */
/* PLL input Crystal frequence range 4KHz~20MHz. */
#define Fosc	12000000
/* System frequence,should be less than 72MHz. */
#define Fcco	360000000

#endif

/* APB clock frequence , must be 1/2/4 multiples of ( Fcclk/4 ). */
/* If USB is enabled, the minimum APB must be greater than 16Mhz */ 
#if USE_USB
#define Fpclk	(Fcclk / 2)
#else
#define Fpclk	(Fcclk / 4)
#endif

#define Fpclk_MHz       (Fpclk / 1000000)
#define Fpclk_UART      (Fcclk)
#define SYS_FREQ        (Fcclk / 1000000)

#ifdef ELUA_BOARD_ELUAPUC

#if   SYS_FREQ == (72)
#define EMC_PERIOD          13.88888888  // 72MHz
#elif   SYS_FREQ == (60)
#define EMC_PERIOD          16.6  // 60MHz
#elif   SYS_FREQ == (57)
#define EMC_PERIOD          17.4  // 57.6MHz
#elif SYS_FREQ == (48)
#define EMC_PERIOD          20.8  // 48MHz
#elif SYS_FREQ == (36)
#define EMC_PERIOD          27.8  // 36MHz
#elif SYS_FREQ == (24)
#define EMC_PERIOD          41.7  // 24MHz
#else
#error Frequency not defined
#endif

#define SDRAM_REFRESH         15625	//max 64ms
#define SDRAM_TRP             18	  //
#define SDRAM_TRAS            42	
#define SDRAM_TAPR            2
#define SDRAM_TWR             2
#define SDRAM_TDAL  			    (SDRAM_TWR+P2C(SDRAM_TRP))
#define SDRAM_TRC             60 //samsung 65 micron 66
#define SDRAM_TRFC            60	
#define SDRAM_TXSR            70		
#define SDRAM_TRRD            12     
#define SDRAM_TMRD            2	
	
#endif

/******************************************************************************
** Function name:		TargetInit
**
** Descriptions:		Initialize the target board; it is called in a 
**				necessary place, change it as needed
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
extern void TargetInit(void);
extern void ConfigurePLL( void );
extern void TargetResetInit(void);

#ifdef __cplusplus
   }
#endif
 
#endif /* end __TARGET_H */
/******************************************************************************
**                            End Of File
******************************************************************************/
