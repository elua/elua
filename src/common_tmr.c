// Common code, timer section
// Also implements virtual timers

#include "platform.h"
#include "platform_conf.h"
#include "type.h"
#include "common.h"
#include "elua_int.h"
#include <stdio.h>

// [TODO] when the new build system is ready, automatically add the
// code below in platform_conf.h
#if defined( BUILD_LUA_INT_HANDLERS ) || defined( BUILD_C_INT_HANDLERS )
#define BUILD_INT_HANDLERS

#ifndef INT_TMR_MATCH
#define INT_TMR_MATCH         ELUA_INT_INVALID_INTERRUPT
#endif

extern const elua_int_descriptor elua_int_table[ INT_ELUA_LAST ];

#endif // #if defined( BUILD_LUA_INT_HANDLERS ) || defined( BUILD_C_INT_HANDLERS )

#ifndef VTMR_NUM_TIMERS
#define VTMR_NUM_TIMERS       0
#endif // #ifndef VTMR_NUM_TIMERS

// ****************************************************************************
// Timers (and vtimers) functions

#if VTMR_NUM_TIMERS > 0

// ============================================================================
// VTMR functions

static volatile u32 vtmr_counters[ VTMR_NUM_TIMERS ];
static volatile s8 vtmr_reset_idx = -1;

#if defined( BUILD_INT_HANDLERS ) && ( INT_TMR_MATCH != ELUA_INT_INVALID_INTERRUPT )
#define CMN_TIMER_INT_SUPPORT
#endif // #if defined( BUILD_INT_HANDLERS ) && ( INT_TMR_MATCH != ELUA_INT_INVALID_INTERRUPT )

#ifdef CMN_TIMER_INT_SUPPORT
static volatile u32 vtmr_period_limit[ VTMR_NUM_TIMERS ];  
static volatile u8 vtmr_int_periodic_flag[ ( VTMR_NUM_TIMERS + 7 ) >> 3 ];
static volatile u8 vtmr_int_enabled[ ( VTMR_NUM_TIMERS + 7 ) >> 3 ];
static volatile u8 vtmr_int_flag[ ( VTMR_NUM_TIMERS + 7 ) >> 3 ];
#endif // #ifdef CMN_TIMER_INT_SUPPORT

// This should be called from the platform's timer interrupt at VTMR_FREQ_HZ
void cmn_virtual_timer_cb()
{
  unsigned i;
#ifdef CMN_TIMER_INT_SUPPORT
  u8 msk;
#endif

  for( i = 0; i < VTMR_NUM_TIMERS; i ++ )
  {
    vtmr_counters[ i ] ++;  
#ifdef CMN_TIMER_INT_SUPPORT
    msk = 1 << ( i & 0x07 );
    if( vtmr_counters[ i ] >= vtmr_period_limit[ i ] )
    {
      vtmr_int_flag[ i >> 3 ] |= msk;
      if( vtmr_int_enabled[ i >> 3 ] & msk )      
        elua_int_add( INT_TMR_MATCH, i + VTMR_FIRST_ID );
      if( vtmr_int_periodic_flag[ i >> 3 ] & msk )
        vtmr_counters[ i ] = 0;
      else
        vtmr_int_enabled[ i >> 3 ] &= ( u8 )~msk;    
    }
#endif // #ifdef CMN_TIMER_INT_SUPPORT
  }    
  if( vtmr_reset_idx != -1 )
  {
    vtmr_counters[ vtmr_reset_idx ] = 0;
    vtmr_reset_idx = -1;
  }
}

static void vtmr_reset_timer( unsigned vid )
{
  unsigned id = VTMR_GET_ID( vid );

  vtmr_reset_idx = ( s8 )id;
  while( vtmr_reset_idx != -1 );  
}

static void vtmr_delay( unsigned vid, u32 delay_us )
{
  timer_data_type final;
  unsigned id = VTMR_GET_ID( vid );
  
  final = ( ( u64 )delay_us * VTMR_FREQ_HZ ) / 1000000;
  vtmr_reset_timer( vid );
  while( vtmr_counters[ id ] < final );  
}

