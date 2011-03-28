/*
 * File:        tower.h
 * Purpose:     Kinetis tower CPU card definitions
 *
 * Notes:
 */

#ifndef __TOWER_H__
#define __TOWER_H__

#include "mcg.h"

/********************************************************************/

/* Global defines to use for all Firebird daughter cards */
#define DEBUG_PRINT

#if (defined(TWR_K40X256))
  #define CPU_MK40X256VMD100

  /*
   * System Bus Clock Info
   */
  #define REF_CLK             XTAL8
  #define CORE_CLK_MHZ        PLL96      /* system/core frequency in MHz */

  /* Serial Port Info */
  #define TERM_PORT           UART3_BASE_PTR
  #define TERMINAL_BAUD       115200
  #undef  HW_FLOW_CONTROL
#elif (defined(TWR_K60N512))
  #define CPU_MK60N512VMD100

  /*
   * System Bus Clock Info
   */
  #define K60_CLK             1
  #define REF_CLK             XTAL8   /* value isn't used, but we still need something defined */
  #define CORE_CLK_MHZ        PLL100  /* 96MHz is only freq tested for a clock input*/

  /* Serial Port Info */
  #define TERM_PORT           UART3_BASE_PTR
  #define TERMINAL_BAUD       115200
  #undef  HW_FLOW_CONTROL
#else
  #error "No valid tower CPU card defined"
#endif


#endif /* __TOWER_H__ */
