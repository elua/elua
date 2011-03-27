/*
 * File:    mcg.c
 * Purpose: Driver for enabling the PLL in 1 of 4 options
 *
 * Notes:
 * Assumes the MCG mode is in the default FEI mode out of reset
 * One of 4 clocking oprions can be selected.
 * One of 16 crystal values can be used
 */

#include "common_kinetis.h"
#include "mcg.h"

unsigned char pll_init(unsigned char clk_option, unsigned char crystal_val)
{
  unsigned char pll_freq;

  if (clk_option > 3) {return 0;} //return 0 if one of the available options is not selected
  if (crystal_val > 15) {return 1;} // return 1 if one of the available crystal options is not available
//This assumes that the MCG is in default FEI mode out of reset.

// First move to FBE mode
#if (defined(K60_CLK) || defined(ASB817))
     MCG_C2 = 0;
#else
// Enable external oscillator, RANGE=2, HGO=1, EREFS=1, LP=0, IRCS=0
    MCG_C2 = MCG_C2_RANGE(2) | MCG_C2_HGO_MASK | MCG_C2_EREFS_MASK;
#endif

// Select external oscilator and Reference Divider and clear IREFS to start ext osc
// CLKS=2, FRDIV=3, IREFS=0, IRCLKEN=0, IREFSTEN=0
  MCG_C1 = MCG_C1_CLKS(2) | MCG_C1_FRDIV(3);

  /* if we aren't using an osc input we don't need to wait for the osc to init */
#if (!defined(K60_CLK) && !defined(ASB817))
    while (!(MCG_S & MCG_S_OSCINIT_MASK)){};  // wait for oscillator to initialize
#endif

  while (MCG_S & MCG_S_IREFST_MASK){}; // wait for Reference clock Status bit to clear

  while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x2){}; // Wait for clock status bits to show clock source is ext ref clk

// Now in FBE

#if (defined(K60_CLK))
   MCG_C5 = MCG_C5_PRDIV(0x18);
#else
// Configure PLL Ref Divider, PLLCLKEN=0, PLLSTEN=0, PRDIV=5
// The crystal frequency is used to select the PRDIV value. Only even frequency crystals are supported
// that will produce a 2MHz reference clock to the PLL.
  MCG_C5 = MCG_C5_PRDIV(crystal_val); // Set PLL ref divider to match the crystal used
#endif

  // Ensure MCG_C6 is at the reset default of 0. LOLIE disabled, PLL disabled, clk monitor disabled, PLL VCO divider is clear
  MCG_C6 = 0x0;
// Select the PLL VCO divider and system clock dividers depending on clocking option
  switch (clk_option) {
    case 0:
      // Set system options dividers
      //MCG=PLL, core = MCG, bus = MCG, FlexBus = MCG, Flash clock= MCG/2
      SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) | SIM_CLKDIV1_OUTDIV2(0) | SIM_CLKDIV1_OUTDIV3(0) | SIM_CLKDIV1_OUTDIV4(1);
      // Set the VCO divider and enable the PLL for 50MHz, LOLIE=0, PLLS=1, CME=0, VDIV=1
      MCG_C6 = MCG_C6_PLLS_MASK | MCG_C6_VDIV(1); //VDIV = 1 (x25)
      pll_freq = 50;
      break;
   case 1:
      // Set system options dividers
      //MCG=PLL, core = MCG, bus = MCG/2, FlexBus = MCG/2, Flash clock= MCG/4
      SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) | SIM_CLKDIV1_OUTDIV2(1) | SIM_CLKDIV1_OUTDIV3(1) | SIM_CLKDIV1_OUTDIV4(3);
      // Set the VCO divider and enable the PLL for 100MHz, LOLIE=0, PLLS=1, CME=0, VDIV=26
      MCG_C6 = MCG_C6_PLLS_MASK | MCG_C6_VDIV(26); //VDIV = 26 (x50)
      pll_freq = 100;
      break;
    case 2:
      // Set system options dividers
      //MCG=PLL, core = MCG, bus = MCG/2, FlexBus = MCG/2, Flash clock= MCG/4
      SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) | SIM_CLKDIV1_OUTDIV2(1) | SIM_CLKDIV1_OUTDIV3(1) | SIM_CLKDIV1_OUTDIV4(3);
      // Set the VCO divider and enable the PLL for 96MHz, LOLIE=0, PLLS=1, CME=0, VDIV=24
      MCG_C6 = MCG_C6_PLLS_MASK | MCG_C6_VDIV(24); //VDIV = 24 (x48)
      pll_freq = 96;
      break;
   case 3:
      // Set system options dividers
      //MCG=PLL, core = MCG, bus = MCG, FlexBus = MCG, Flash clock= MCG/2
      SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) | SIM_CLKDIV1_OUTDIV2(0) | SIM_CLKDIV1_OUTDIV3(0) | SIM_CLKDIV1_OUTDIV4(1);
      // Set the VCO divider and enable the PLL for 48MHz, LOLIE=0, PLLS=1, CME=0, VDIV=0
      MCG_C6 = MCG_C6_PLLS_MASK; //VDIV = 0 (x24)
      pll_freq = 48;
      break;
  }
  while (!(MCG_S & MCG_S_PLLST_MASK)){}; // wait for PLL status bit to set

  while (!(MCG_S & MCG_S_LOCK_MASK)){}; // Wait for LOCK bit to set

// Now running PBE Mode

// Transition into PEE by setting CLKS to 0
// CLKS=0, FRDIV=3, IREFS=0, IRCLKEN=0, IREFSTEN=0
  MCG_C1 &= ~MCG_C1_CLKS_MASK;

// Wait for clock status bits to update
  while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x3){};

// Now running PEE Mode

return pll_freq;
} //pll_init