
// An eLua module for elua-zeisig-gemacht's default TFT interface

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "auxmods.h"
#include <string.h>

/*******************************************************************************
 * Macros
 *******************************************************************************/

#define TFT_INTERFACE            (2U)

/* Font stuff */
#define UD_DIGITAL_40X80_INDEX   (0x07)
#define UD_DIGITAL_40X80_CDOTS   (0x11)
#define UD_DIGITAL_40X80_CMODE   (0xC0)

/* Colors */
#define COLOR_BLACK              (0x0000)
#define COLOR_WHITE              (0xFFFF)
#define COLOR_ORANGE             (0xF144)

/* Low byte access */
#define lowByte(v)               ((unsigned char) (v))

/* High byte access */
#define highByte(v)              ((unsigned char) (((unsigned int) (v)) >> 8))

/* Handy serial comm commands */

#define SERIAL_WRITE(x)\
  platform_uart_send(TFT_INTERFACE, x);

#define START_FRAME\
  SERIAL_WRITE(0xAA)

#define END_FRAME\
  SERIAL_WRITE(0xCC);\
  SERIAL_WRITE(0x33);\
  SERIAL_WRITE(0xC3);\
  SERIAL_WRITE(0x3C)

/*******************************************************************************
 * TFT module code
 *******************************************************************************/

//Lua: init( baud, databits, parity, stopbits )
static int tft_init( lua_State *L )
{
  unsigned databits, parity, stopbits;
  u32 baud, res;

  baud = luaL_checkinteger( L, 1 );
  databits = luaL_checkinteger( L, 2 );
  parity = luaL_checkinteger( L, 3 );
  stopbits = luaL_checkinteger( L, 4 );
  res = platform_uart_setup( TFT_INTERFACE, baud, databits, parity, stopbits );
  lua_pushinteger( L, res );

  return 1;
}

//Lua: setcolor( fc, bc )
static int tft_set_color( lua_State *L )
{
  uint16_t fc = luaL_checkinteger( L, 1 );
  uint16_t bc = luaL_checkinteger( L, 2 );

  START_FRAME;
  SERIAL_WRITE( 0x40 );
  SERIAL_WRITE( highByte( fc ) );
  SERIAL_WRITE( lowByte( fc ) );
  SERIAL_WRITE( highByte( bc ) );
  SERIAL_WRITE( lowByte( bc ) );
  END_FRAME;

  return 0;
}

//Lua: clear()
static int tft_clear( lua_State *L )
{
  START_FRAME;
  SERIAL_WRITE( 0x52 );
  END_FRAME;

  return 0;
}

//Lua: on()
static int tft_on( lua_State *L )
{
  START_FRAME;
  SERIAL_WRITE( 0x5F );
  END_FRAME;

  return 0;
}

//Lua: off()
static int tft_off( lua_State *L )
{
  START_FRAME;
  SERIAL_WRITE( 0x5E );
  END_FRAME;

  return 0;
}

//Lua: loadimage( index )
static int tft_load_image( lua_State *L )
{
  uint16_t index = luaL_checkinteger( L, 1 );

  START_FRAME;
  SERIAL_WRITE( 0x70 );
  SERIAL_WRITE( highByte( index ) );
  SERIAL_WRITE( lowByte( index ) );
  END_FRAME;

  return 0;
}

//Lua: calibrate()
static int tft_calibrate( lua_State *L )
{
  START_FRAME;
  SERIAL_WRITE( 0x00 );
  END_FRAME;

  START_FRAME;
  SERIAL_WRITE( 0xE4 );
  SERIAL_WRITE( 0x55 );
  SERIAL_WRITE( 0xAA );
  SERIAL_WRITE( 0x5A );
  SERIAL_WRITE( 0xA5 );
  END_FRAME;

  return 0;
}

