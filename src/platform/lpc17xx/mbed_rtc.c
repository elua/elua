// MBED specific RTC support
#include <string.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "auxmods.h"
#include "LPC17xx.h"
#include "mbed_rtc.h"
#include <time.h>
#include <math.h>
#include <stdlib.h>

// ****************************************************************************
// Helpers and local variables

static const char *mbed_datetime_names[] = { "day", "month", "year", "hour", "min", "sec" };
static const uint16_t mbed_datetime_max[] = { 31,   12,      9999,   24,     60,    60};
static const uint16_t mbed_datetime_min[] = { 1,    1,       0,      0,      0,     0};
static const void * mbed_datetime_regs[] = { &(LPC_RTC->DOM), &(LPC_RTC->MONTH), &(LPC_RTC->YEAR), 
                                             &(LPC_RTC->HOUR), &(LPC_RTC->MIN), &(LPC_RTC->SEC)}; 
static const void * mbed_datetime_alregs[] = { &(LPC_RTC->ALDOM), &(LPC_RTC->ALMON), &(LPC_RTC->ALYEAR), 
                                             &(LPC_RTC->ALHOUR), &(LPC_RTC->ALMIN), &(LPC_RTC->ALSEC)}; 

// ****************************************************************************

// C LIB

void platform_rtc_get( int* day, int* month, int* year, int* hour, int* min, int* sec  )
{
  *year = LPC_RTC->YEAR;
  *month = LPC_RTC->MONTH;
  *day = LPC_RTC->DOM;

  *hour = LPC_RTC->HOUR;
  *min = LPC_RTC->MIN;
  *sec = LPC_RTC->SEC;
}

void platform_rtc_set( int day, int month, int year, int hour, int min, int sec )
{
  // RTC OFF
  LPC_RTC->CCR = 0;

  // Set datetime
  LPC_RTC->HOUR = hour; 
  LPC_RTC->MIN = min;
  LPC_RTC->SEC = sec;
  
  LPC_RTC->YEAR = year;
  LPC_RTC->MONTH = month;
  LPC_RTC->DOM = day;

  // RTC ON, Calibration OFF
  LPC_RTC->CCR = 1 | 1<<4; // Clock enabled, calibration disabled
}

void platform_rtc_setalarm( int day, int month, int year, int hour, int min, int sec )
{
  // Set datetime
  LPC_RTC->ALYEAR = year;
  LPC_RTC->ALMON = month;
  LPC_RTC->ALDOM = day;

  LPC_RTC->ALHOUR = hour; 
  LPC_RTC->ALMIN = min;
  LPC_RTC->ALSEC = sec;

  // Set mask ( Ignore DOY and DOW )
  LPC_RTC->AMR = 1<<4 | 1<<5;
}

// ****************************************************************************

// LUA Lib

// Lua: mbed.rtc.set( arg )
// arg can be a string formated as dd/mm/yyyy hh:mm:ss
// or a table with 'day', 'month', 'year', 'hour', 'minute' and 'second' fields
static int mbed_rtc_set( lua_State *L )
{
  const char *data;
  int vals[6] = {-1, -1, -1, -1, -1, -1}; 
  unsigned i;
  int sz;
  char error[20] = "invalid "; // Error message
  uint8_t update = 0;          // Each bit flags a reg for update

  // If we receive a string, split the time using sscanf
  if( lua_isstring( L, 1 ) )
  {
    data = luaL_checkstring( L, 1 );

    if( sscanf( data, "%d/%d/%d %d:%d:%d%n", &vals[0], &vals[1], &vals[2], &vals[3], &vals[4], &vals[5], &sz ) != 6 || sz != strlen( data ) )
      return luaL_error( L, "invalid datetime format" );

    update = 0x3F;
  }
  else // If we receive a table, get the values by their name
  {
    // Read all values
    luaL_checktype( L, 1, LUA_TTABLE );
    for( i = 0; i < 6; i ++ )
    {
      lua_getfield( L, 1, mbed_datetime_names[i] );

      if ( lua_type( L, -1 ) == LUA_TNUMBER )
      {
        vals[i] = luaL_checkinteger( L, -1 );
        update |= 1<<i;
      }

      lua_pop( L, 1 );
    }
  }

  // Check all values first ( avoid partial update in case of errors )
  for( i = 0; i < 6; i ++ )
    if ( update & 1<<i )
      if ( vals[i] < mbed_datetime_min[i] || vals[i] > mbed_datetime_max[i] )
      {
        strcat( error, mbed_datetime_names[i] );
        return luaL_error( L, error );
      }

  // RTC OFF
  LPC_RTC->CCR = 0;

  // Copy the values to the registers
  for( i = 0; i < 6; i ++ )
    if ( update & 1<<i )
    {
      if (i == 2) // Year is the only 16bit val
        *((uint16_t *)mbed_datetime_regs[i]) = vals[i];
      else
        *((uint8_t *)mbed_datetime_regs[i]) = vals[i];
    }

  // RTC ON, Calibration OFF
  LPC_RTC->CCR = 1 | 1<<4; // Clock enabled, calibration disabled

  return 0;
}

