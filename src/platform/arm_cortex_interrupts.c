// Generic functions for ARM CPUs

#include "arm_constants.h"
#include "cortex_constants.h"
#include "type.h"
#include "platform.h"

extern void arm_enable_ints( void );
extern void arm_disable_ints( void );
extern u32 arm_get_int_status( void );

int platform_cpu_set_global_interrupts( int status )
{
  u32 crt_status = arm_get_int_status();

  if( status == PLATFORM_CPU_ENABLE )
    arm_enable_ints();
  else
    arm_disable_ints();
  return ( crt_status & INTERRUPT_MASK_BIT ) == INTERRUPT_ACTIVE;
}

int platform_cpu_get_global_interrupts( void )
{
  return ( arm_get_int_status() & INTERRUPT_MASK_BIT ) == INTERRUPT_ACTIVE;
}

