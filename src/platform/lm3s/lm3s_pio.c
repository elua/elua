// LM3S specific PIO support

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "inc/hw_gpio.h"
#include "gpio.h"
#include "auxmods.h"
#include <string.h>

#if LUA_OPTIMIZE_MEMORY == 0
#error lm3s.pio can only be compiled with LTR on (optram=true)
#endif

// Alternate function setting is not available on all CPUs
#if defined( ELUA_CPU_LM3S9B92 ) || defined( ELUA_CPU_LM3S9D92 )
#define LM3S_HAS_ALTERNATE_PIO
#endif

// ****************************************************************************
// Alternate function handling

#ifdef LM3S_HAS_ALTERNATE_PIO

// List all alternative pin functions here. Long live gpio.h...
#define LM3S_ALTERNATE_FUNCTIONS\
  _M( PA0_U0RX ),\
  _M( PA0_I2C1SCL ),\
  _M( PA0_U1RX ),\
  _M( PA1_U0TX ),\
  _M( PA1_I2C1SDA ),\
  _M( PA1_U1TX ),\
  _M( PA2_SSI0CLK ),\
  _M( PA2_TXD2 ),\
  _M( PA2_PWM4 ),\
  _M( PA2_I2S0RXSD ),\
  _M( PA3_SSI0FSS ),\
  _M( PA3_TXD1 ),\
  _M( PA3_PWM5 ),\
  _M( PA3_I2S0RXMCLK ),\
  _M( PA4_SSI0RX ),\
  _M( PA4_TXD0 ),\
  _M( PA4_PWM6 ),\
  _M( PA4_CAN0RX ),\
  _M( PA4_I2S0TXSCK ),\
  _M( PA5_SSI0TX ),\
  _M( PA5_RXDV ),\
  _M( PA5_PWM7 ),\
  _M( PA5_CAN0TX ),\
  _M( PA5_I2S0TXWS ),\
  _M( PA6_I2C1SCL ),\
  _M( PA6_CCP1 ),\
  _M( PA6_RXCK ),\
  _M( PA6_PWM0 ),\
  _M( PA6_PWM4 ),\
  _M( PA6_CAN0RX ),\
  _M( PA6_USB0EPEN ),\
  _M( PA6_U1CTS ),\
  _M( PA7_I2C1SDA ),\
  _M( PA7_CCP4 ),\
  _M( PA7_RXER ),\
  _M( PA7_PWM1 ),\
  _M( PA7_PWM5 ),\
  _M( PA7_CAN0TX ),\
  _M( PA7_CCP3 ),\
  _M( PA7_USB0PFLT ),\
  _M( PA7_U1DCD ),\
  _M( PB0_CCP0 ),\
  _M( PB0_PWM2 ),\
  _M( PB0_U1RX ),\
  _M( PB1_CCP2 ),\
  _M( PB1_PWM3 ),\
  _M( PB1_CCP1 ),\
  _M( PB1_U1TX ),\
  _M( PB2_I2C0SCL ),\
  _M( PB2_IDX0 ),\
  _M( PB2_CCP3 ),\
  _M( PB2_CCP0 ),\
  _M( PB2_USB0EPEN ),\
  _M( PB3_I2C0SDA ),\
  _M( PB3_FAULT0 ),\
  _M( PB3_FAULT3 ),\
  _M( PB3_USB0PFLT ),\
  _M( PB4_U2RX ),\
  _M( PB4_CAN0RX ),\
  _M( PB4_IDX0 ),\
  _M( PB4_U1RX ),\
  _M( PB4_EPI0S23 ),\
  _M( PB5_C0O ),\
  _M( PB5_CCP5 ),\
  _M( PB5_CCP6 ),\
  _M( PB5_CCP0 ),\
  _M( PB5_CAN0TX ),\
  _M( PB5_CCP2 ),\
  _M( PB5_U1TX ),\
  _M( PB5_EPI0S22 ),\
  _M( PB6_CCP1 ),\
  _M( PB6_CCP7 ),\
  _M( PB6_C0O ),\
  _M( PB6_FAULT1 ),\
  _M( PB6_IDX0 ),\
  _M( PB6_CCP5 ),\
  _M( PB6_I2S0TXSCK ),\
  _M( PB7_NMI ),\
  _M( PB7_RXD1 ),\
  _M( PC0_TCK ),\
  _M( PC1_TMS ),\
  _M( PC2_TDI ),\
  _M( PC3_TDO ),\
  _M( PC4_CCP5 ),\
  _M( PC4_PHA0 ),\
  _M( PC4_TXD3 ),\
  _M( PC4_PWM6 ),\
  _M( PC4_CCP2 ),\
  _M( PC4_CCP4 ),\
  _M( PC4_EPI0S2 ),\
  _M( PC4_CCP1 ),\
  _M( PC5_CCP1 ),\
  _M( PC5_C1O ),\
  _M( PC5_C0O ),\
  _M( PC5_FAULT2 ),\
  _M( PC5_CCP3 ),\
  _M( PC5_USB0EPEN ),\
  _M( PC5_EPI0S3 ),\
  _M( PC6_CCP3 ),\
  _M( PC6_PHB0 ),\
  _M( PC6_C2O ),\
  _M( PC6_PWM7 ),\
  _M( PC6_U1RX ),\
  _M( PC6_CCP0 ),\
  _M( PC6_USB0PFLT ),\
  _M( PC6_EPI0S4 ),\
  _M( PC7_CCP4 ),\
  _M( PC7_PHB0 ),\
  _M( PC7_CCP0 ),\
  _M( PC7_U1TX ),\
  _M( PC7_USB0PFLT ),\
  _M( PC7_C1O ),\
  _M( PC7_EPI0S5 ),\
  _M( PD0_PWM0 ),\
  _M( PD0_CAN0RX ),\
  _M( PD0_IDX0 ),\
  _M( PD0_U2RX ),\
  _M( PD0_U1RX ),\
  _M( PD0_CCP6 ),\
  _M( PD0_RXDV ),\
  _M( PD0_I2S0RXSCK ),\
  _M( PD0_U1CTS ),\
  _M( PD1_PWM1 ),\
  _M( PD1_CAN0TX ),\
  _M( PD1_PHA0 ),\
  _M( PD1_U2TX ),\
  _M( PD1_U1TX ),\
  _M( PD1_CCP7 ),\
  _M( PD1_TXER ),\
  _M( PD1_I2S0RXWS ),\
  _M( PD1_U1DCD ),\
  _M( PD1_CCP2 ),\
  _M( PD1_PHB1 ),\
  _M( PD2_U1RX ),\
  _M( PD2_CCP6 ),\
  _M( PD2_PWM2 ),\
  _M( PD2_CCP5 ),\
  _M( PD2_EPI0S20 ),\
  _M( PD3_U1TX ),\
  _M( PD3_CCP7 ),\
  _M( PD3_PWM3 ),\
  _M( PD3_CCP0 ),\
  _M( PD3_EPI0S21 ),\
  _M( PD4_CCP0 ),\
  _M( PD4_CCP3 ),\
  _M( PD4_TXD3 ),\
  _M( PD4_I2S0RXSD ),\
  _M( PD4_U1RI ),\
  _M( PD4_EPI0S19 ),\
  _M( PD5_CCP2 ),\
  _M( PD5_CCP4 ),\
  _M( PD5_TXD2 ),\
  _M( PD5_I2S0RXMCLK ),\
  _M( PD5_U2RX ),\
  _M( PD5_EPI0S28 ),\
  _M( PD6_FAULT0 ),\
  _M( PD6_TXD1 ),\
  _M( PD6_I2S0TXSCK ),\
  _M( PD6_U2TX ),\
  _M( PD6_EPI0S29 ),\
  _M( PD7_IDX0 ),\
  _M( PD7_C0O ),\
  _M( PD7_CCP1 ),\
  _M( PD7_TXD0 ),\
  _M( PD7_I2S0TXWS ),\
  _M( PD7_U1DTR ),\
  _M( PD7_EPI0S30 ),\
  _M( PE0_PWM4 ),\
  _M( PE0_SSI1CLK ),\
  _M( PE0_CCP3 ),\
  _M( PE0_EPI0S8 ),\
  _M( PE0_USB0PFLT ),\
  _M( PE1_PWM5 ),\
  _M( PE1_SSI1FSS ),\
  _M( PE1_FAULT0 ),\
  _M( PE1_CCP2 ),\
  _M( PE1_CCP6 ),\
  _M( PE1_EPI0S9 ),\
  _M( PE2_CCP4 ),\
  _M( PE2_SSI1RX ),\
  _M( PE2_PHB1 ),\
  _M( PE2_PHA0 ),\
  _M( PE2_CCP2 ),\
  _M( PE2_EPI0S24 ),\
  _M( PE3_CCP1 ),\
  _M( PE3_SSI1TX ),\
  _M( PE3_PHA1 ),\
  _M( PE3_PHB0 ),\
  _M( PE3_CCP7 ),\
  _M( PE3_EPI0S25 ),\
  _M( PE4_CCP3 ),\
  _M( PE4_CAN2RX ),\
  _M( PE4_FAULT0 ),\
  _M( PE4_U2TX ),\
  _M( PE4_CCP2 ),\
  _M( PE4_RXD0 ),\
  _M( PE4_I2S0TXWS ),\
  _M( PE5_CCP5 ),\
  _M( PE5_CAN2TX ),\
  _M( PE5_I2S0TXSD ),\
  _M( PE6_PWM4 ),\
  _M( PE6_C1O ),\
  _M( PE6_U1CTS ),\
  _M( PE7_PWM5 ),\
  _M( PE7_C2O ),\
  _M( PE7_U1DCD ),\
  _M( PF0_CAN1RX ),\
  _M( PF0_PHB0 ),\
  _M( PF0_PWM0 ),\
  _M( PF0_RXCK ),\
  _M( PF0_I2S0TXSD ),\
  _M( PF0_U1DSR ),\
  _M( PF1_CAN1TX ),\
  _M( PF1_IDX1 ),\
  _M( PF1_PWM1 ),\
  _M( PF1_RXER ),\
  _M( PF1_I2S0TXMCLK ),\
  _M( PF1_U1RTS ),\
  _M( PF1_CCP3 ),\
  _M( PF2_LED1 ),\
  _M( PF2_PWM4 ),\
  _M( PF2_PHYINT ),\
  _M( PF2_PWM2 ),\
  _M( PF2_SSI1CLK ),\
  _M( PF3_LED0 ),\
  _M( PF3_PWM5 ),\
  _M( PF3_MDC ),\
  _M( PF3_PWM3 ),\
  _M( PF3_SSI1FSS ),\
  _M( PF4_CCP0 ),\
  _M( PF4_C0O ),\
  _M( PF4_MDIO ),\
  _M( PF4_FAULT0 ),\
  _M( PF4_EPI0S12 ),\
  _M( PF4_SSI1RX ),\
  _M( PF5_CCP2 ),\
  _M( PF5_C1O ),\
  _M( PF5_RXD3 ),\
  _M( PF5_EPI0S15 ),\
  _M( PF5_SSI1TX ),\
  _M( PF6_CCP1 ),\
  _M( PF6_C2O ),\
  _M( PF6_RXD2 ),\
  _M( PF6_PHA0 ),\
  _M( PF6_I2S0TXMCLK ),\
  _M( PF6_U1RTS ),\
  _M( PF7_CCP4 ),\
  _M( PF7_RXD1 ),\
  _M( PF7_PHB0 ),\
  _M( PF7_EPI0S12 ),\
  _M( PF7_FAULT1 ),\
  _M( PG0_U2RX ),\
  _M( PG0_PWM0 ),\
  _M( PG0_I2C1SCL ),\
  _M( PG0_PWM4 ),\
  _M( PG0_USB0EPEN ),\
  _M( PG0_EPI0S13 ),\
  _M( PG1_U2TX ),\
  _M( PG1_PWM1 ),\
  _M( PG1_I2C1SDA ),\
  _M( PG1_PWM5 ),\
  _M( PG1_EPI0S14 ),\
  _M( PG2_PWM0 ),\
  _M( PG2_COL ),\
  _M( PG2_FAULT0 ),\
  _M( PG2_IDX1 ),\
  _M( PG2_I2S0RXSD ),\
  _M( PG3_PWM1 ),\
  _M( PG3_CRS ),\
  _M( PG3_FAULT2 ),\
  _M( PG3_FAULT0 ),\
  _M( PG3_I2S0RXMCLK ),\
  _M( PG4_CCP3 ),\
  _M( PG4_RXD0 ),\
  _M( PG4_FAULT1 ),\
  _M( PG4_EPI0S15 ),\
  _M( PG4_PWM6 ),\
  _M( PG4_U1RI ),\
  _M( PG5_CCP5 ),\
  _M( PG5_TXEN ),\
  _M( PG5_IDX0 ),\
  _M( PG5_FAULT1 ),\
  _M( PG5_PWM7 ),\
  _M( PG5_I2S0RXSCK ),\
  _M( PG5_U1DTR ),\
  _M( PG6_PHA1 ),\
  _M( PG6_TXCK ),\
  _M( PG6_PWM6 ),\
  _M( PG6_FAULT1 ),\
  _M( PG6_I2S0RXWS ),\
  _M( PG6_U1RI ),\
  _M( PG7_PHB1 ),\
  _M( PG7_TXER ),\
  _M( PG7_PWM7 ),\
  _M( PG7_CCP5 ),\
  _M( PG7_EPI0S31 ),\
  _M( PH0_CCP6 ),\
  _M( PH0_PWM2 ),\
  _M( PH0_EPI0S6 ),\
  _M( PH0_PWM4 ),\
  _M( PH1_CCP7 ),\
  _M( PH1_PWM3 ),\
  _M( PH1_EPI0S7 ),\
  _M( PH1_PWM5 ),\
  _M( PH2_IDX1 ),\
  _M( PH2_C1O ),\
  _M( PH2_FAULT3 ),\
  _M( PH2_EPI0S1 ),\
  _M( PH2_TXD3 ),\
  _M( PH3_PHB0 ),\
  _M( PH3_FAULT0 ),\
  _M( PH3_USB0EPEN ),\
  _M( PH3_EPI0S0 ),\
  _M( PH3_TXD2 ),\
  _M( PH4_USB0PFLT ),\
  _M( PH4_EPI0S10 ),\
  _M( PH4_TXD1 ),\
  _M( PH4_SSI1CLK ),\
  _M( PH5_EPI0S11 ),\
  _M( PH5_TXD0 ),\
  _M( PH5_FAULT2 ),\
  _M( PH5_SSI1FSS ),\
  _M( PH6_EPI0S26 ),\
  _M( PH6_RXDV ),\
  _M( PH6_PWM4 ),\
  _M( PH6_SSI1RX ),\
  _M( PH7_RXCK ),\
  _M( PH7_EPI0S27 ),\
  _M( PH7_PWM5 ),\
  _M( PH7_SSI1TX ),\
  _M( PJ0_RXER ),\
  _M( PJ0_EPI0S16 ),\
  _M( PJ0_PWM0 ),\
  _M( PJ0_I2C1SCL ),\
  _M( PJ1_EPI0S17 ),\
  _M( PJ1_USB0PFLT ),\
  _M( PJ1_PWM1 ),\
  _M( PJ1_I2C1SDA ),\
  _M( PJ2_EPI0S18 ),\
  _M( PJ2_CCP0 ),\
  _M( PJ2_FAULT0 ),\
  _M( PJ3_EPI0S19 ),\
  _M( PJ3_U1CTS ),\
  _M( PJ3_CCP6 ),\
  _M( PJ4_EPI0S28 ),\
  _M( PJ4_U1DCD ),\
  _M( PJ4_CCP4 ),\
  _M( PJ5_EPI0S29 ),\
  _M( PJ5_U1DSR ),\
  _M( PJ5_CCP2 ),\
  _M( PJ6_EPI0S30 ),\
  _M( PJ6_U1RTS ),\
  _M( PJ6_CCP1 ),\
  _M( PJ7_U1DTR ),\
  _M( PJ7_CCP0 ),

