
// eLua module for XMC4500's hardware RTC module. Code taken from
// Mizar2's RTC implementation and adapted for the XMC4500.

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
//#include "lrodefs.h"
#include "platform_conf.h"
#include "DAVE.h"

/*
 * All defines here
 */

// The number of fields in the above tables
#define NFIELDS 6

// The order offsets of the fields in the DS1337 register set
#define SEC   0
#define MIN   1
#define HOUR  2
#define WDAY  3
#define DAY   4
#define MONTH 5
#define YEAR  6

// The names of the Lua time table elements to return, in the order they are
// in the DS1337 register set.
static const char * const fieldnames[] = {
  "sec", "min", "hour", "day", "month", "year"
};

static const u16 minval[] = {
  0, 0, 0, 1, 1, 1, 1900
};

static const u16 maxval[] = {
  59, 59, 23, 7, 31, 12, 2099
};

/*
 * Functions start here
 */

// Initialize the RTC module
static int rtc_init( lua_State *L ) {
  RTC_Init(&RTC_0);
  return 0;
}

// Get elapsed number of seconds
static int rtc_elapsed( lua_State *L ) {
  time_t t;

  RTC_Time(&t);
  lua_pushinteger(L, (lua_Integer)t);
  return 1;
}

// Get elapsed number of seconds
static int rtc_status( lua_State *L ) {
  lua_pushinteger(L, (lua_Integer)RTC_GetFlagStatus());
  return 1;
}

// Read the time from the RTC.
static int rtc_get( lua_State *L ) {
  XMC_RTC_TIME_t t;

  // Get the time.
  RTC_GetTime( &t );

  // Construct the table to return the result
  lua_createtable( L, 0, 6 );

  lua_pushstring( L, "sec" );
  lua_pushinteger( L, t.seconds );
  lua_rawset( L, -3 );

  lua_pushstring( L, "min" );
  lua_pushinteger( L, t.minutes );
  lua_rawset( L, -3 );

  lua_pushstring( L, "hour" );
  lua_pushinteger( L, t.hours );
  lua_rawset( L, -3 );

  lua_pushstring( L, "day" );
  lua_pushinteger( L, t.days );
  lua_rawset( L, -3 );

  lua_pushstring( L, "month" );
  lua_pushinteger( L, t.month );
  lua_rawset( L, -3 );

  lua_pushstring( L, "year" );
  lua_pushinteger( L, t.year );
  lua_rawset( L, -3 );

  return 1;
}

// xmc4000.rtc.set()
// Parameter is a table containing fields with the usual Lua time
// field names. Missing elements are not set and remain the same as
// they were.
static int rtc_set( lua_State *L ) {
  lua_Integer value;
  int field; // Which field are we handling (0-5)
  XMC_RTC_TIME_t t;

  // Set any values that they specified as table entries
  for( field = 0; field < NFIELDS; field++ ) {
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
          case SEC: t.seconds = value; break;
          case MIN: t.minutes = value; break;
          case HOUR: t.hours  = value; break;
          case DAY: t.days = value; break;
	  case MONTH: t.month = value; break;
          case YEAR: t.year = value; break;
        };
        break;
      default:
        return luaL_error( L, "Time values must be numbers" );
    };

    lua_pop( L, 1 );
  }

  RTC_SetTime( &t );

  return 0;
}

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"

// xmc4000.rtc.*() module function map
const LUA_REG_TYPE rtc_map[] = {
  { LSTRKEY( "init" ), LFUNCVAL( rtc_init ) },
  { LSTRKEY( "get" ), LFUNCVAL( rtc_get ) },
  { LSTRKEY( "set" ), LFUNCVAL( rtc_set ) },
  { LSTRKEY( "elapsed" ), LFUNCVAL( rtc_elapsed ) },
  { LSTRKEY( "status" ), LFUNCVAL( rtc_status ) },
  { LNILKEY, LNILVAL }
};
