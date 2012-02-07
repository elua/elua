// Common code, timer section
// Also implements virtual timers

#include "platform.h"
#include "platform_conf.h"
#include "type.h"
#include "common.h"
#include "elua_int.h"
#include "utils.h"
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

#ifndef PLATFORM_HAS_SYSTIMER
#warning This platform does not have a system timer. Your eLua image might not work as expected.
#define SYSTIMER_SUPPORT 0
#else // #ifndef PLATFORM_HAS_SYSTIMER
#define SYSTIMER_SUPPORT 1
#endif // #ifndef PLATFORM_HAS_SYSTIMER

// ****************************************************************************
// Timers (and vtimers) functions

#if VTMR_NUM_TIMERS > 0

#define VTMR_MAX_PERIOD       ( ( 1LL << 32 ) - 1 )

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

static void vtmr_delay( unsigned vid, timer_data_type delay_us )
{
  timer_data_type final;
  unsigned id = VTMR_GET_ID( vid );
  
  if( delay_us > VTMR_MAX_PERIOD )
    return;
  final = ( ( u64 )delay_us * VTMR_FREQ_HZ ) / 1000000;
  vtmr_reset_timer( vid );
  while( vtmr_counters[ id ] < final );  
}

#ifdef CMN_TIMER_INT_SUPPORT

static int vtmr_set_match_int( unsigned vid, timer_data_type period_us, int type )
{
  timer_data_type final;
  unsigned id = VTMR_GET_ID( vid );
  u8 msk = 1 << ( id & 0x07 );

  if( period_us > VTMR_MAX_PERIOD )
    return PLATFORM_TIMER_INT_TOO_LONG;
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

int platform_timer_sys_available()
{
  return SYSTIMER_SUPPORT;
}

#ifndef PLATFORM_HAS_SYSTIMER
timer_data_type platform_timer_read_sys()
{
  return 0;
}
#endif

int platform_timer_exists( unsigned id )
{
#if VTMR_NUM_TIMERS > 0
  if( id >= VTMR_FIRST_ID )
    return TIMER_IS_VIRTUAL( id );
  else
#endif
    return id < NUM_TIMER || ( id == PLATFORM_TIMER_SYS_ID && SYSTIMER_SUPPORT );
}

void platform_timer_delay( unsigned id, timer_data_type delay_us )
{
#if VTMR_NUM_TIMERS > 0
  if( TIMER_IS_VIRTUAL( id ) )
    vtmr_delay( id, delay_us );
  else
#endif
  if( id == PLATFORM_TIMER_SYS_ID )
  {
    if( !SYSTIMER_SUPPORT )
      return;
    if( delay_us > 0 )
    {
      u64 tstart = platform_timer_read_sys(), tend;
      while( 1 )
      {
        if( ( tend = platform_timer_read_sys() ) < tstart ) // overflow
          tend += ( u64 )PLATFORM_TIMER_SYS_MAX + 1;
        if( tend - tstart >= delay_us )
          return;
      }
    }
  }
  else
    platform_s_timer_delay( id, delay_us );
}
      
timer_data_type platform_timer_op( unsigned id, int op, timer_data_type data )
{
  timer_data_type res = 0;

  if( id == PLATFORM_TIMER_SYS_ID ) // the system timer gets special treatment
  {
    if( !SYSTIMER_SUPPORT )
      return 0;
    switch( op )
    {
      case PLATFORM_TIMER_OP_START:
      case PLATFORM_TIMER_OP_READ:
        res = platform_timer_read_sys();
        break;

      case PLATFORM_TIMER_OP_SET_CLOCK:
      case PLATFORM_TIMER_OP_GET_CLOCK:
        res = PLATFORM_TIMER_SYS_FREQ;
        break;

      case PLATFORM_TIMER_OP_GET_MAX_DELAY:
        res = PLATFORM_TIMER_SYS_MAX + 1;
        break;

      case PLATFORM_TIMER_OP_GET_MAX_CNT:
        res = PLATFORM_TIMER_SYS_MAX;
        break;

      case PLATFORM_TIMER_OP_GET_MIN_DELAY:
        res = 1;
        break;
    }

    return res;
  }
  if( ( VTMR_NUM_TIMERS == 0 ) || ( !TIMER_IS_VIRTUAL( id ) ) )
  {
    // 'get min delay' and 'get max delay' are very common cases, handle them here
    if( op == PLATFORM_TIMER_OP_GET_MAX_DELAY )
      return platform_timer_get_diff_us( id, 0, platform_timer_get_max_cnt( id ) );
    else if( op == PLATFORM_TIMER_OP_GET_MIN_DELAY )
      return platform_timer_get_diff_us( id, 0, 1 );
    else
      return platform_s_timer_op( id, op, data );
  }
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

    case PLATFORM_TIMER_OP_GET_MAX_CNT:
      res = VTMR_MAX_PERIOD;
      break;
      
    case PLATFORM_TIMER_OP_SET_CLOCK:
    case PLATFORM_TIMER_OP_GET_CLOCK:
      res = VTMR_FREQ_HZ;
      break;      
  }
#endif
  return res;
}