typedef struct
{
  const char *name;
  u32 val;
} LM3S_PIN_DATA;

#define _M( x )   { #x, GPIO_##x }
static const LM3S_PIN_DATA lm3s_pin_data[] = 
{
  LM3S_ALTERNATE_FUNCTIONS
  { NULL, 0 }
};

static int lm3s_pio_mt_index( lua_State *L )
{
  const char *key = luaL_checkstring( L, 2 );
  unsigned i = 0;
  
  while( lm3s_pin_data[ i ].name != NULL )
  {
    if( !strcmp( lm3s_pin_data[ i ].name, key ) )
    {
      lua_pushnumber( L, ( lua_Number )lm3s_pin_data[ i ].val );
      return 1;
    }
    i ++;
  }
  return 0;
}

// Lua: lm3s.pio.set_function( func1, func2, ..., funcn )
static int lm3s_pio_set_function( lua_State *L )
{
  unsigned i;

  for( i = 1; i <= lua_gettop( L ); i ++ )
    GPIOPinConfigure( ( u32 )luaL_checknumber( L, i ) );
  return 0;
}

#endif // #ifdef LM3S_HAS_ALTERNATE_PIO

// ****************************************************************************
// Other LM3S PIO specific functions

extern const u32 pio_base[];

// Helper: check a port/pin specification
// Return 1 if OK, 0 if false
// Set port and pin in args as side effect
static int lm3s_pioh_check_pio_spec( int v, int *pport, int *ppin )
{
  *pport = PLATFORM_IO_GET_PORT( v );
  *ppin = PLATFORM_IO_GET_PIN( v );
  if( PLATFORM_IO_IS_PORT( v ) || !platform_pio_has_port( *pport ) || !platform_pio_has_pin( *pport, *ppin ) )
    return 0;
  return 1;
}