#ifdef CMN_TIMER_INT_SUPPORT
static int vtmr_set_match_int( unsigned vid, u32 period_us, int type )
{
  timer_data_type final;
  unsigned id = VTMR_GET_ID( vid );
  u8 msk = 1 << ( id & 0x07 );

  if( period_us == 0 )
  {
    vtmr_int_enabled[ id >> 3 ] &= ( u8 )~msk;
    vtmr_int_flag[ id >> 3 ] &= ( u8 )~msk;
    return PLATFORM_TIMER_INT_OK;
  }
  if( ( final = ( ( u64 )period_us * VTMR_FREQ_HZ ) / 1000000 ) == 0 )
    return PLATFORM_TIMER_INT_TOO_SHORT;
  vtmr_period_limit[ id ] = final;
  if( type == PLATFORM_TIMER_INT_ONESHOT )
    vtmr_int_periodic_flag[ id >> 3 ] &= ( u8 )~msk;
  else
    vtmr_int_periodic_flag[ id >> 3 ] |= msk;
  vtmr_int_flag[ id >> 3 ] &= ( u8 )~msk;
  vtmr_reset_timer( vid ); 
  vtmr_int_enabled[ id >> 3 ] |= msk;
  return PLATFORM_TIMER_INT_OK;
}

static int vtmr_int_get_flag( elua_int_resnum resnum, int clear )
{
  unsigned id = VTMR_GET_ID( resnum );
  u8 msk = 1 << ( id & 0x07 );
  int status = ( vtmr_int_flag[ id >> 3 ] & msk ) != 0;

  if( clear )
    vtmr_int_flag[ id >> 3 ] &= ( u8 )~msk;
  return status;
}

static int vtmr_int_set_status( elua_int_resnum resnum, int status )
{
  unsigned id = VTMR_GET_ID( resnum );
  u8 msk = 1 << ( id & 0x07 );
  int prev = ( vtmr_int_enabled[ id >> 3 ] & msk ) != 0;

  if( status == PLATFORM_CPU_ENABLE )
    vtmr_int_enabled[ id >> 3 ] |= msk;
  else
    vtmr_int_enabled[ id >> 3 ] &= ( u8 )~msk;
  return prev;
}

static int vtmr_int_get_status( elua_int_resnum resnum )
{
  unsigned id = VTMR_GET_ID( resnum );
  u8 msk = 1 << ( id & 0x07 );
  return ( vtmr_int_enabled[ id >> 3 ] & msk ) != 0;
}
#endif // #ifdef CMN_TIMER_INT_SUPPORT 

#else // #if VTMR_NUM_TIMERS > 0

void cmn_virtual_timer_cb()
{
}

#endif // #if VTMR_NUM_TIMERS > 0

// ============================================================================
// Actual timer functions

int platform_timer_exists( unsigned id )
{
#if VTMR_NUM_TIMERS > 0
  if( id >= VTMR_FIRST_ID )
    return TIMER_IS_VIRTUAL( id );
  else
#endif
    return id < NUM_TIMER;
}

void platform_timer_delay( unsigned id, u32 delay_us )
{
#if VTMR_NUM_TIMERS > 0
  if( TIMER_IS_VIRTUAL( id ) )
    vtmr_delay( id, delay_us );
  else
#endif
    platform_s_timer_delay( id, delay_us );
}
      
u32 platform_timer_op( unsigned id, int op, u32 data )
{
  u32 res = 0;

  if( ( VTMR_NUM_TIMERS == 0 ) || ( !TIMER_IS_VIRTUAL( id ) ) )
    return platform_s_timer_op( id, op, data );
#if VTMR_NUM_TIMERS > 0
  switch( op )
  {
    case PLATFORM_TIMER_OP_START:
      vtmr_reset_timer( id );
      res = 0;
      break;
      
    case PLATFORM_TIMER_OP_READ:
      res = vtmr_counters[ VTMR_GET_ID( id ) ];
      break;
      
    case PLATFORM_TIMER_OP_GET_MAX_DELAY:
      res = platform_timer_get_diff_us( id, 0, 0xFFFFFFFF );
      break;
      
    case PLATFORM_TIMER_OP_GET_MIN_DELAY:
      res = platform_timer_get_diff_us( id, 0, 1 );
      break;
      
    case PLATFORM_TIMER_OP_SET_CLOCK:
    case PLATFORM_TIMER_OP_GET_CLOCK:
      res = VTMR_FREQ_HZ;
      break;      
  }
#endif
  return res;
}

