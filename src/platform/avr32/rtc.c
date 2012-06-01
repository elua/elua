// eLua module for Mizar2 Real Time Clock hardware

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
//#include "lrodefs.h"
#include "platform_conf.h"

//#include "rtc.h"
#include "i2c.h"


// Mizar32 has a real time clock chip on the Ethernet/RTC add-on board
// which may be a DS1337 or may be a PCF8563 part. Both are on the main I2C bus
// but the DS1337 reaponds to 7-bit decimal slave address 104 (8-bit 0xD0, 0xD1)
// and the PCF8563 responds to slave address 81 (0xA2, 0xA3).
// Their functionality is much the same and their register contents are similar
// but their register layouts differ and a few bits mean different things.

// It provides two functions: mizar32.rtc.get() and mizer32.rtc.set(table).
// The first returns a table of lua-like exploded time/date fields:
//  year (1900-2099) month (1-12) day (1-13) hour (0-23) min (0-59) sec (0-59)
// The second accepts the same kind of table, replacing nil values with 0.
// Lua's isdst field is ignored and not returned (so t.isdst is nil==false).
// Both devices have a day-of-week field (with values 1-7 and 0-6 respectively);
// we may one day either use this or just calculate the DOW.

// DS1337 registers: http://datasheets.maxim-ic.com/en/ds/DS1337-DS1337C.pdf p.8
// PCF8563 registers: http://www.nxp.com/documents/data_sheet/PCF8563.pdf p.6

// Slave addresses of the two possible RTC chips
#define DS1337_SLAVE_ADDRESS 104
#define PCF8536_SLAVE_ADDRESS 81

// Which type of RTC clock do we have?
static enum rtc_type {
  RTC_DS1337,
  RTC_PCF8536,
  RTC_SYSTEM,	// No RTC present: use the system timer.
} rtc_type;

// Both devices respond up to 100kHz so set maximum I2C frequency.
#define RTC_BUS_FREQUENCY 100000

static u8 rtc_slave_address;    // 7-bit I2C slave address of RTC chip
static u8 rtc_registers_start;  // Index in hardware of "seconds" register

/* Initialise the RTC subsystem:
 * - Probe the possible devices to know what kind of RTC we have
 * - If none, do the RTC stuff using the one-second-tick system timer
 */
static void rtc_init()
{
  static u8 rtc_initialized = 0;	// Have we probed it?

  if (rtc_initialized) return;

  if ( i2c_probe( DS1337_SLAVE_ADDRESS ) ) {
    rtc_type = RTC_DS1337;
    rtc_slave_address = DS1337_SLAVE_ADDRESS;
    rtc_registers_start = 0;
  } else if( i2c_probe( PCF8536_SLAVE_ADDRESS ) ) {
    rtc_type = RTC_PCF8536;
    rtc_slave_address = PCF8536_SLAVE_ADDRESS;
    rtc_registers_start = 2;
  } else { 
    rtc_type = RTC_SYSTEM;
  }

  rtc_initialized++;
}

// The names of the Lua time table elements to return, in the order they are
// in the DS1337 register set.
static const char * const fieldnames[] = {
  "sec", "min", "hour", "wday", "day", "month", "year"
};
static const u16 minval[] = {
  0, 0, 0, 1, 1, 1, 1900
};
static const u16 maxval[] = {
  59, 59, 23, 7, 31, 12, 2099
};

// The number of fields in the above tables, also the number of bytes in the
// chip's time-oriented register set.
#define NFIELDS 7

// The order offsets of the fields in the DS1337 register set
#define SEC   0
#define MIN   1
#define HOUR  2
#define WDAY  3
#define DAY   4
#define MONTH 5
#define YEAR  6

// Convert from BCD to a regular integer
static unsigned fromBCD(u8 bcd)
{
   return (bcd & 0xF) + (bcd >> 4) * 10;
}

// Convert from an integer to 2-digit BCD
static u8 toBCD(unsigned n)
{
   return ( ( n / 10 ) << 4 ) | ( n % 10 );
}

static void read_rtc_regs(u8 *regs)
{
  // Read the chip's registers into our buffer
  i2c_send(rtc_slave_address, &rtc_registers_start, 1, false);
  i2c_recv(rtc_slave_address, regs, NFIELDS, true);

  // Convert PCF8536 data format to DS1337 data format
  if( rtc_type == RTC_PCF8536 ) {
    u8 tmp;

    // day-of-week and day-of-month are swapped
    // and day-of-week is 0-6 instead of Lua's 1-7
    tmp = regs[WDAY];           // reads "date" register
    regs[WDAY] = regs[DAY] + 1; // reads "weekday"register"
    regs[DAY] = tmp;
  }
}

