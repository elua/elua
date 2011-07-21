// eLua module for Mizar32 LCD character display

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"

#include "disp.h"       
#include "i2c.h"

// Declaration to save/restore the I2C clock rate
extern u32 i2c_delay;
static u32 old_i2c_delay;

// Two functions bracket all I2C packets to the LCD module
// to save, change and restore the I2C clock rate.
static void disp_start()
{
  old_i2c_delay = i2c_delay;
  i2c_delay = REQ_CPU_FREQ / DISP_BUS_FREQ / 2;
}

static void disp_stop()
{
  i2c_delay = old_i2c_delay;
}

// Utility function: pause for N * 5ms
static void disp_delay(unsigned long n)
{
  // Code stolen from sdramc.c::sdramc_ck_delay()

  // Use the CPU cycle counter (CPU and HSB clocks are the same).
  u32 delay_start_cycle = Get_system_register(AVR32_COUNT);
  u32 delay_end_cycle = delay_start_cycle + n * DISP_DELAY_TICKS;
 
  // To be safer, the end of wait is based on an inequality test, so CPU cycle
  // counter wrap around is checked.
  if (delay_start_cycle > delay_end_cycle)
  { 
    while ((unsigned long)Get_system_register(AVR32_COUNT) > delay_end_cycle);
  }
  while ((unsigned long)Get_system_register(AVR32_COUNT) < delay_end_cycle);
}

// Generic function to emit an LCD command byte, normally used in a tail call.
static int disp_command(unsigned char command)
{
  disp_start();
  i2c_start_cond();
  i2c_write_byte( DISP_CMD );
  i2c_write_byte( command );
  i2c_stop_cond();
  disp_stop();
  disp_delay(1);
  return 0;
}

//Lua: mizar32.disp.clear()
static int disp_clear(lua_State *L) {
  return disp_command( DISP_CMD_CLEAR );
}

//Lua: mizar32.disp.goto( row, col )
static int disp_goto(lua_State *L) {
  unsigned row = luaL_checkinteger( L, 1 );
  unsigned col = luaL_checkinteger( L, 2 );
  unsigned address;

  if ( row < 1 || row > 2 || col < 1 || col > 16 ) {
    return luaL_error( L, "row/column must be 1-2 and 1-16" );
  }

  address = ( row - 1 ) * 0x40 + ( col - 1 ) ;

  return disp_command( DISP_CMD_DDADDR + address );
}

//Lua: mizar32.disp.print( string )
static int disp_print(lua_State *L) {
  size_t len;  // Number of chars in string
  const char *str = luaL_checklstring( L, 1, &len );

  disp_start();
  while (len > 0) {
    int nbytes = 0;	// number of bytes sent in this I2C packet

    i2c_start_cond();
    i2c_write_byte( DISP_DATA );
    // Mizar32 LCD module has a maximum of 31 bytes per data packet
    while ( len > 0 && nbytes < 31 ) {
      i2c_write_byte( *str++ );
      nbytes++; len--;
    }
    i2c_stop_cond();
    disp_delay(nbytes);
  }
  disp_stop();
  return 0;
}


#define MIN_OPT_LEVEL 2
#include "lrodefs.h"

// Module function map
const LUA_REG_TYPE disp_map[] =
{
  { LSTRKEY( "clear" ), LFUNCVAL( disp_clear ) },
  { LSTRKEY( "goto" ),  LFUNCVAL( disp_goto ) },
  { LSTRKEY( "print" ),  LFUNCVAL( disp_print ) },
};

LUALIB_API int luaopen_disp( lua_State *L )
{
  LREGISTER( L, AUXLIB_DISP, disp_map );
}