// Lua: lm3s.pio.set_strength( drive, pin1, pin2, ..., pinn )
static int lm3s_pio_set_strength( lua_State *L )
{
  int port = 0, pin = 0;
  u8 pins;
  u32 base;
  u32 drive = luaL_checkinteger( L, 1 );
  unsigned i;
  
  for( i = 2; i <= lua_gettop( L ); i ++ )
  {
    if( !lm3s_pioh_check_pio_spec( luaL_checkinteger( L, i ), &port, &pin ) )
      return luaL_error( L, "invalid pin '%u'", luaL_checkinteger( L, i ) );
    base = pio_base[ port ];
    pins = 1 << pin;
    // The next sequence is taken from gpio.c
    HWREG(base + GPIO_O_DR2R) = ((drive & 1) ?
                                   (HWREG(base + GPIO_O_DR2R) | pins) :
                                   (HWREG(base + GPIO_O_DR2R) & ~(pins)));
    HWREG(base + GPIO_O_DR4R) = ((drive & 2) ?
                                   (HWREG(base + GPIO_O_DR4R) | pins) :
                                   (HWREG(base + GPIO_O_DR4R) & ~(pins)));
    HWREG(base + GPIO_O_DR8R) = ((drive & 4) ?
                                   (HWREG(base + GPIO_O_DR8R) | pins) :
                                   (HWREG(base + GPIO_O_DR8R) & ~(pins)));
    HWREG(base + GPIO_O_SLR) = ((drive & 8) ?
                                  (HWREG(base + GPIO_O_SLR) | pins) :
                                  (HWREG(base + GPIO_O_SLR) & ~(pins)));
  }
  return 0;
}

