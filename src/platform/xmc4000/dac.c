
// eLua Module for talking to the XMC's DAC peripheral
//
// Raman: I would really like to have this as a generic eLua module in
// src/modules/dac.c. More thoughts on stack. For now, we'll hack a
// simple function generator.

#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"
#include "platform_conf.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "xmc_dac.h"

/* Local reference for a global DAC */
#define ELUA_XMC_DAC ( ( XMC_DAC_t  * )( void * )DAC )

#define DAC_PATTERN_FIELDS 9

/* The DAC's amplitude in milli volts */
typedef enum DAC_AMP_MILLI_VOLTS_t {
  DAC_AMP_MILLI_VOLTS_1 = 0,
  DAC_AMP_MILLI_VOLTS_2,
  DAC_AMP_MILLI_VOLTS_4,      
  DAC_AMP_MILLI_VOLTS_9,
  DAC_AMP_MILLI_VOLTS_17,
  DAC_AMP_MILLI_VOLTS_34,
  DAC_AMP_MILLI_VOLTS_69,
  DAC_AMP_MILLI_VOLTS_138,
  DAC_AMP_MILLI_VOLTS_275,
  DAC_AMP_MILLI_VOLTS_550,
  DAC_AMP_MILLI_VOLTS_1100,
  DAC_AMP_MILLI_VOLTS_2200,
} DAC_AMP_MILLI_VOLTS_t;

// ****************************************************************************
// DAC pattern mode

// Lua: dac.pattern.start( id )
static int dac_pattern_start( lua_State *L )
{
  unsigned id;

  id = luaL_checkinteger( L, 1 );
  if( !( id < NUM_DAC ) )
    return luaL_error( L, "dac %d does not exist", ( unsigned )id );
  XMC_DAC_CH_EnableOutput( ELUA_XMC_DAC, id );
  return 0;
}

// Lua: dac.pattern.stop( id )
static int dac_pattern_stop( lua_State *L )
{
  unsigned id;

  id = luaL_checkinteger( L, 1 );
  if( !( id < NUM_DAC ) )
    return luaL_error( L, "dac %d does not exist", ( unsigned )id );
  XMC_DAC_CH_DisableOutput( ELUA_XMC_DAC, id );
  return 0;
}

// Lua: dac.pattern.setup( id, pattern, amplitude, freq )
// Raman: How about the DAC offsets? You can generate some really
// strange signals with it!
static int dac_pattern_setup( lua_State *L )
{
  XMC_DAC_CH_CONFIG_t dac_config;
  uint32_t output_scale = 0;
  int field; // Which field are we handling ( 0 - 8 )
  uint8_t pat[ DAC_PATTERN_FIELDS ];
  DAC_AMP_MILLI_VOLTS_t amp;
  uint32_t freq;
  unsigned id;
  uint8_t patval;

  id = luaL_checkinteger( L, 1 );
  if( !( id < NUM_DAC ) )
    return luaL_error( L, "dac %d does not exist", ( unsigned )id );

  luaL_checktype( L, 2, LUA_TTABLE );
  if( lua_objlen( L, 2) != DAC_PATTERN_FIELDS )
    return luaL_error( L, "Pattern length must be 9 (PAT0, .. PAT8)" );
  for( field = 0; field < DAC_PATTERN_FIELDS; field++ )
  {
    lua_pushinteger( L, field + 1 );
    lua_gettable( L, 2 );
    if( lua_isnumber( L, -1 ) )
    {
      patval = ( uint8_t )lua_tonumber( L, -1 );
      if( patval < 0 || patval > 31 )
	return luaL_error( L, "DAC's 5-bit pattern values must be between 0 - 31" );
      pat[ field ] = patval;
    }
    else
    {
      return luaL_error( L, "DAC pattern: Expected number ( 0 - 31 )" );
    }
    lua_pop( L, 1 );
  }

  amp = luaL_checkinteger( L, 3 );
  if( amp < DAC_AMP_MILLI_VOLTS_1 || amp > DAC_AMP_MILLI_VOLTS_2200 )
    return luaL_error( L, "Invalid DAC amplitude setting" );

  freq = luaL_checkinteger( L, 4 );
  if( freq <= 0 )
    return luaL_error( L, "DAC frequency must be > 0" );

  dac_config.data_type = XMC_DAC_CH_DATA_TYPE_SIGNED;
  dac_config.output_negation = XMC_DAC_CH_OUTPUT_NEGATION_DISABLED;
  dac_config.output_offset = 0U;

  switch( amp )
  {
    case DAC_AMP_MILLI_VOLTS_1:
      output_scale = XMC_DAC_CH_OUTPUT_SCALE_DIV_32;
      break;
    case DAC_AMP_MILLI_VOLTS_2:
      output_scale = XMC_DAC_CH_OUTPUT_SCALE_DIV_16;
      break;
    case DAC_AMP_MILLI_VOLTS_4:
      output_scale = XMC_DAC_CH_OUTPUT_SCALE_DIV_8;
      break;
    case DAC_AMP_MILLI_VOLTS_9:
      output_scale = XMC_DAC_CH_OUTPUT_SCALE_DIV_4;
      break;
    case DAC_AMP_MILLI_VOLTS_17:
      output_scale = XMC_DAC_CH_OUTPUT_SCALE_DIV_2;
      break;
    case DAC_AMP_MILLI_VOLTS_34:
      output_scale = XMC_DAC_CH_OUTPUT_SCALE_NONE;
      break;
    case DAC_AMP_MILLI_VOLTS_69:
      output_scale = XMC_DAC_CH_OUTPUT_SCALE_MUL_2;
      break;
    case DAC_AMP_MILLI_VOLTS_138:
      output_scale = XMC_DAC_CH_OUTPUT_SCALE_MUL_4;
      break;
    case DAC_AMP_MILLI_VOLTS_275:
      output_scale = XMC_DAC_CH_OUTPUT_SCALE_MUL_8;
      break;
    case DAC_AMP_MILLI_VOLTS_550:
      output_scale = XMC_DAC_CH_OUTPUT_SCALE_MUL_16;
      break;
    case DAC_AMP_MILLI_VOLTS_1100:
      output_scale = XMC_DAC_CH_OUTPUT_SCALE_MUL_32;
      break;
    case DAC_AMP_MILLI_VOLTS_2200:
      output_scale = XMC_DAC_CH_OUTPUT_SCALE_MUL_64;
      break;
  }
  dac_config.output_scale = output_scale;

  XMC_DAC_CH_Init( ELUA_XMC_DAC, id, &dac_config );
  XMC_DAC_CH_StartPatternMode( ELUA_XMC_DAC, id, pat, XMC_DAC_CH_PATTERN_SIGN_OUTPUT_DISABLED, XMC_DAC_CH_TRIGGER_INTERNAL, freq );
  //XMC_DAC_CH_SetMode( ELUA_XMC_DAC, id, XMC_DAC_CH_MODE_IDLE );

  return 0;
  
}

