// Common platform functions

#ifndef __COMMON_H__
#define __COMMON_H__

#include "elua_int.h"
#include "lua.h"
#include "platform.h"

// Virtual timers data
// VTMR_FIRST_ID must be LARGER than PLATFORM_TIMER_SYS_ID (as declared in platform.h)
#define VTMR_FIRST_ID           ( 0x200 )
#define VTMR_GET_ID( x )        ( ( x ) - VTMR_FIRST_ID )
#define TIMER_IS_VIRTUAL( x )   ( ( VTMR_NUM_TIMERS > 0 ) && ( ( x ) >= VTMR_FIRST_ID ) && ( ( x ) < VTMR_NUM_TIMERS + VTMR_FIRST_ID ) )

// Functions exported by the common platform layer
void cmn_platform_init();
void cmn_virtual_timer_cb();
void cmn_int_handler( elua_int_id id, elua_int_resnum resnum );
// Timer-specific functions
int cmn_tmr_int_set_status( elua_int_resnum resnum, int status );
int cmn_tmr_int_get_status( elua_int_resnum resnum );
int cmn_tmr_int_get_flag( elua_int_resnum resnum, int clear );
// System timer generic implemenation
void cmn_systimer_set_base_freq( u32 freq_hz );
void cmn_systimer_set_interrupt_freq( u32 freq_hz );
void cmn_systimer_set_interrupt_period_us( u32 period );
void cmn_systimer_periodic();
timer_data_type cmn_systimer_get();

void cmn_uart_setup_sermux();

unsigned int intlog2( unsigned int v );
const char* cmn_str64( u64 x );
void cmn_get_timeout_data( lua_State *L, int pidx, unsigned *pid, timer_data_type *ptimeout );

#endif // #ifndef __COMMON_H__