// Lua: lm3s.pio.set_direction( dir, pin1, pin2, ..., pinn )
static int lm3s_pio_set_direction( lua_State *L )
{
  int port = 0, pin = 0;
  u32 base, dir;
  u8 pins;
  unsigned i;

  dir = ( u32 )luaL_checkinteger( L, 1 );
  for( i = 2; i <= lua_gettop( L ); i ++ )
  {
    if( !lm3s_pioh_check_pio_spec( luaL_checkinteger( L, i ), &port, &pin ) )
      return luaL_error( L, "invalid pin '%u'", luaL_checkinteger( L, i ) );
    base = pio_base[ port ];
    pins = 1 << pin;
    GPIODirModeSet( base, pins, dir );
    HWREG( base  + GPIO_O_DEN ) |= pins;
  }
  return 0;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h" 
const LUA_REG_TYPE lm3s_pio_map[] =
{
#ifdef LM3S_HAS_ALTERNATE_PIO
  { LSTRKEY( "__index" ), LFUNCVAL( lm3s_pio_mt_index ) },
  { LSTRKEY( "__metatable" ), LROVAL( lm3s_pio_map ) },
  { LSTRKEY( "set_function" ), LFUNCVAL( lm3s_pio_set_function ) },
#endif // #ifdef LM3S_HAS_ALTERNATE_PIO
  { LSTRKEY( "set_strength" ),  LFUNCVAL( lm3s_pio_set_strength ) },
  { LSTRKEY( "MA_2" ), LNUMVAL( GPIO_STRENGTH_2MA ) },
  { LSTRKEY( "MA_4" ), LNUMVAL( GPIO_STRENGTH_4MA ) },
  { LSTRKEY( "MA_8" ), LNUMVAL( GPIO_STRENGTH_8MA ) },
  { LSTRKEY( "MA_8SC" ), LNUMVAL( GPIO_STRENGTH_8MA_SC ) },
  { LSTRKEY( "set_direction" ), LFUNCVAL( lm3s_pio_set_direction ) },
  { LSTRKEY( "GPIO_IN" ), LNUMVAL( GPIO_DIR_MODE_IN ) },
  { LSTRKEY( "GPIO_OUT" ), LNUMVAL( GPIO_DIR_MODE_OUT ) },
  { LSTRKEY( "HW" ), LNUMVAL( GPIO_DIR_MODE_HW ) },
  { LNILKEY, LNILVAL }
};