static void write_rtc_regs(u8 *regs)
{
  // Convert DS1337 data format to PCF8536 data format
  if( rtc_type == RTC_PCF8536 ) {
    u8 tmp;

    // day-of-week and day-of-month are swapped
    // and day-of-week is 0-6 instead of Lua's 1-7
    tmp = regs[WDAY];
    regs[WDAY] = regs[DAY]; // writes to PCF's "date" register
    regs[DAY] = tmp - 1;    // writes to PCF's weekday register
  }
  // The buffer has a spare byte at the start to poke the register address into
  regs[-1] = rtc_registers_start;
  i2c_send(rtc_slave_address, regs - 1, NFIELDS+1, true);
}

// Read the time from the RTC.
static int rtc_get( lua_State *L )
{
  u8 regs[7];  // seconds to years register contents, read from device

  rtc_init();

  if( rtc_type == RTC_SYSTEM ) {
    // Unimplemented as yet
    return luaL_error( L, "No real-time clock chip present" );
  }

  read_rtc_regs( regs );

  // Construct the table to return the result
  lua_createtable( L, 0, 7 );

  lua_pushstring( L, "sec" );
  lua_pushinteger( L, fromBCD(regs[SEC] & 0x7F) );
  lua_rawset( L, -3 );

  lua_pushstring( L, "min" );
  lua_pushinteger( L, fromBCD(regs[MIN] & 0x7F) );
  lua_rawset( L, -3 );

  lua_pushstring( L, "hour" );
  lua_pushinteger( L, fromBCD(regs[HOUR] & 0x3F) );
  lua_rawset( L, -3 );

  lua_pushstring( L, "wday" );
  lua_pushinteger( L, regs[WDAY] & 0x07 );
  lua_rawset( L, -3 );

  lua_pushstring( L, "day" );
  lua_pushinteger( L, fromBCD(regs[DAY] & 0x3F) );
  lua_rawset( L, -3 );

  lua_pushstring( L, "month" );
  lua_pushinteger( L, fromBCD(regs[MONTH] & 0x1F) );
  lua_rawset( L, -3 );

  lua_pushstring( L, "year" );
  lua_pushinteger( L, fromBCD(regs[YEAR] & 0xFF)            // Year in century
                      + ( ( regs[MONTH] & 0x80 ) ? 2000 : 1900 ) ); // Century
  lua_rawset( L, -3 );

  return 1;
}

// mizar32.rtc.set()
// Parameter is a table containing fields with the usual Lua time field
// names.  Missing elements are not set and remain the same as they were.

static int rtc_set( lua_State *L )
{
  u8 buf[NFIELDS+1];	// I2C message: First byte is the register address
  u8 *regs = buf + 1;	// The registers
  lua_Integer value;
  int field;         // Which field are we handling (0-6)

  rtc_init();

  if( rtc_type == RTC_SYSTEM ) {
    // Unimplemented as yet
    return luaL_error( L, "No real-time clock chip present" );
  }

  // Read the chip's registers into our buffer so that unspecified fields
  // remain at the same value as before
  read_rtc_regs( regs );

  // Set any values that they specified as table entries
  for (field=0; field<NFIELDS; field++) {
    lua_getfield( L, 1, fieldnames[field] );
    switch( lua_type( L, -1 ) ) {
    case LUA_TNIL:
      // Do not set unspecified fields
      break;

    case LUA_TNUMBER:
    case LUA_TSTRING:
      value = lua_tointeger( L, -1 );
      if (value < minval[field] || value > maxval[field])
        return luaL_error( L, "Time value out of range" );

      // Special cases for some fields
      switch( field ) {
      case SEC: case MIN: case HOUR:
      case WDAY: case DAY:
        regs[field] = toBCD( value );
        break;
      case MONTH:
        // Set new month but preserve century bit in case they don't set the year
        regs[MONTH] = ( regs[MONTH] & 0x80 ) | toBCD( value );
        break;
      case YEAR:
        if ( value < 2000 ) {
          value -= 1900; regs[MONTH] &= 0x7F;
        } else {
          value -= 2000; regs[MONTH] |= 0x80;
        }
        regs[YEAR] = toBCD( value );
        break;
      }
      break;

    default:
      return luaL_error( L, "Time values must be numbers" );
    }
    lua_pop( L, 1 );
  }

  write_rtc_regs( regs );

  return 0;
}

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"

// mizar32.rtc.*() module function map
const LUA_REG_TYPE rtc_map[] =
{
  { LSTRKEY( "get" ), LFUNCVAL( rtc_get ) },
  { LSTRKEY( "set" ), LFUNCVAL( rtc_set ) },
  { LNILKEY, LNILVAL }
};
