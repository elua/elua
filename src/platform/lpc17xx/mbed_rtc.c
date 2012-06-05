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

// ****************************************************************************
// Helpers and local variables

static const char *mbed_time_names[] = { "hour", "min", "sec" };
static const char *mbed_date_names[] = { "day", "month", "year" };

// ****************************************************************************

// C LIB

// TMP
static void platform_rtc_gettime( int* hour, int* min, int* sec )
{
  *hour = LPC_RTC->HOUR;
  *min = LPC_RTC->MIN;
  *sec = LPC_RTC->SEC;
}

static void platform_rtc_settime( int hour, int min, int sec )
{
  LPC_RTC->HOUR = hour; 
  LPC_RTC->MIN = min;
  LPC_RTC->SEC = sec;
}

static void platform_rtc_getdate( int* day, int* month, int* year )
{
  *year = LPC_RTC->YEAR;
  *month = LPC_RTC->MONTH;
  *day = LPC_RTC->DOM;
}

static void platform_rtc_setdate( int day, int month, int year )
{
  LPC_RTC->YEAR = year;
  LPC_RTC->MONTH = month;
  LPC_RTC->DOM = day;
}

static void platform_rtc_setalarmdate( int day, int month, int year )
{
  LPC_RTC->ALYEAR = year;
  LPC_RTC->ALMON = month;
  LPC_RTC->ALDOM = day;
}

static void platform_rtc_setalarmtime( int hour, int min, int sec )
{
  LPC_RTC->ALHOUR = hour; 
  LPC_RTC->ALMIN = min;
  LPC_RTC->ALSEC = sec;

  // TMP - Enable alarm interrupt
  NVIC_EnableIRQ( RTC_IRQn);

  // Clear interrupt flag
  LPC_RTC->ILR = 2;
}

// ****************************************************************************

// LUA Lib

// Lua: mbed.rtc.settime( arg )
// arg can be a string formated as "hh:mm:ss"
// or a table with 'hour', 'minute' and 'second' fields
static int mbed_rtc_settime( lua_State *L )
{
  const char *data;
  int hh = -1, mm = -1, ss = -1;
  int *pvals[] = { &hh, &mm, &ss };
  unsigned i;
  int sz;

  // If we receive a string, split the time using sscanf
  if( lua_isstring( L, 1 ) )
  {
    data = luaL_checkstring( L, 1 );      
    if( sscanf( data, "%d:%d:%d%n", &hh, &mm, &ss, &sz ) != 3 || sz != strlen( data ) )
      return luaL_error( L, "invalid time format" );
  }
  else // If we receive a table, get the values by their name
  {
    luaL_checktype( L, 1, LUA_TTABLE );
    for( i = 0; i < 3; i ++ )
    {
      lua_pushstring( L, mbed_time_names[ i ] );
      lua_gettable( L, -2 );
      *pvals[ i ] = luaL_checkinteger( L, -1 );
      lua_pop( L, 1 ); 
    }
  }

  // Check if the time is valid
  if( hh < 0 || hh >= 24 )
    return luaL_error( L, "invalid hour" );
  if( mm < 0 || mm >= 60 )
    return luaL_error( L, "invalid minute" );
  if( ss < 0 || ss >= 60 )
    return luaL_error( L, "invalid second" );

  platform_rtc_settime( hh, mm, ss );

  return 0;
}

// Lua: time = mbed.rtc.gettime( format )
// format can be '*s' to return the time as a string hh:mm:ss
// or '*t' to return the time as a table with fields 'hour', 'minute' and 'second'
static int mbed_rtc_gettime( lua_State *L )
{
  int hh = -1, mm = -1, ss = -1;
  int *pvals[] = { &hh, &mm, &ss };
  const char *fmt = luaL_checkstring( L, 1 );
  char buff[ 9 ];
  unsigned i;
  
  platform_rtc_gettime( &hh, &mm, &ss );

  if( !strcmp( fmt, "*s" ) )
  {
    sprintf( buff, "%02d:%02d:%02d", hh, mm, ss );
    lua_pushstring( L, buff );
  }
  else if( !strcmp( fmt, "*t" ) )
  {
    lua_newtable( L );
    for( i = 0; i < 3; i ++ )
    {
      lua_pushstring( L, mbed_time_names[ i ] );
      lua_pushinteger( L, *pvals[ i ] );
      lua_settable( L, -3 );
    }
  }
  else
    return luaL_error( L, "invalid format" );
  return 1;
}

// Lua: mbed.rtc.setdate( arg )
// arg can be a string formated as "dd/mm/yyyy"
// or a table with 'day', 'month' and 'year' fields
static int mbed_rtc_setdate( lua_State *L )
{
  const char *data;
  int dd = -1, mm = -1, yyyy = -1;
  int *pvals[] = { &dd, &mm, &yyyy };
  unsigned i;
  int sz;

  if( lua_isstring( L, 1 ) )
  {
    data = luaL_checkstring( L, 1 );      
    if( sscanf( data, "%d/%d/%d%n", &dd, &mm, &yyyy, &sz ) != 3 || sz != strlen( data ) )
      return luaL_error( L, "invalid time format" );
  }
  else
  {
    luaL_checktype( L, 1, LUA_TTABLE );
    for( i = 0; i < 3; i ++ )
    {
      lua_pushstring( L, mbed_date_names[ i ] );
      lua_gettable( L, -2 );
      *pvals[ i ] = luaL_checkinteger( L, -1 );
      lua_pop( L, 1 ); 
    }
  }
  if( dd < 1 || dd > 31 )
    return luaL_error( L, "invalid day" );
  if( mm < 1 || mm > 12 )
    return luaL_error( L, "invalid month" );
  if( yyyy < 0 || yyyy > 9999 )
    return luaL_error( L, "invalid year" );    

  platform_rtc_setdate( dd, mm, yyyy );

  return 0;
}

