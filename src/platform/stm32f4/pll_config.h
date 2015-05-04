// PLL configuration for the STM32F4 platform

#ifndef __PLL_CONFIG_H__
#define __PLL_CONFIG_H__

#include "platform_conf.h"

/******************************************************************************
Assuming HSE (crystal) as the input to PLL
Assumes that a macro named ELUA_BOARD_EXTERNAL_CLOCK_HZ exists and gives the
frequency of the external crystal in Hz (Fin below)
If ELUA_BOARD_CPU_CLOCK_HZ is defined, it will be used a base for
PLL computations. If not, it defaults to the maxium frequency (168MHz)

PLL computations:

Fvco = Fin * ( PLL_N / PLL_M )
Fsysclk = Fvco / PLL_P
Fother = Fcvo / PLL_Q (other: USB OTG FS, SDIO, RNG). Assumed to be 48Mhz.

Restrictions: 
  2 <= PLL_Q <= 15
  PLL_P = 2, 4, 6 or 8
  192 <= PLL_N <= 432
  2 <= PLL_M <= 63

******************************************************************************/

#ifdef ELUA_BOARD_CPU_CLOCK_HZ
#define STM32F4_DESIRED_SYSCLK_FREQ_MHZ           ( ELUA_BOARD_CPU_CLOCK_HZ / 1000000 )
#else
#define STM32F4_DESIRED_SYSCLK_FREQ_MHZ           168
#endif

#define STM32F4_CRYSTAL_FREQ_MHZ                  ( ELUA_BOARD_EXTERNAL_CLOCK_HZ / 1000000 )

#define STM32F4_F_OTHER_FREQ_MHZ                  48

#define PLL_M                                     ( STM32F4_CRYSTAL_FREQ_MHZ )
#if PLL_M < 2 || PLL_M > 63
#error PLL_M out of range, unable to compute PLL parameters
#endif

#if STM32F4_DESIRED_SYSCLK_FREQ_MHZ <= 100
#define PLL_P                                     4
#else
#define PLL_P                                     2
#endif
#define PLL_N                                     ( STM32F4_DESIRED_SYSCLK_FREQ_MHZ * PLL_P )
#if PLL_N < 192 || PLL_N > 432
#error PLL_N out of range, unable to compute PLL parameters
#endif

#define F_VCO                                     ( ( STM32F4_CRYSTAL_FREQ_MHZ * PLL_N ) / PLL_M )
#define STM32F4_ACTUAL_SYSCLK_FREQ_MHZ            ( F_VCO / PLL_P )
#define PLL_Q                                     ( F_VCO / STM32F4_F_OTHER_FREQ_MHZ )
#if ( F_VCO % STM32F4_F_OTHER_FREQ_MHZ ) != 0
#warning Non-integer value detected for PLL_Q, USB might not work properly
#endif
#if PLL_Q < 2 || PLL_Q > 15
#error PLL_Q out of range, unable to compute PLL parameters
#endif

#endif

