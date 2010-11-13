// LPC24xx interrupt support

// Generic headers
#include "platform.h"
#include "platform_conf.h"
#include "elua_int.h"
#include "common.h"

// Platform-specific headers
#include "irq.h"
#include "LPC23xx.h"
#include "target.h"

// ****************************************************************************
// Interrupt handlers

static PREG const posedge_status[] = { ( PREG )&IO0_INT_STAT_R, ( PREG )&IO2_INT_STAT_R };
static PREG const negedge_status[] = { ( PREG )&IO0_INT_STAT_F, ( PREG )&IO2_INT_STAT_F };
static PREG const intclr_regs[] = { ( PREG )&IO0_INT_CLR, ( PREG )&IO2_INT_CLR };

#define EINT3_BIT             3

// EINT3 (INT_GPIO) interrupt handler
static void int_handler_eint3()
{
  elua_int_id id = ELUA_INT_INVALID_INTERRUPT;
  pio_code resnum = 0;
  int pidx, pin;
  
  EXTINT |= 1 << EINT3_BIT; // clear interrupt
  // Look for interrupt source
  // In can only be GPIO0/GPIO2, as the EXT interrupts are not (yet) used
  pidx = ( IO_INT_STAT & 1 ) ? 0 : 1;
  if( *posedge_status[ pidx ] )
  {
    id = INT_GPIO_POSEDGE;
    pin = intlog2( *posedge_status[ pidx ] );
  }
  else
  {
    id = INT_GPIO_NEGEDGE;
    pin = intlog2( *negedge_status[ pidx ] );
  }
  resnum = PLATFORM_IO_ENCODE( pidx * 2, pin, PLATFORM_IO_ENC_PIN );   
  *intclr_regs[ pidx ] = 1 << pin;
  
  // Run the interrupt through eLua
  cmn_int_handler( id, resnum );
  VICVectAddr = 0; // ACK interrupt    
}

// ****************************************************************************
// GPIO helper functions

static PREG const posedge_regs[] = { ( PREG )&IO0_INT_EN_R, NULL, ( PREG )&IO2_INT_EN_R };
static PREG const negedge_regs[] = { ( PREG )&IO0_INT_EN_F, NULL, ( PREG )&IO0_INT_EN_F };

static int gpioh_get_int_status( elua_int_id id, elua_int_resnum resnum )
{
  int port, pin;
  
  port = PLATFORM_IO_GET_PORT( resnum ); 
  pin = PLATFORM_IO_GET_PIN( resnum ); 
  if( id == INT_GPIO_POSEDGE )
    return *posedge_regs[ port ] & ( 1 << pin );
  else
    return *negedge_regs[ port ] & ( 1 << pin );        
  return 0;
}

static int gpioh_set_int_status( elua_int_id id, elua_int_resnum resnum, int status )
{
  int crt_status = gpioh_get_int_status( id, resnum );
  int port, pin;
  
  port = PLATFORM_IO_GET_PORT( resnum ); 
  pin = PLATFORM_IO_GET_PIN( resnum ); 
  if( id == INT_GPIO_POSEDGE )
  {
    if( status == PLATFORM_CPU_ENABLE )
      *posedge_regs[ port ] |= 1 << pin;
    else
      *posedge_regs[ port ] &= ~( 1 << pin );       
  }
  else
  {
    if( status == PLATFORM_CPU_ENABLE )
      *negedge_regs[ port ] |= 1 << pin;
    else
      *negedge_regs[ port ] &= ~( 1 << pin );         
  }    
  EXTINT |= 1 << EINT3_BIT;
  return crt_status;
}

static int gpioh_get_flag( elua_int_id id, elua_int_resnum resnum, int clear )
{
  int pidx;
  int flag = 0;
  
  // Look for interrupt source
  // In can only be GPIO0/GPIO2, as the EXT interrupts are not (yet) used
  pidx = ( IO_INT_STAT & 1 ) ? 0 : 1;
  if( id == INT_GPIO_POSEDGE && ( *posedge_status[ pidx ] && ( 1 << resnum ) ) )
    flag = 1;
  else if( id == INT_GPIO_NEGEDGE && ( *negedge_status[ pidx ] && ( 1 << resnum ) ) )
    flag = 1;
  if( flag && clear )
    *intclr_regs[ pidx ] = 1 << resnum;  
  return flag;
}

// ****************************************************************************
// Interrupt: INT_GPIO_POSEDGE

static int int_gpio_posedge_set_status( elua_int_resnum resnum, int status )
{
  return gpioh_set_int_status( INT_GPIO_POSEDGE, resnum, status );
}

static int int_gpio_posedge_get_status( elua_int_resnum resnum )
{
  return gpioh_get_int_status( INT_GPIO_POSEDGE, resnum );
}

static int int_gpio_posedge_get_flag( elua_int_resnum resnum, int clear )
{
  return gpioh_get_flag( INT_GPIO_POSEDGE, resnum, clear );
}

// ****************************************************************************
// Interrupt: INT_GPIO_NEGEDGE

static int int_gpio_negedge_set_status( elua_int_resnum resnum, int status )
{
  return gpioh_set_int_status( INT_GPIO_NEGEDGE, resnum, status );
}

static int int_gpio_negedge_get_status( elua_int_resnum resnum )
{
  return gpioh_get_int_status( INT_GPIO_NEGEDGE, resnum );
}

static int int_gpio_negedge_get_flag( elua_int_resnum resnum, int clear )
{
  return gpioh_get_flag( INT_GPIO_NEGEDGE, resnum, clear );
}

// ****************************************************************************
// Interrupt: INT_TMR_MATCH

static int int_tmr_match_set_status( elua_int_resnum resnum, int status )
{
  return PLATFORM_INT_NOT_HANDLED;
}

static int int_tmr_match_get_status( elua_int_resnum resnum )
{
  return PLATFORM_INT_NOT_HANDLED;
}

static int int_tmr_match_get_flag( elua_int_resnum resnum, int clear )
{
  return PLATFORM_INT_NOT_HANDLED;
}

// ****************************************************************************
// Interrupt initialization

void platform_int_init()
{
  install_irq( EINT3_INT, int_handler_eint3, HIGHEST_PRIORITY - 1 );   
}

// ****************************************************************************
// Interrupt table
// Must have a 1-to-1 correspondence with the interrupt enum in platform_conf.h!

const elua_int_descriptor elua_int_table[ INT_ELUA_LAST ] = 
{
  { int_gpio_posedge_set_status, int_gpio_posedge_get_status, int_gpio_posedge_get_flag },
  { int_gpio_negedge_set_status, int_gpio_negedge_get_status, int_gpio_negedge_get_flag },
  { int_tmr_match_set_status, int_tmr_match_get_status, int_tmr_match_get_flag }
};

