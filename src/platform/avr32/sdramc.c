/* This source file is part of the ATMEL AVR-UC3-SoftwareFramework-1.6.1 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief SDRAMC on EBI driver for AVR32 UC3.
 *
 * - Compiler:           IAR EWAVR32 and GNU GCC for AVR32
 * - Supported devices:  All AVR32 devices with an SDRAMC module can be used.
 * - AppNote:
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support and FAQ: http://support.atmel.no/
 *
 ******************************************************************************/

/* Copyright (c) 2009 Atmel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an Atmel
 * AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 *
 */

#include "compiler.h"
#include "preprocessor.h"
#include "gpio.h"
#include "sdramc.h"


/*! \brief Waits during at least the specified delay before returning.
 *
 * \param ck Number of HSB clock cycles to wait.
 */
static void sdramc_ck_delay(unsigned long ck)
{
  // Use the CPU cycle counter (CPU and HSB clocks are the same).
  unsigned long delay_start_cycle = Get_system_register(AVR32_COUNT);

  // at 60MHz the count register wraps every 71.68 secs, at 66MHz every 65s.
  // The following unsigned arithmetic handles the wraparound condition.
  while ((unsigned long)Get_system_register(AVR32_COUNT) - delay_start_cycle < ck)
    /* wait */;
}


/*! \brief Waits during at least the specified delay before returning.
 *
 * \param ns Number of nanoseconds to wait.
 * \param hsb_mhz_up Rounded-up HSB frequency in MHz.
 */
#define sdramc_ns_delay(ns, hsb_mhz_up)   sdramc_ck_delay(((ns) * (hsb_mhz_up) + 999) / 1000)


/*! \brief Waits during at least the specified delay before returning.
 *
 * \param us Number of microseconds to wait.
 * \param hsb_mhz_up Rounded-up HSB frequency in MHz.
 */
#define sdramc_us_delay(us, hsb_mhz_up)   sdramc_ck_delay((us) * (hsb_mhz_up))


/*! \brief Puts the multiplexed MCU pins used for the SDRAM under control of the
 *         SDRAMC.
 */
#if BOARD == EVK1100 || BOARD == EVK1104 || BOARD == EVK1105 \
 || BOARD == MIZAR32