u32 platform_timer_get_diff_us( unsigned id, timer_data_type end, timer_data_type start )
{
  timer_data_type temp;
  u32 freq, res;
    
  freq = platform_timer_op( id, PLATFORM_TIMER_OP_GET_CLOCK, 0 );
  if( start < end )
  {
    temp = end;
    end = start;
    start = temp;
  }

  res = ( ( u64 )( start - end ) * 1000000 ) / freq;

  // The result always ends up being given to lua_pushinteger() which turns
  // 0x80000000-0xFFFFFFFF into negative numbers, so max out at 2^31-1 to
  // avoid getting negative results from tmr.getmaxdelay(tmr.VIRT0) and
  // tmr.gettimediff(N, small, large).
#define MAX_U32 ( ~(u32)0 >> 1 )
  if ( res > MAX_U32 ) res = MAX_U32;

  return res;
}

#ifdef BUILD_INT_HANDLERS
int platform_timer_set_match_int( unsigned id, u32 period_us, int type )
{
#if VTMR_NUM_TIMERS > 0 && defined( CMN_TIMER_INT_SUPPORT )
  if( TIMER_IS_VIRTUAL( id ) )
    return vtmr_set_match_int( id, period_us, type );
  else
#endif
    return platform_s_timer_set_match_int( id, period_us, type );
}

int cmn_tmr_int_set_status( elua_int_resnum resnum, int status )
{
#if VTMR_NUM_TIMERS > 00 && defined( CMN_TIMER_INT_SUPPORT )
  if( TIMER_IS_VIRTUAL( resnum ) )
    return vtmr_int_set_status( resnum, status );
#endif
  elua_int_p_set_status ps;
  if( ( ps = elua_int_table[ INT_TMR_MATCH - ELUA_INT_FIRST_ID ].int_set_status ) == NULL )
    return PLATFORM_INT_NOT_HANDLED;
  return ps( resnum, status );
}

int cmn_tmr_int_get_status( elua_int_resnum resnum )
{
#if VTMR_NUM_TIMERS > 00 && defined( CMN_TIMER_INT_SUPPORT )
  if( TIMER_IS_VIRTUAL( resnum ) )
    return vtmr_int_get_status( resnum );
#endif
  elua_int_p_get_status pg;
  if( ( pg = elua_int_table[ INT_TMR_MATCH - ELUA_INT_FIRST_ID ].int_get_status ) == NULL )
    return PLATFORM_INT_NOT_HANDLED;
  return pg( resnum );
}

int cmn_tmr_int_get_flag( elua_int_resnum resnum, int clear )
{
#if VTMR_NUM_TIMERS > 00 && defined( CMN_TIMER_INT_SUPPORT )
  if( TIMER_IS_VIRTUAL( resnum ) )
    return vtmr_int_get_flag( resnum, clear );
#endif
  elua_int_p_get_flag pf;
  if( ( pf = elua_int_table[ INT_TMR_MATCH - ELUA_INT_FIRST_ID ].int_get_flag ) == NULL )
    return PLATFORM_INT_NOT_HANDLED;
  return pf( resnum, clear );
}

#else // #ifdef BUILD_INT_HANDLERS

int platform_timer_set_match_int( unsigned id, u32 period_us, int type )
{
  fprintf( stderr, "Timer match interrupt not available when eLua interrupt support is not enabled.\n" );
  return 0;
}

#endif // #ifdef BUILD_INT_HANDLERS