timer_data_type platform_timer_get_diff_us( unsigned id, timer_data_type start, timer_data_type end )
{
  u32 freq;
  u64 tstart = ( u64 )start, tend = ( u64 )end;
    
  freq = platform_timer_op( id, PLATFORM_TIMER_OP_GET_CLOCK, 0 );
  if( tstart > tend )
    tend += platform_timer_op( id, PLATFORM_TIMER_OP_GET_MAX_CNT, 0 ) + 1;
  tstart = ( ( tend - tstart ) * 1000000 ) / freq;
  return UMIN( tstart, PLATFORM_TIMER_SYS_MAX );
}

#ifdef BUILD_INT_HANDLERS
int platform_timer_set_match_int( unsigned id, timer_data_type period_us, int type )
{
#if VTMR_NUM_TIMERS > 0 && defined( CMN_TIMER_INT_SUPPORT )
  if( TIMER_IS_VIRTUAL( id ) )
    return vtmr_set_match_int( id, period_us, type );
  else
#endif
  if( id == PLATFORM_TIMER_SYS_ID )
    return PLATFORM_TIMER_INT_INVALID_ID;
  else
    return platform_s_timer_set_match_int( id, period_us, type );
}

int cmn_tmr_int_set_status( elua_int_resnum resnum, int status )
{
  elua_int_p_set_status ps;
#if VTMR_NUM_TIMERS > 00 && defined( CMN_TIMER_INT_SUPPORT )
  if( TIMER_IS_VIRTUAL( resnum ) )
    return vtmr_int_set_status( resnum, status );
#endif
  if( resnum == PLATFORM_TIMER_SYS_ID )
    return PLATFORM_INT_BAD_RESNUM;
  if( ( ps = elua_int_table[ INT_TMR_MATCH - ELUA_INT_FIRST_ID ].int_set_status ) == NULL )
    return PLATFORM_INT_NOT_HANDLED;
  return ps( resnum, status );
}

int cmn_tmr_int_get_status( elua_int_resnum resnum )
{
  elua_int_p_get_status pg;
#if VTMR_NUM_TIMERS > 00 && defined( CMN_TIMER_INT_SUPPORT )
  if( TIMER_IS_VIRTUAL( resnum ) )
    return vtmr_int_get_status( resnum );
#endif
  if( resnum == PLATFORM_TIMER_SYS_ID )
    return PLATFORM_INT_BAD_RESNUM;
  if( ( pg = elua_int_table[ INT_TMR_MATCH - ELUA_INT_FIRST_ID ].int_get_status ) == NULL )
    return PLATFORM_INT_NOT_HANDLED;
  return pg( resnum );
}

int cmn_tmr_int_get_flag( elua_int_resnum resnum, int clear )
{
  elua_int_p_get_flag pf;
#if VTMR_NUM_TIMERS > 00 && defined( CMN_TIMER_INT_SUPPORT )
  if( TIMER_IS_VIRTUAL( resnum ) )
    return vtmr_int_get_flag( resnum, clear );
#endif
  if( resnum == PLATFORM_TIMER_SYS_ID )
    return PLATFORM_INT_BAD_RESNUM;
  if( ( pf = elua_int_table[ INT_TMR_MATCH - ELUA_INT_FIRST_ID ].int_get_flag ) == NULL )
    return PLATFORM_INT_NOT_HANDLED;
  return pf( resnum, clear );
}

#else // #ifdef BUILD_INT_HANDLERS

int platform_timer_set_match_int( unsigned id, timer_data_type period_us, int type )
{
  fprintf( stderr, "Timer match interrupt not available when eLua interrupt support is not enabled.\n" );
  return 0;
}

#endif // #ifdef BUILD_INT_HANDLERS

// ****************************************************************************
// Generic system timer support

static u32 cmn_systimer_ticks_for_us;
static volatile u64 cmn_systimer_counter;
static u32 cmn_systimer_us_per_interrupt;

void cmn_systimer_set_base_freq( u32 freq_hz )
{
  cmn_systimer_ticks_for_us = freq_hz / 1000000;
}

void cmn_systimer_set_interrupt_freq( u32 freq_hz )
{
  cmn_systimer_us_per_interrupt = 1000000 / freq_hz;
}

void cmn_systimer_set_interrupt_period_us( u32 period )
{
  cmn_systimer_us_per_interrupt = period;
}

void cmn_systimer_periodic()
{
  cmn_systimer_counter += cmn_systimer_us_per_interrupt;
}

timer_data_type cmn_systimer_get()
{
  u64 tempsys, tempcnt, crtsys;

  tempcnt = platform_timer_sys_raw_read();
  tempsys = cmn_systimer_counter;
  while( ( crtsys = cmn_systimer_counter ) != tempsys )
  {
    tempcnt = platform_timer_sys_raw_read();
    tempsys = crtsys;
  }
  crtsys += tempcnt / cmn_systimer_ticks_for_us;
  if( crtsys > PLATFORM_TIMER_SYS_MAX ) // timer overflow
  {
    crtsys %= PLATFORM_TIMER_SYS_MAX;
    platform_timer_sys_disable_int();
    cmn_systimer_counter = 0;
    platform_timer_sys_enable_int();
  }
  return ( timer_data_type )crtsys;
}