// Lua: time = mbed.rtc.get( format )
// format can be '*s' to return the datetime as a string dd/mm/yyyy hh:mm:ss
// or '*t' to return the time as a table with fields 'day', 'month', 'year', 'hour', 'minute' and 'second'
static int mbed_rtc_get( lua_State *L )
{
  int dd = -1, mon = -1, yy = -1, hh = -1, mm = -1, ss = -1;
  int *pvals[] = { &dd, &mon, &yy, &hh, &mm, &ss };
  const char *fmt; 
  unsigned i;
  char buff[ 51 ];
  struct tm t;
  char m_days[] = {31,28,31,30,31,30,31,31,30,31,30,31};

  platform_rtc_get( &dd, &mon, &yy, &hh, &mm, &ss );

  t.tm_yday = 0;
  for (i=0; i<mon-1; i++)
    t.tm_yday += m_days[i];

  if (( yy % 4 == 0 ) && (mon > 2))
    t.tm_yday ++;

  t.tm_yday += dd;
  
  t.tm_sec = ss;
  t.tm_min = mm;
  t.tm_hour = hh;
  t.tm_mday = dd;
  t.tm_mon = mon -1;
  t.tm_year = yy -1900;
  t.tm_isdst = 0;

  // From wikipedia:
  t.tm_wday = (dd+=mon<3?yy--:yy-2,23*mon/9+dd+4+yy/4-yy/100+yy/400)%7;


  if ( lua_isstring( L, 1 ))
      fmt = luaL_checkstring( L, 1 );
  else // No parameter
  {
    strftime( buff, 50, "%c", &t );
    lua_pushstring( L, buff );
    return 1;
  }

  if ( strcmp( fmt, "*t" ) == 0 )
  {
    lua_newtable( L );
    for( i = 0; i < 6; i ++ )
    {
      lua_pushstring( L, mbed_datetime_names[ i ] );
      lua_pushinteger( L, *pvals[ i ] );
      lua_settable( L, -3 );
    }
  }
  else
  {
    strftime( buff, 50, fmt, &t );
    lua_pushstring( L, buff );
  }

  return 1;
}

// Lua: mbed.rtc.setalarmdate( arg )
// arg can be a string formated as dd/mm/yyyy hh:mm:ss
// or a table with 'day', 'month', 'year', 'hour', 'minute' and 'second' fields
static int mbed_rtc_setalarm( lua_State *L )
{
  const char *data;
  int vals[6] = {-1, -1, -1, -1, -1, -1}; 
  unsigned i;
  int sz;
  char error[20] = "invalid "; // Error message
  uint8_t update = 0;          // Each bit flags a reg for update

  // If we receive a string, split the time using sscanf
  if( lua_isstring( L, 1 ) )
  {
    data = luaL_checkstring( L, 1 );

    if( sscanf( data, "%d/%d/%d %d:%d:%d%n", &vals[0], &vals[1], &vals[2], &vals[3], &vals[4], &vals[5], &sz ) != 6 || sz != strlen( data ) )
      return luaL_error( L, "invalid datetime format" );

    update = 0x3F;
  }
  else // If we receive a table, get the values by their name
  {
    // Read all values
    luaL_checktype( L, 1, LUA_TTABLE );
    for( i = 0; i < 6; i ++ )
    {
      lua_getfield( L, 1, mbed_datetime_names[i] );

      if ( lua_type( L, -1 ) == LUA_TNUMBER )
      {
        vals[i] = luaL_checkinteger( L, -1 );
        update |= 1<<i;
      }

      lua_pop( L, 1 );
    }
  }

  // Check all values first ( avoid partial update in case of errors )
  for( i = 0; i < 6; i ++ )
    if ( update & 1<<i )
      if ( vals[i] < mbed_datetime_min[i] || vals[i] > mbed_datetime_max[i] )
      {
        strcat( error, mbed_datetime_names[i] );
        return luaL_error( L, error );
      }

  // Copy the values to the registers
  for( i = 0; i < 6; i ++ )
    if ( update & 1<<i )
    {
      if (i == 2) // Year is the only 16bit val
        *((uint16_t *)mbed_datetime_alregs[i]) = vals[i];
      else
        *((uint8_t *)mbed_datetime_alregs[i]) = vals[i];
    }

  // Set mask ( Ignore DOY and DOW )
  LPC_RTC->AMR = 1<<4 | 1<<5;
  
  return 0;
}

static int mbed_rtc_alarmed( lua_State *L )
{
  lua_pushboolean( L, (LPC_RTC->ILR & 2) >> 1 ); 
  LPC_RTC->ILR |= 2; // Clear alarm flag
  return 1;
}

static int mbed_rtc_strftime( lua_State *L )
{
  char out[51];
  struct tm t;
  t.tm_sec = 1;
  t.tm_min = 2;
  t.tm_hour = 3;
  t.tm_mday = 1;
  t.tm_mon = 1;
  t.tm_year = 2000 - 1900;
  t.tm_wday = 6;
  t.tm_yday = 0;
  t.tm_isdst = 0;

  strftime( out, 50, "%c", &t );

  lua_pushstring( L, out );

  return 1;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h" 
const LUA_REG_TYPE mbed_rtc_map[] =
{
  { LSTRKEY( "set" ),  LFUNCVAL( mbed_rtc_set ) },
  { LSTRKEY( "get" ),  LFUNCVAL( mbed_rtc_get ) },
  { LSTRKEY( "setalarm" ),  LFUNCVAL( mbed_rtc_setalarm ) },
  { LSTRKEY( "alarmed" ),  LFUNCVAL( mbed_rtc_alarmed ) },
  { LSTRKEY( "strftime" ),  LFUNCVAL( mbed_rtc_strftime ) },
  { LNILKEY, LNILVAL }
};

