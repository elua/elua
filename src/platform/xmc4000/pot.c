
// eLua module to read the value of the on-board pot. The maximum
// value given by the ADC is 4095.
//
// The pot module was specifically written to get eLua's SpaceShip
// game running on the XMC4500. There is just one general purpose push
// button on the Hexagon. In that situation, the pot can be used to
// move the ship around the screen instead of push buttons.
//
// The ADC part of the XMC4000's EasyMain example is used in this
// module.

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "DAVE.h"

// Lua: xmc4000.pot.read()
static int pot_adc0_read(lua_State *L) {
  unsigned long adc_result;

  do {
    adc_result =  VADC_G0->RES[1];
  } while (!(adc_result >> VADC_G_RES_VF_Pos));

  adc_result &= 0xFFF;
  lua_pushinteger(L, (lua_Integer)adc_result);

  return 1;
}

// Lua: xmc4000.pot.init()
//
// This function should be called before invoking read().
static int pot_adc0_init(lua_State *L) {
  SCU_RESET->PRCLR0 |= ((1 << SCU_RESET_PRSET0_VADCRS_Pos) & SCU_RESET_PRSET0_VADCRS_Msk);

  // Global ADC Initialization
  VADC->CLC = ((0 << VADC_CLC_DISR_Pos) & VADC_CLC_DISR_Msk) |
              ((0 << VADC_CLC_DISS_Pos) & VADC_CLC_DISS_Msk) |
              ((0 << VADC_CLC_EDIS_Pos) & VADC_CLC_EDIS_Msk);

  // Wait for module clock enabled
  while(VADC->CLC!=0);
  VADC->GLOBCFG = ((6 << VADC_GLOBCFG_DIVA_Pos) & VADC_GLOBCFG_DIVA_Msk) |
                  ((1 << VADC_GLOBCFG_DIVWC_Pos) & VADC_GLOBCFG_DIVWC_Msk);

  VADC_G0->ARBCFG = ((VADC_G0->ARBCFG &
                     ~(VADC_G_ARBCFG_ANONC_Msk | VADC_G_ARBCFG_ARBRND_Msk | VADC_G_ARBCFG_ARBM_Msk)) |
                     (((3 << VADC_G_ARBCFG_ANONC_Pos) & VADC_G_ARBCFG_ANONC_Msk) |
                     ((0 << VADC_G_ARBCFG_ARBRND_Pos) & VADC_G_ARBCFG_ARBRND_Msk)|
                     ((0 << VADC_G_ARBCFG_ARBM_Pos) & VADC_G_ARBCFG_ARBM_Msk)));
  
  VADC_G0->ARBPR = ((VADC_G0->ARBPR &
                   ~(VADC_G_ARBPR_ASEN2_Msk | VADC_G_ARBPR_PRIO0_Msk)) |
                   (((1 << VADC_G_ARBPR_PRIO0_Pos) & VADC_G_ARBPR_PRIO0_Msk) |
                   ((1 << VADC_G_ARBPR_ASEN2_Pos) & VADC_G_ARBPR_ASEN2_Msk)));

  // Conversion result storage
  VADC_G0->CHCTR[1] = ((0x1 << VADC_G_CHCTR_RESREG_Pos) & VADC_G_CHCTR_RESREG_Msk);

  // Background source
  VADC->BRSSEL[0] = (1UL << 1);

  VADC->BRSCTRL = ((0 << VADC_BRSCTRL_XTSEL_Pos) & VADC_BRSCTRL_XTSEL_Msk) |
                  ((0 << VADC_BRSCTRL_XTMODE_Pos) & VADC_BRSCTRL_XTMODE_Msk) |
                  ((1 << VADC_BRSCTRL_XTWC_Pos) & VADC_BRSCTRL_XTWC_Msk) |
                  ((0 << VADC_BRSCTRL_GTSEL_Pos) & VADC_BRSCTRL_GTSEL_Msk) |
                  ((1 << VADC_BRSCTRL_GTWC_Pos) & VADC_BRSCTRL_GTWC_Msk);

  VADC->BRSMR = ((1 << VADC_BRSMR_ENGT_Pos) & VADC_BRSMR_ENGT_Msk) |
               	((0 << VADC_BRSMR_ENTR_Pos) & VADC_BRSMR_ENTR_Msk) |
                ((0 << VADC_BRSMR_ENSI_Pos) & VADC_BRSMR_ENSI_Msk) |
                ((1 << VADC_BRSMR_SCAN_Pos) & VADC_BRSMR_SCAN_Msk) |
                ((0 << VADC_BRSMR_LDM_Pos) & VADC_BRSMR_LDM_Msk) |
                ((1 << VADC_BRSMR_LDEV_Pos) & VADC_BRSMR_LDEV_Msk);

  return 0;
}

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE pot_map[] =
{
  { LSTRKEY( "init" ),  LFUNCVAL( pot_adc0_init ) },
  { LSTRKEY( "read" ),  LFUNCVAL( pot_adc0_read ) },
  { LNILKEY, LNILVAL }
};