static void sdramc_enable_muxed_pins(void)
{
  static const gpio_map_t SDRAMC_EBI_GPIO_MAP =
  {
    // Enable data pins.
#define SDRAMC_ENABLE_DATA_PIN(DATA_BIT, unused) \
    {AVR32_EBI_DATA_##DATA_BIT##_PIN, AVR32_EBI_DATA_##DATA_BIT##_FUNCTION},
    MREPEAT(SDRAM_DBW, SDRAMC_ENABLE_DATA_PIN, ~)
#undef SDRAMC_ENABLE_DATA_PIN

    // Enable row/column address pins.
    {AVR32_EBI_ADDR_2_PIN,            AVR32_EBI_ADDR_2_FUNCTION           },
    {AVR32_EBI_ADDR_3_PIN,            AVR32_EBI_ADDR_3_FUNCTION           },
    {AVR32_EBI_ADDR_4_PIN,            AVR32_EBI_ADDR_4_FUNCTION           },
    {AVR32_EBI_ADDR_5_PIN,            AVR32_EBI_ADDR_5_FUNCTION           },
    {AVR32_EBI_ADDR_6_PIN,            AVR32_EBI_ADDR_6_FUNCTION           },
    {AVR32_EBI_ADDR_7_PIN,            AVR32_EBI_ADDR_7_FUNCTION           },
    {AVR32_EBI_ADDR_8_PIN,            AVR32_EBI_ADDR_8_FUNCTION           },
    {AVR32_EBI_ADDR_9_PIN,            AVR32_EBI_ADDR_9_FUNCTION           },
    {AVR32_EBI_ADDR_10_PIN,           AVR32_EBI_ADDR_10_FUNCTION          },
    {AVR32_EBI_ADDR_11_PIN,           AVR32_EBI_ADDR_11_FUNCTION          },
    {AVR32_EBI_SDA10_0_PIN,           AVR32_EBI_SDA10_0_FUNCTION          },
#if SDRAM_ROW_BITS >= 12
    {AVR32_EBI_ADDR_13_PIN,           AVR32_EBI_ADDR_13_FUNCTION          },
  #if SDRAM_ROW_BITS >= 13
    {AVR32_EBI_ADDR_14_PIN,           AVR32_EBI_ADDR_14_FUNCTION          },
  #endif
#endif

    // Enable bank address pins.
    {AVR32_EBI_ADDR_16_PIN,           AVR32_EBI_ADDR_16_FUNCTION          },
#if SDRAM_BANK_BITS >= 2
    {AVR32_EBI_ADDR_17_PIN,           AVR32_EBI_ADDR_17_FUNCTION          },
#endif

    // Enable data mask pins.
    {AVR32_EBI_ADDR_0_PIN,            AVR32_EBI_ADDR_0_FUNCTION           },
    {AVR32_EBI_NWE1_0_PIN,            AVR32_EBI_NWE1_0_FUNCTION           },
#if SDRAM_DBW >= 32
    {AVR32_EBI_ADDR_1_PIN,            AVR32_EBI_ADDR_1_FUNCTION           },
    {AVR32_EBI_NWE3_0_PIN,            AVR32_EBI_NWE3_0_FUNCTION           },
#endif

    // Enable control pins.
    {AVR32_EBI_SDWE_0_PIN,            AVR32_EBI_SDWE_0_FUNCTION           },
    {AVR32_EBI_CAS_0_PIN,             AVR32_EBI_CAS_0_FUNCTION            },
    {AVR32_EBI_RAS_0_PIN,             AVR32_EBI_RAS_0_FUNCTION            },
    {AVR32_EBI_NCS_1_PIN,             AVR32_EBI_NCS_1_FUNCTION            },

    // Enable clock-related pins.
    {AVR32_EBI_SDCK_0_PIN,            AVR32_EBI_SDCK_0_FUNCTION           },
    {AVR32_EBI_SDCKE_0_PIN,           AVR32_EBI_SDCKE_0_FUNCTION          }
  };

  gpio_enable_module(SDRAMC_EBI_GPIO_MAP, sizeof(SDRAMC_EBI_GPIO_MAP) / sizeof(SDRAMC_EBI_GPIO_MAP[0]));
}
#elif BOARD == UC3C_EK 
static void sdramc_enable_muxed_pins(void)
{
  static const gpio_map_t SDRAMC_EBI_GPIO_MAP =
  {
    // Enable data pins.
#define SDRAMC_ENABLE_DATA_PIN(DATA_BIT, unused) \
    {AVR32_EBI_DATA_##DATA_BIT##_PIN, AVR32_EBI_DATA_##DATA_BIT##_FUNCTION},
    MREPEAT(SDRAM_DBW, SDRAMC_ENABLE_DATA_PIN, ~)
#undef SDRAMC_ENABLE_DATA_PIN

    // Enable row/column address pins.
    {AVR32_EBI_ADDR_2_PIN,            AVR32_EBI_ADDR_2_FUNCTION           },
    {AVR32_EBI_ADDR_3_PIN,            AVR32_EBI_ADDR_3_FUNCTION           },
    {AVR32_EBI_ADDR_4_PIN,            AVR32_EBI_ADDR_4_FUNCTION           },
    {AVR32_EBI_ADDR_5_PIN,            AVR32_EBI_ADDR_5_FUNCTION           },
    {AVR32_EBI_ADDR_6_PIN,            AVR32_EBI_ADDR_6_FUNCTION           },
    {AVR32_EBI_ADDR_7_PIN,            AVR32_EBI_ADDR_7_FUNCTION           },
    {AVR32_EBI_ADDR_8_PIN,            AVR32_EBI_ADDR_8_FUNCTION           },
    {AVR32_EBI_ADDR_9_PIN,            AVR32_EBI_ADDR_9_FUNCTION           },
    {AVR32_EBI_ADDR_10_PIN,           AVR32_EBI_ADDR_10_FUNCTION          },
    {AVR32_EBI_ADDR_11_PIN,           AVR32_EBI_ADDR_11_FUNCTION          },
    {AVR32_EBI_SDA10_PIN,           AVR32_EBI_SDA10_FUNCTION          },
#if SDRAM_ROW_BITS >= 12
    {AVR32_EBI_ADDR_13_PIN,           AVR32_EBI_ADDR_13_FUNCTION          },
  #if SDRAM_ROW_BITS >= 13
    {AVR32_EBI_ADDR_14_PIN,           AVR32_EBI_ADDR_14_FUNCTION          },
  #endif
#endif

    // Enable bank address pins.
    {AVR32_EBI_ADDR_16_PIN,           AVR32_EBI_ADDR_16_FUNCTION          },
#if SDRAM_BANK_BITS >= 2
    {AVR32_EBI_ADDR_17_PIN,           AVR32_EBI_ADDR_17_FUNCTION          },
#endif

    // Enable data mask pins.
    {AVR32_EBI_ADDR_0_PIN,            AVR32_EBI_ADDR_0_FUNCTION           },
    {AVR32_EBI_NWE1_PIN,            AVR32_EBI_NWE1_FUNCTION           },
#if SDRAM_DBW >= 32
    {AVR32_EBI_ADDR_1_PIN,            AVR32_EBI_ADDR_1_FUNCTION           },
    {AVR32_EBI_NWE3_PIN,            AVR32_EBI_NWE3_FUNCTION           },
#endif

    // Enable control pins.
    {AVR32_EBI_SDWE_PIN,            AVR32_EBI_SDWE_FUNCTION           },
    {AVR32_EBI_CAS_PIN,             AVR32_EBI_CAS_FUNCTION            },
    {AVR32_EBI_RAS_PIN,             AVR32_EBI_RAS_FUNCTION            },
    {AVR32_EBI_NCS_1_PIN,             AVR32_EBI_NCS_1_FUNCTION            },

    // Enable clock-related pins.
    {AVR32_EBI_SDCK_PIN,            AVR32_EBI_SDCK_FUNCTION           },
    {AVR32_EBI_SDCKE_PIN,           AVR32_EBI_SDCKE_FUNCTION          }
  };

  gpio_enable_module(SDRAMC_EBI_GPIO_MAP, sizeof(SDRAMC_EBI_GPIO_MAP) / sizeof(SDRAMC_EBI_GPIO_MAP[0]));
}
#elif BOARD == STK1000
static void sdramc_enable_muxed_pins(void)
{
  volatile avr32_hmatrix_t *hmatrix = &AVR32_HMATRIX;

  // Enable SDRAM mode for CS1 in the BAMBI mux
  hmatrix->sfr[4] |= 0x0002;
  hmatrix->sfr[4] |= 0x0100;

  static const gpio_map_t SDRAMC_EBI_GPIO_MAP =
  {
    // Enable DATA 16 through 31 pins, which is muxed with LCD
    {AVR32_EBI_DATA_16_PIN,           AVR32_EBI_DATA_16_FUNCTION          },
    {AVR32_EBI_DATA_17_PIN,           AVR32_EBI_DATA_17_FUNCTION          },
    {AVR32_EBI_DATA_18_PIN,           AVR32_EBI_DATA_18_FUNCTION          },
    {AVR32_EBI_DATA_19_PIN,           AVR32_EBI_DATA_19_FUNCTION          },
    {AVR32_EBI_DATA_20_PIN,           AVR32_EBI_DATA_20_FUNCTION          },
    {AVR32_EBI_DATA_21_PIN,           AVR32_EBI_DATA_21_FUNCTION          },
    {AVR32_EBI_DATA_22_PIN,           AVR32_EBI_DATA_22_FUNCTION          },
    {AVR32_EBI_DATA_23_PIN,           AVR32_EBI_DATA_23_FUNCTION          },
    {AVR32_EBI_DATA_24_PIN,           AVR32_EBI_DATA_24_FUNCTION          },
    {AVR32_EBI_DATA_25_PIN,           AVR32_EBI_DATA_25_FUNCTION          },
    {AVR32_EBI_DATA_26_PIN,           AVR32_EBI_DATA_26_FUNCTION          },
    {AVR32_EBI_DATA_27_PIN,           AVR32_EBI_DATA_27_FUNCTION          },
    {AVR32_EBI_DATA_28_PIN,           AVR32_EBI_DATA_28_FUNCTION          },
    {AVR32_EBI_DATA_29_PIN,           AVR32_EBI_DATA_29_FUNCTION          },
    {AVR32_EBI_DATA_30_PIN,           AVR32_EBI_DATA_30_FUNCTION          },
    {AVR32_EBI_DATA_31_PIN,           AVR32_EBI_DATA_31_FUNCTION          }
  };

  gpio_enable_module(SDRAMC_EBI_GPIO_MAP, sizeof(SDRAMC_EBI_GPIO_MAP) / sizeof(SDRAMC_EBI_GPIO_MAP[0]));


}

#endif

void sdramc_init(unsigned long hsb_hz)
{
  unsigned long hsb_mhz_dn = hsb_hz / 1000000;
  unsigned long hsb_mhz_up = (hsb_hz + 999999) / 1000000;
  volatile ATPASTE2(U, SDRAM_DBW) *sdram = SDRAM;
  unsigned int i;

  // Put the multiplexed MCU pins used for the SDRAM under control of the SDRAMC.
  sdramc_enable_muxed_pins();

  // Enable SDRAM mode for CS1.
  AVR32_HMATRIX.sfr[AVR32_EBI_HMATRIX_NR] |= 1 << AVR32_EBI_SDRAM_CS;
  AVR32_HMATRIX.sfr[AVR32_EBI_HMATRIX_NR];

  // Configure the SDRAM Controller with SDRAM setup and timing information.
  // All timings below are rounded up because they are minimal values.
  AVR32_SDRAMC.cr =
      ((( SDRAM_COL_BITS                 -    8) << AVR32_SDRAMC_CR_NC_OFFSET  ) & AVR32_SDRAMC_CR_NC_MASK  ) |
      ((( SDRAM_ROW_BITS                 -   11) << AVR32_SDRAMC_CR_NR_OFFSET  ) & AVR32_SDRAMC_CR_NR_MASK  ) |
      ((( SDRAM_BANK_BITS                -    1) << AVR32_SDRAMC_CR_NB_OFFSET  ) & AVR32_SDRAMC_CR_NB_MASK  ) |
      ((  SDRAM_CAS                              << AVR32_SDRAMC_CR_CAS_OFFSET ) & AVR32_SDRAMC_CR_CAS_MASK ) |
      ((( SDRAM_DBW                      >>   4) << AVR32_SDRAMC_CR_DBW_OFFSET ) & AVR32_SDRAMC_CR_DBW_MASK ) |
      ((((SDRAM_TWR  * hsb_mhz_up + 999) / 1000) << AVR32_SDRAMC_CR_TWR_OFFSET ) & AVR32_SDRAMC_CR_TWR_MASK ) |
      ((((SDRAM_TRC  * hsb_mhz_up + 999) / 1000) << AVR32_SDRAMC_CR_TRC_OFFSET ) & AVR32_SDRAMC_CR_TRC_MASK ) |
      ((((SDRAM_TRP  * hsb_mhz_up + 999) / 1000) << AVR32_SDRAMC_CR_TRP_OFFSET ) & AVR32_SDRAMC_CR_TRP_MASK ) |
      ((((SDRAM_TRCD * hsb_mhz_up + 999) / 1000) << AVR32_SDRAMC_CR_TRCD_OFFSET) & AVR32_SDRAMC_CR_TRCD_MASK) |
      ((((SDRAM_TRAS * hsb_mhz_up + 999) / 1000) << AVR32_SDRAMC_CR_TRAS_OFFSET) & AVR32_SDRAMC_CR_TRAS_MASK) |
      ((((SDRAM_TXSR * hsb_mhz_up + 999) / 1000) << AVR32_SDRAMC_CR_TXSR_OFFSET) & AVR32_SDRAMC_CR_TXSR_MASK);
  AVR32_SDRAMC.cr;

  // Issue a NOP command to the SDRAM in order to start the generation of SDRAMC signals.
  AVR32_SDRAMC.mr = AVR32_SDRAMC_MR_MODE_NOP;
  AVR32_SDRAMC.mr;
  sdram[0];

  // Wait during the SDRAM stable-clock initialization delay.
  sdramc_us_delay(SDRAM_STABLE_CLOCK_INIT_DELAY, hsb_mhz_up);

  // Issue a PRECHARGE ALL command to the SDRAM.
  AVR32_SDRAMC.mr = AVR32_SDRAMC_MR_MODE_BANKS_PRECHARGE;
  AVR32_SDRAMC.mr;
  sdram[0];
  sdramc_ns_delay(SDRAM_TRP, hsb_mhz_up);

  // Issue initialization AUTO REFRESH commands to the SDRAM.
  AVR32_SDRAMC.mr = AVR32_SDRAMC_MR_MODE_AUTO_REFRESH;
  AVR32_SDRAMC.mr;
  for (i = 0; i < SDRAM_INIT_AUTO_REFRESH_COUNT; i++)
  {
    sdram[0];
    sdramc_ns_delay(SDRAM_TRFC, hsb_mhz_up);
  }

  // Issue a LOAD MODE REGISTER command to the SDRAM.
  // This configures the SDRAM with the following parameters in the mode register:
  //  - bits 0 to 2: burst length: 1 (000b);
  //  - bit 3: burst type: sequential (0b);
  //  - bits 4 to 6: CAS latency: AVR32_SDRAMC.CR.cas;
  //  - bits 7 to 8: operating mode: standard operation (00b);
  //  - bit 9: write burst mode: programmed burst length (0b);
  //  - all other bits: reserved: 0b.
  AVR32_SDRAMC.mr = AVR32_SDRAMC_MR_MODE_LOAD_MODE;
  AVR32_SDRAMC.mr;
  sdram[0];
  sdramc_ns_delay(SDRAM_TMRD, hsb_mhz_up);

  // Switch the SDRAM Controller to normal mode.
  AVR32_SDRAMC.mr = AVR32_SDRAMC_MR_MODE_NORMAL;
  AVR32_SDRAMC.mr;
  sdram[0];

  // Write the refresh period into the SDRAMC Refresh Timer Register.
  // tR is rounded down because it is a maximal value.
  AVR32_SDRAMC.tr = (SDRAM_TR * hsb_mhz_dn) / 1000;
  AVR32_SDRAMC.tr;
}
