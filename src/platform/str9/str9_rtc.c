// STR9 specific RTC support

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "91x_rtc.h"
#include "91x_vic.h"
#include "auxmods.h"
#include <stdio.h>
#include <string.h>

// ****************************************************************************
// Helpers and local variables

static const char *str9_time_names[] = { "hour", "min", "sec" };
static const char *str9_date_names[] = { "day", "month", "year" };

// ****************************************************************************
// Module functions

// Lua: str9.rtc.settime( arg )
// arg can be a string formated as "hh:mm:ss"
// or a table with 'hour', 'minute' and 'second' fields
static int str9_rtc_settime( lua_State *L )
{
  const char *data;
  int hh = -1, mm = -1, ss = -1;
  int *pvals[] = { &hh, &mm, &ss };
  unsigned i;
  RTC_TIME time;
  int sz;

  if( lua_isstring( L, 1 ) )
  {
    data = luaL_checkstring( L, 1 );      
    if( sscanf( data, "%d:%d:%d%n", &hh, &mm, &ss, &sz ) != 3 || sz != strlen( data ) )
      return luaL_error( L, "invalid time format" );
  }
  else
  {
    luaL_checktype( L, 1, LUA_TTABLE );
    for( i = 0; i < 3; i ++ )
    {
      lua_pushstring( L, str9_time_names[ i ] );
      lua_gettable( L, -2 );
      *pvals[ i ] = luaL_checkinteger( L, -1 );
      lua_pop( L, 1 ); 
    }
  }
  if( hh < 0 || hh >= 24 )
    return luaL_error( L, "invalid hour" );
  if( mm < 0 || mm >= 60 )
    return luaL_error( L, "invalid minute" );
  if( ss < 0 || ss >= 60 )
    return luaL_error( L, "invalid second" );
  time.hours = hh;
  time.minutes = mm;
  time.seconds = ss;
  time.milliseconds = 0;
  RTC_SetTime( time );
  return 0;
}

// Lua: time = str9.rtc.gettime( format )
// format can be '*s' to return the time as a string hh:mm:ss
// or '*t' to return the time as a table with fields 'hour', 'minute' and 'second'
static int str9_rtc_gettime( lua_State *L )
{
  RTC_TIME time;
  const char *fmt = luaL_checkstring( L, 1 );
  char buff[ 9 ];
  unsigned i;
  u8 *pvals[] = { &time.hours, &time.minutes, &time.seconds };

  RTC_GetTime( BINARY, &time );
  if( !strcmp( fmt, "*s" ) )
  {
    sprintf( buff, "%02d:%02d:%02d", time.hours, time.minutes, time.seconds );
    lua_pushstring( L, buff );
  }
  else if( !strcmp( fmt, "*t" ) )
  {
    lua_newtable( L );
    for( i = 0; i < 3; i ++ )
    {
      lua_pushstring( L, str9_time_names[ i ] );
      lua_pushinteger( L, *pvals[ i ] );
      lua_settable( L, -3 );
    }
  }
  else
    return luaL_error( L, "invalid format" );
  return 1;
}

// Lua: str9.rtc.setdate( arg )
// arg can be a string formated as "dd/mm/yyyy"
// or a table with 'day', 'month' and 'year' fields
static int str9_rtc_setdate( lua_State *L )
{
  const char *data;
  int dd = -1, mm = -1, yyyy = -1;
  int *pvals[] = { &dd, &mm, &yyyy };
  unsigned i;
  RTC_DATE date;
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
      lua_pushstring( L, str9_date_names[ i ] );
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
  date.day = dd;
  date.weekday = 0;
  date.month = mm;
  date.year = yyyy % 100;
  date.century = yyyy / 100;
  RTC_SetDate( date );
  return 0;
}

// Lua: time = str9.rtc.getdate( format )
// format can be '*s' to return the date as a string dd/mm/yyyy
// or '*t' to return the date as a table with fields 'day', 'month' and 'year'
static int str9_rtc_getdate( lua_State *L )
{  
  RTC_DATE date;
  const char *fmt = luaL_checkstring( L, 1 );
  char buff[ 11 ];
  unsigned i;
  int mm, dd, yyyy;
  int *pvals[] = { &dd, &mm, &yyyy };

  RTC_GetDate( BINARY, &date );
  dd = date.day;
  mm = date.month;
  yyyy = date.century * 100 + date.year;
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
      lua_pushstring( L, str9_date_names[ i ] );
      lua_pushinteger( L, *pvals[ i ] );
      lua_settable( L, -3 );
    }
  }
  else
    return luaL_error( L, "invalid format" );
  return 1;
}

// Lua: str9.rtc.setalarm( day, hour )
// day is the day number as a number or string
// hour is a string formated as "hh:mm:ss"
static int str9_rtc_setalarm( lua_State *L )
{
  const char *time;
  int hh = -1, mm = -1, ss = -1, dd = -1;
  RTC_ALARM alarm;
  int sz;

  // Get day
  if ( lua_isstring( L, 1 ) )
    sscanf( luaL_checkstring( L, 1 ), "%d", &dd );
  else
    dd = luaL_checkinteger( L, 1 );
	
  // Get time
  time = luaL_checkstring( L, 2 );      
  if( sscanf( time, "%d:%d:%d%n", &hh, &mm, &ss, &sz ) != 3 || sz != strlen( time ) )
      return luaL_error( L, "invalid time format" );

  // Check ranges
  if( dd < 1 || dd > 31 )
    return luaL_error( L, "invalid day" );
  if( hh < 0 || hh >= 24 )
    return luaL_error( L, "invalid hour" );
  if( mm < 0 || mm >= 60 )
    return luaL_error( L, "invalid minute" );
  if( ss < 0 || ss >= 60 )
    return luaL_error( L, "invalid second" );

  // Set alarm
  alarm.day = dd;
  alarm.hours = hh;
  alarm.minutes = mm;
  alarm.seconds = ss;
  RTC_SetAlarm( alarm );

  // Turn ON alarm
  RTC_ClearFlag(RTC_FLAG_Alarm);
  RTC_AlarmCmd(ENABLE); 

  return 0;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h" 
const LUA_REG_TYPE str9_rtc_map[] =
{
  { LSTRKEY( "setalarm" ), LFUNCVAL( str9_rtc_setalarm ) },
  { LSTRKEY( "settime" ), LFUNCVAL( str9_rtc_settime ) },
  { LSTRKEY( "gettime" ), LFUNCVAL( str9_rtc_gettime ) },
  { LSTRKEY( "setdate" ), LFUNCVAL( str9_rtc_setdate ) },
  { LSTRKEY( "getdate" ), LFUNCVAL( str9_rtc_getdate ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_disp( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else
  #error "This module is not supported in LUA_OPTIMIZE_MEMORY == 0 mode"
#endif
}  