// Lua: dac.noise.setup( id, amplitude, period )
static int dac_noise_setup( lua_State *L )
{
  XMC_DAC_CH_CONFIG_t dac_config;
  uint32_t output_scale = 0;
  DAC_AMP_MILLI_VOLTS_t amp;
  uint32_t freq;
  unsigned id;

  id = luaL_checkinteger( L, 1 );
  if( !( id < NUM_DAC ) )
    return luaL_error( L, "dac %d does not exist", ( unsigned )id );

  amp = luaL_checkinteger( L, 2 );
  if( amp < DAC_AMP_MILLI_VOLTS_17 || amp > DAC_AMP_MILLI_VOLTS_2200 )
    return luaL_error( L, "Invalid DAC amplitude setting" );

  freq = luaL_checkinteger( L, 3 );
  if( freq <= 0 )
    return luaL_error( L, "DAC noise period must be > 0" );

  dac_config.data_type = XMC_DAC_CH_DATA_TYPE_SIGNED;
  dac_config.output_negation = XMC_DAC_CH_OUTPUT_NEGATION_DISABLED;
  dac_config.output_offset = 0U;

  switch( amp )
  {
    case DAC_AMP_MILLI_VOLTS_1:
    case DAC_AMP_MILLI_VOLTS_2:
    case DAC_AMP_MILLI_VOLTS_4:
    case DAC_AMP_MILLI_VOLTS_9:
    case DAC_AMP_MILLI_VOLTS_17:
      output_scale = XMC_DAC_CH_OUTPUT_SCALE_DIV_128;
      break;
    case DAC_AMP_MILLI_VOLTS_34:
      output_scale = XMC_DAC_CH_OUTPUT_SCALE_DIV_64;
      break;
    case DAC_AMP_MILLI_VOLTS_69:
      output_scale = XMC_DAC_CH_OUTPUT_SCALE_DIV_32;
      break;
    case DAC_AMP_MILLI_VOLTS_138:
      output_scale = XMC_DAC_CH_OUTPUT_SCALE_DIV_16;
      break;
    case DAC_AMP_MILLI_VOLTS_275:
      output_scale = XMC_DAC_CH_OUTPUT_SCALE_DIV_8;
      break;
    case DAC_AMP_MILLI_VOLTS_550:
      output_scale = XMC_DAC_CH_OUTPUT_SCALE_DIV_4;
      break;
    case DAC_AMP_MILLI_VOLTS_1100:
      output_scale = XMC_DAC_CH_OUTPUT_SCALE_DIV_2;
      break;
    case DAC_AMP_MILLI_VOLTS_2200:
      output_scale = XMC_DAC_CH_OUTPUT_SCALE_NONE;
      break;
  }
  dac_config.output_scale = output_scale;

  XMC_DAC_CH_Init( ELUA_XMC_DAC, id, &dac_config );
  XMC_DAC_CH_StartNoiseMode( ELUA_XMC_DAC, id, XMC_DAC_CH_TRIGGER_INTERNAL, (uint32_t)roundf(1000000/ freq));

  return 0;

}