//Lua: loadicon( id, xstart, xend, ystart, yend, x, y )
static int tft_load_icon( lua_State *L )
{
  uint16_t id = luaL_checkinteger( L, 1 );
  uint16_t xs = luaL_checkinteger( L, 2 );
  uint16_t ys = luaL_checkinteger( L, 3 );
  uint16_t xe = luaL_checkinteger( L, 4 );
  uint16_t ye = luaL_checkinteger( L, 5 );
  uint16_t x = luaL_checkinteger( L, 6 );
  uint16_t y = luaL_checkinteger( L, 7 );

  START_FRAME;
  SERIAL_WRITE( 0x70 );
  SERIAL_WRITE( 0x02 );
  SERIAL_WRITE( highByte( id ) );
  SERIAL_WRITE( lowByte( id ) );
  SERIAL_WRITE( highByte( xs ) );
  SERIAL_WRITE( lowByte( xs ) );
  SERIAL_WRITE( highByte( ys ) );
  SERIAL_WRITE( lowByte( ys ) );
  SERIAL_WRITE( highByte( xe ) );
  SERIAL_WRITE( lowByte( xe ) );
  SERIAL_WRITE( highByte( ye ) );
  SERIAL_WRITE( lowByte( ye ) );
  SERIAL_WRITE( highByte( x ) );
  SERIAL_WRITE( lowByte( x ) );
  SERIAL_WRITE( highByte( y ) );
  SERIAL_WRITE( lowByte( y ) );
  END_FRAME;

  return 0;
}

/* Text display */
static void text_display( uint16_t x,
			  uint16_t y,
			  uint8_t font,
			  uint8_t cmode,
			  uint8_t cdots,
			  uint16_t fgc,
			  uint16_t bgc,
			  const char *str,
			  uint8_t len )
{
  uint8_t i;

  START_FRAME;
  SERIAL_WRITE( 0x98 );
  SERIAL_WRITE( highByte( x ) );
  SERIAL_WRITE( lowByte( x ) );
  SERIAL_WRITE( highByte( y ) );
  SERIAL_WRITE( lowByte( y ) );
  SERIAL_WRITE( font );
  SERIAL_WRITE( cmode );
  SERIAL_WRITE( cdots );
  SERIAL_WRITE( highByte( fgc ) );
  SERIAL_WRITE( lowByte( fgc) );
  SERIAL_WRITE( highByte( bgc ) );
  SERIAL_WRITE( lowByte( bgc ) );

  /* Transmit the string */
  for ( i = 0; i < len; i++ )
    SERIAL_WRITE( str[ i ] );

  END_FRAME;
}

//Lua: print( str, x, y )
static int tft_print( lua_State *L )
{
  const char *str;
  uint16_t x, y;

  str = luaL_checkstring( L, 1 );
  x = luaL_checkinteger( L, 2 );
  y = luaL_checkinteger( L, 3 );

  text_display( x,
		y,
		UD_DIGITAL_40X80_INDEX,
		UD_DIGITAL_40X80_CMODE,
		UD_DIGITAL_40X80_CDOTS,
		COLOR_WHITE,
		COLOR_BLACK,
		str,
		strlen( str ) );
  return 0;
}

//Lua: brightness( level )
static int tft_brightness( lua_State *L )
{
  uint8_t brightness = luaL_checkinteger( L, 1 );

  START_FRAME;
  SERIAL_WRITE( 0x5F );
  SERIAL_WRITE( brightness );
  END_FRAME;

  return 0;
}

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"

// Module function map
const LUA_REG_TYPE tft_map[] =
{
  { LSTRKEY( "init" ),  LFUNCVAL( tft_init ) },
  { LSTRKEY( "setcolor" ),  LFUNCVAL( tft_set_color ) },
  { LSTRKEY( "clear" ),  LFUNCVAL( tft_clear ) },
  { LSTRKEY( "on" ),  LFUNCVAL( tft_on ) },
  { LSTRKEY( "off" ),  LFUNCVAL( tft_off ) },
  { LSTRKEY( "loadimage" ),  LFUNCVAL( tft_load_image ) },
  { LSTRKEY( "loadicon" ),  LFUNCVAL( tft_load_icon ) },
  { LSTRKEY( "calibrate" ),  LFUNCVAL( tft_calibrate ) },
  { LSTRKEY( "print" ),  LFUNCVAL( tft_print ) },
  { LSTRKEY( "brightness" ),  LFUNCVAL( tft_brightness ) },
  //
  // Expose colors: The monster's interface is (for most people -
  // boring) simple. It should work just fine for echo of process
  // control parameters and the few tiny games I care about.
  //
  { LSTRKEY( "COLOR_WHITE" ), LNUMVAL( COLOR_WHITE ) },
  { LSTRKEY( "COLOR_BLACK" ), LNUMVAL( COLOR_BLACK ) },
  { LSTRKEY( "COLOR_ORANGE" ), LNUMVAL( COLOR_ORANGE ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_tft( lua_State *L )
{
  LREGISTER( L, AUXLIB_TFT, tft_map );
}

