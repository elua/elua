#include "lpc288x.h"
#include "sdram_lpc.h"
#include <stdio.h>
#include "type.h"
#include "target.h"

#define SDRAM_PERIOD          16.6  // 60MHz
#define P2C(Period)           (((Period<SDRAM_PERIOD)?0:(u32)((float)Period/SDRAM_PERIOD))+1)
#define PLL_MUL               5
#define PLL_DIV               1

typedef enum _SectionInClockSelect_t
{
  LF_Osc = 0, HF_Osc, MCI_ClkPin, DAI_BCLK_ClkPin, DAI_WS_ClkPin,
  HighPLL = 7, MainPLL,
} SectionInClockSelect_t;

static void LPC288x_SectionClockSelect(volatile unsigned int * pReg, u32 Mode)
{
#define ST_OFFSET   ((unsigned long *)&SYSSSR - (unsigned long *)&SYSSCR)
#define CH1_OFFSET  ((unsigned long *)&SYSFSR1 - (unsigned long *)&SYSSCR)
#define CH2_OFFSET  ((unsigned long *)&SYSFSR2 - (unsigned long *)&SYSSCR)
unsigned long Channel = (*(pReg+ST_OFFSET) & 0x3) ^ 0x3;  // get unused channel
  // Apply new clock source setting
  if(Channel & 0x1)
  {
    *(pReg+CH1_OFFSET) = Mode;
  }
  else
  {
    *(pReg+CH2_OFFSET) = Mode;
  }
  // change the channel
  *pReg = Channel;
}

static void InitSDRAMCtrl(void) 
{
  volatile u32 i;
    
  // assigne GPIO to EMC
  MODE0_0 = 0xFFFFFFFF;
  MODE0_1 = 0x000FFFFF;
  MODE1_0 = 0x00000000;
  MODE1_1 = 0x00000000;   
  
  // enable EMC clk
  // disable fractional divider
  EMCESR0 &= ~0x01;
  EMCESR1 &= ~0x01;
  // release resert of the EMC
  EMCRES = 1;
     
  // Init SDRAM controller and memory
  EMC_CTRL = 1; // Enable EMC
  EMC_DYN_RD_CFG = 1; // Command delayed strategy, using AHBHCLKDELAY     
  EMC_MISC = 0;      
  EMC_DYN_RASCAS0 = ( 2 << 8 ) | 2;   // RAS and CAS
  EMC_DYN_RP = P2C(SDRAM_TRP);
  EMC_DYN_RAS = P2C(SDRAM_TRAS);
  EMC_DYN_SREX = P2C(SDRAM_TXSR);
  EMC_DYN_APR = SDRAM_TAPR;
  EMC_DYN_DAL = SDRAM_TDAL+P2C(SDRAM_TRP);
  EMC_DYN_WR = SDRAM_TWR;
  EMC_DYN_RC = P2C(SDRAM_TRC);
  EMC_DYN_RFC = P2C(SDRAM_TRFC);
  EMC_DYN_XSR = P2C(SDRAM_TXSR);
  EMC_DYN_RRD = P2C(SDRAM_TRRD);
  EMC_DYN_MRD = SDRAM_TMRD;     
  EMC_DYN_CFG0 = 0x0000680;        // 13 row, 9 - col, SDRAM
    
  // JEDEC General SDRAM Initialization Sequence
  // DELAY to allow power and clocks to stabilize ~100 us
  // NOP
  EMC_DYN_CTRL = 0x4183;         
  for( i = 0; i < 1000; i ++ );
  
  // PALL
  EMC_DYN_CTRL = ( EMC_DYN_CTRL & ~0x180 ) | ( 2 << 7 );
  EMC_DYN_RFSH = 1;
  for(i= 128; i; --i); // > 128 clk
  EMC_DYN_RFSH = P2C(SDRAM_REFRESH) >> 4;
  // COMM
  EMC_DYN_CTRL = ( EMC_DYN_CTRL & ~0x180 ) | ( 1 << 7 );
  // Busrt 8, Sequental, CAS-2
  volatile unsigned long Dummy = *(volatile unsigned short *)(SDRAM_BASE_ADDR | (0x23UL << 12));
  // NORM
  EMC_DYN_CTRL = 0x4000;
  EMC_DYN_CFG0 |= ( 1 << 19 );  
}

// Platform initialization function
void lpc288x_init()
{
  
  // Disable WDT
  WDT_TCR &= ~0x01;

  // Disable and reset cache
  CACHE_PAGE_CTRL = 0;
  CACHE_SETTINGS = 1;
  CACHE_SETTINGS &= ~0x01;
  while((CACHE_RST_STAT) & 0x1);	/* Wait for reset to complete */
  
  // Set flash read wait states
  F_WAIT |= 4;
  asm( "nop" ); 
  asm( "nop" );
  asm( "nop" );
  asm( "nop" );      
  
  // map iflash memory on address 0x00000000
  ADDRESS_PAGE_0 = (FLASH_ADDR >>21);
  CACHE_PAGE_CTRL = 0x01;
  // Enable cache
  CACHE_SETTINGS = 0x06;
  
  // Set PLL and clocks  
  OSCEN |= 1;
  
  // Init all section to fast clock
  LPC288x_SectionClockSelect(&SYSSCR ,HF_Osc);
  LPC288x_SectionClockSelect(&APB0SCR,HF_Osc);
  LPC288x_SectionClockSelect(&APB1SCR,HF_Osc);
  LPC288x_SectionClockSelect(&APB3SCR,HF_Osc);
  LPC288x_SectionClockSelect(&DCDCSCR,HF_Osc);
  LPC288x_SectionClockSelect(&RTCSCR ,HF_Osc);
  LPC288x_SectionClockSelect(&MCISCR ,HF_Osc);
  LPC288x_SectionClockSelect(&UARTSCR,HF_Osc);
  LPC288x_SectionClockSelect(&DAIOSCR,HF_Osc);
  LPC288x_SectionClockSelect(&DAISCR ,HF_Osc);
  
  // Init PLL
  LPPDN  = 1;           // stop the main PLL
  LPFIN  = HF_Osc;      // fast osc input clock
  LPMSEL = PLL_MUL-1;   // set PLL multiplier
  LPPSEL = PLL_DIV;     // set PLL divider
  LPPDN  = 0;           // start the main PLL
  // wait until PLL lock frequency
  while(!LPLOCK);  
  
  // Select Main PLL clock of system state
  LPC288x_SectionClockSelect(&SYSSCR,MainPLL);  
  LPC288x_SectionClockSelect(&UARTSCR,MainPLL);
  
  // Initialize interrupt controller
  INT_PRIOMASK0 = INT_PRIOMASK1 = 0;  
  
  // Init SDRAM controller
  InitSDRAMCtrl();  
}