// Lua: time = mbed.rtc.getdate( format )
// format can be '*s' to return the date as a string dd/mm/yyyy
// or '*t' to return the date as a table with fields 'day', 'month' and 'year'
static int mbed_rtc_getdate( lua_State *L )
{  
  const char *fmt = luaL_checkstring( L, 1 );
  char buff[ 11 ];
  unsigned i;
  int mm, dd, yyyy;
  int *pvals[] = { &dd, &mm, &yyyy };

  platform_rtc_getdate( &dd, &mm, &yyyy );

  if( !strcmp( fmt, "*s" ) )
  {
    sprintf( buff, "%02d/%02d/%04d", dd, mm, yyyy );
    lua_pushstring( L, buff );
  }
  else if( !strcmp( fmt, "*t" ) )
  {
    lua_newtable( L );
    for( i = 0; i < 3; i ++ )
    {
      lua_pushstring( L, mbed_date_names[ i ] );
      lua_pushinteger( L, *pvals[ i ] );
      lua_settable( L, -3 );
    }
  }
  else
    return luaL_error( L, "invalid format" );
  return 1;
}

// Lua: mbed.rtc.setalarmdate( arg )
// arg can be a string formated as "dd/mm/yyyy"
// or a table with 'day', 'month' and 'year' fields
static int mbed_rtc_setalarmdate( lua_State *L )
{
  const char *data;
  int dd = -1, mm = -1, yyyy = -1;
  int *pvals[] = { &dd, &mm, &yyyy };
  unsigned i;
  int sz;

  if( lua_isstring( L, 1 ) )
  {
    data = luaL_checkstring( L, 1 );      
    if( sscanf( data, "%d/%d/%d%n", &dd, &mm, &yyyy, &sz ) != 3 || sz != strlen( data ) )
      return luaL_error( L, "invalid time format" );
  }
  else
  {
    luaL_checktype( L, 1, LUA_TTABLE );
    for( i = 0; i < 3; i ++ )
    {
      lua_pushstring( L, mbed_date_names[ i ] );
      lua_gettable( L, -2 );
      *pvals[ i ] = luaL_checkinteger( L, -1 );
      lua_pop( L, 1 ); 
    }
  }
  if( dd < 1 || dd > 31 )
    return luaL_error( L, "invalid day" );
  if( mm < 1 || mm > 12 )
    return luaL_error( L, "invalid month" );
  if( yyyy < 0 || yyyy > 9999 )
    return luaL_error( L, "invalid year" );    

  platform_rtc_setalarmdate( dd, mm, yyyy );

  return 0;
}

// Lua: mbed.rtc.setalarmtime( arg )
// arg can be a string formated as "hh:mm:ss"
// or a table with 'hour', 'minute' and 'second' fields
static int mbed_rtc_setalarmtime( lua_State *L )
{
  const char *data;
  int hh = -1, mm = -1, ss = -1;
  int *pvals[] = { &hh, &mm, &ss };
  unsigned i;
  int sz;

  // If we receive a string, split the time using sscanf
  if( lua_isstring( L, 1 ) )
  {
    data = luaL_checkstring( L, 1 );      
    if( sscanf( data, "%d:%d:%d%n", &hh, &mm, &ss, &sz ) != 3 || sz != strlen( data ) )
      return luaL_error( L, "invalid time format" );
  }
  else // If we receive a table, get the values by their name
  {
    luaL_checktype( L, 1, LUA_TTABLE );
    for( i = 0; i < 3; i ++ )
    {
      lua_pushstring( L, mbed_time_names[ i ] );
      lua_gettable( L, -2 );
      *pvals[ i ] = luaL_checkinteger( L, -1 );
      lua_pop( L, 1 ); 
    }
  }

  // Check if the time is valid
  if( hh < 0 || hh >= 24 )
    return luaL_error( L, "invalid hour" );
  if( mm < 0 || mm >= 60 )
    return luaL_error( L, "invalid minute" );
  if( ss < 0 || ss >= 60 )
    return luaL_error( L, "invalid second" );

  platform_rtc_setalarmtime( hh, mm, ss );

  return 0;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h" 
const LUA_REG_TYPE mbed_rtc_map[] =
{
  { LSTRKEY( "settime" ),  LFUNCVAL( mbed_rtc_settime ) },
  { LSTRKEY( "gettime" ),  LFUNCVAL( mbed_rtc_gettime ) },
  { LSTRKEY( "setdate" ),  LFUNCVAL( mbed_rtc_setdate ) },
  { LSTRKEY( "getdate" ),  LFUNCVAL( mbed_rtc_getdate ) },
  { LSTRKEY( "setalarmdate" ),  LFUNCVAL( mbed_rtc_setalarmdate ) },
  { LSTRKEY( "setalarmtime" ),  LFUNCVAL( mbed_rtc_setalarmtime ) },
  { LNILKEY, LNILVAL }
};