// *****************************************************************************
// DAC function map

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"

static const LUA_REG_TYPE dac_pattern_map[] =
{
  { LSTRKEY( "setup" ), LFUNCVAL( dac_pattern_setup ) },
  { LNILKEY, LNILVAL }
};

static const LUA_REG_TYPE dac_noise_map[] =
{
  { LSTRKEY( "setup" ), LFUNCVAL( dac_noise_setup ) },
  { LNILKEY, LNILVAL }
};

const LUA_REG_TYPE dac_map[] =
{
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "start" ), LFUNCVAL ( dac_pattern_start ) },
  { LSTRKEY( "stop" ), LFUNCVAL( dac_pattern_stop ) },
  { LSTRKEY( "pattern" ), LROVAL( dac_pattern_map ) },
  { LSTRKEY( "noise" ), LROVAL( dac_noise_map ) },
  { LSTRKEY( "MILLI_VOLTS_1" ), LNUMVAL( DAC_AMP_MILLI_VOLTS_1 ) },
  { LSTRKEY( "MILLI_VOLTS_2" ), LNUMVAL( DAC_AMP_MILLI_VOLTS_2 ) },
  { LSTRKEY( "MILLI_VOLTS_4" ), LNUMVAL( DAC_AMP_MILLI_VOLTS_4 ) },
  { LSTRKEY( "MILLI_VOLTS_9" ), LNUMVAL( DAC_AMP_MILLI_VOLTS_9 ) },
  { LSTRKEY( "MILLI_VOLTS_17" ), LNUMVAL( DAC_AMP_MILLI_VOLTS_17 ) },
  { LSTRKEY( "MILLI_VOLTS_34" ), LNUMVAL( DAC_AMP_MILLI_VOLTS_34 ) },
  { LSTRKEY( "MILLI_VOLTS_69" ), LNUMVAL( DAC_AMP_MILLI_VOLTS_69 ) },
  { LSTRKEY( "MILLI_VOLTS_138" ), LNUMVAL( DAC_AMP_MILLI_VOLTS_138 ) },
  { LSTRKEY( "MILLI_VOLTS_275" ), LNUMVAL( DAC_AMP_MILLI_VOLTS_275 ) },
  { LSTRKEY( "MILLI_VOLTS_550" ), LNUMVAL( DAC_AMP_MILLI_VOLTS_550 ) },
  { LSTRKEY( "MILLI_VOLTS_1100" ), LNUMVAL( DAC_AMP_MILLI_VOLTS_1100 ) },
  { LSTRKEY( "MILLI_VOLTS_2200" ), LNUMVAL( DAC_AMP_MILLI_VOLTS_2200 ) },
  { LSTRKEY( "__metatable" ), LROVAL( dac_map ) },
#endif
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_dac( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, AUXLIB_DAC, dac_map );

  // Set it as its own metatable
  lua_pushvalue( L, -1 );
  lua_setmetatable( L, -2 );
  
  // Set constants for direction/pullups
  MOD_REG_NUMBER( L, "MILLI_VOLTS_1", DAC_AMP_MILLI_VOLTS_1 );
  MOD_REG_NUMBER( L, "MILLI_VOLTS_2", DAC_AMP_MILLI_VOLTS_2 );
  MOD_REG_NUMBER( L, "MILLI_VOLTS_4", DAC_AMP_MILLI_VOLTS_4 );
  MOD_REG_NUMBER( L, "MILLI_VOLTS_9", DAC_AMP_MILLI_VOLTS_9 );
  MOD_REG_NUMBER( L, "MILLI_VOLTS_17", DAC_AMP_MILLI_VOLTS_17 );
  MOD_REG_NUMBER( L, "MILLI_VOLTS_34", DAC_AMP_MILLI_VOLTS_34 );
  MOD_REG_NUMBER( L, "MILLI_VOLTS_69", DAC_AMP_MILLI_VOLTS_69 );
  MOD_REG_NUMBER( L, "MILLI_VOLTS_138", DAC_AMP_MILLI_VOLTS_138 );
  MOD_REG_NUMBER( L, "MILLI_VOLTS_275", DAC_AMP_MILLI_VOLTS_275 );
  MOD_REG_NUMBER( L, "MILLI_VOLTS_550", DAC_AMP_MILLI_VOLTS_550 );
  MOD_REG_NUMBER( L, "MILLI_VOLTS_1100", DAC_AMP_MILLI_VOLTS_1100 );
  MOD_REG_NUMBER( L, "MILLI_VOLTS_2200", DAC_AMP_MILLI_VOLTS_2200);

  // Setup the new tables (pattern, others) inside dac
  lua_newtable( L );
  luaL_register( L, NULL, dac_pattern_map );
  lua_setfield( L, -2, "pattern" );

  lua_newtable( L );
  luaL_register( L, NULL, dac_noise_map );
  lua_setfield( L, -2, "noise" );

  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}


