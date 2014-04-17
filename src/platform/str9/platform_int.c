// STR9 interrupt support

// Generic headers
#include "platform.h"
#include "platform_conf.h"
#include "elua_int.h"
#include "common.h"

// Platform-specific headers
#include "91x_vic.h"
#include "91x_wiu.h"
#include "91x_tim.h"

#include <stdio.h>

#ifndef VTMR_TIMER_ID
#define VTMR_TIMER_ID         ( -1 )
#endif

// ****************************************************************************
// Interrupt handlers

// Dummy interrupt handlers avoid spurious interrupts (AN2593)
static void dummy_int_handler()
{
  VIC0->VAR = 0xFF;
  VIC1->VAR = 0xFF;
}  

static const u8 exint_group_to_gpio[] = { 3, 5, 6, 7 };

// Convert an EXINT source number to a GPIO ID
static u16 exint_src_to_gpio( u32 exint )
{
  return PLATFORM_IO_ENCODE( exint_group_to_gpio[ exint >> 3 ], exint & 0x07, 0 ); 
}

// Convert a GPIO ID to a EXINT number
static int exint_gpio_to_src( pio_type piodata )
{
  u16 port = PLATFORM_IO_GET_PORT( piodata );
  u16 pin = PLATFORM_IO_GET_PIN( piodata );
  unsigned i;

  for( i = 0; i < sizeof( exint_group_to_gpio ) / sizeof( u8 ); i ++ )
    if( exint_group_to_gpio[ i ] == port )
      break;
  // Restrictions: only the specified port(s) have interrupt capabilities
  //               for port 0 (GPIO3), only pins 2..7 have interrupt capabilities
  if( ( i == sizeof( exint_group_to_gpio ) / sizeof( u8 ) ) || ( ( i == 0 ) && ( pin < 2 ) ) )
    return PLATFORM_INT_BAD_RESNUM;
  return ( i << 3 ) + pin;
}

// ----------------------------------------------------------------------------
// External interrupt handlers

void WIU_IRQHandler(void)
{
  u32 bmask;
  u32 pr = WIU->PR;
  u32 mr = WIU->MR;
  u32 tr = WIU->TR;
  unsigned i;

  // Iterate through all the bits in the mask, queueing interrupts as needed
  for( i = 2, bmask = 4; i < 32; i ++, bmask <<= 1 )
    if( ( pr & bmask ) && ( mr & bmask ) )
    {
      // Enqueue interrupt
      if( tr & bmask )
        cmn_int_handler( INT_GPIO_POSEDGE, exint_src_to_gpio( i ) );
      else
        cmn_int_handler( INT_GPIO_NEGEDGE, exint_src_to_gpio( i ) );
      // Then clear it
      WIU->PR  = bmask;
    }

  // Clear interrupt source
  VIC1->VAR = 0xFF;
}

// ----------------------------------------------------------------------------
// Timer interrupt handlers

extern TIM_TypeDef* const str9_timer_data[];
extern u8 str9_timer_int_periodic_flag[ NUM_PHYS_TIMER ];

static void tmr_int_handler( unsigned id )
{
  TIM_TypeDef *base = ( TIM_TypeDef* )str9_timer_data[ id ];

  TIM_ClearFlag( base, TIM_FLAG_OC1 );
  TIM_CounterCmd( base, TIM_CLEAR );
  if( id == VTMR_TIMER_ID )
  {
    cmn_virtual_timer_cb();
    cmn_systimer_periodic();
  }
  else
    cmn_int_handler( INT_TMR_MATCH, id );
  if( str9_timer_int_periodic_flag[ id ] != PLATFORM_TIMER_INT_CYCLIC )
    TIM_ITConfig( base, TIM_IT_OC1, DISABLE );    
  VIC0->VAR = 0xFF;
}

void TIM0_IRQHandler(void)
{
  tmr_int_handler( 0 );
}

void TIM1_IRQHandler(void)
{
  tmr_int_handler( 1 );
}

void TIM2_IRQHandler(void)
{
  tmr_int_handler( 2 );
}

void TIM3_IRQHandler(void)
{
  tmr_int_handler( 3 );
}

// ****************************************************************************
// GPIO helper functions

static int gpioh_get_int_status( elua_int_id id, elua_int_resnum resnum )
{
  int temp;
  u32 mask;
  
  if( ( temp = exint_gpio_to_src( resnum ) ) == -1 )
    return PLATFORM_INT_BAD_RESNUM;
  mask = 1 << temp;
  if( WIU->MR & mask )
  {
    if( id == INT_GPIO_POSEDGE )
      return ( WIU->TR & mask ) != 0;
    else
      return ( WIU->TR & mask ) == 0;
  }
  else
    return 0;
}

static int gpioh_set_int_status( elua_int_id id, elua_int_resnum resnum, int status )
{
  int crt_status = gpioh_get_int_status( id, resnum );
  int temp;
  u32 mask;
  
  if( ( temp = exint_gpio_to_src( resnum ) ) == -1 )
    return PLATFORM_INT_BAD_RESNUM;
  mask = 1 << temp;
  if( status == PLATFORM_CPU_ENABLE )
  {
    // Set edge type
    if( id == INT_GPIO_POSEDGE )
      WIU->TR |= mask;
    else
      WIU->TR &= ~mask;
    // Clear interrupt flag?
    WIU->PR = mask;
    // Enable interrupt
    WIU->MR |= mask;
  }     
  else
    WIU->MR &= ~mask; 
  return crt_status;
}

static int gpioh_get_int_flag( elua_int_id id, elua_int_resnum resnum, int clear )
{
  int temp, flag = 0;
  u32 mask;
  u32 pr = WIU->PR;
  u32 tr = WIU->TR;
 
  if( ( temp = exint_gpio_to_src( resnum ) ) == -1 )
    return PLATFORM_INT_BAD_RESNUM;
  mask = 1 << temp;
  if( pr & mask ) 
  {
    if( id == INT_GPIO_POSEDGE )
      flag = ( tr & mask ) != 0;
    else
      flag = ( tr & mask ) == 0;
  }
  if( flag && clear )
    WIU->PR = mask;
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
  return gpioh_get_int_flag( INT_GPIO_POSEDGE, resnum, clear );
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
  return gpioh_get_int_flag( INT_GPIO_NEGEDGE, resnum, clear );
}

// ****************************************************************************
// Interrupt: INT_TMR_MATCH

static int int_tmr_match_get_status( elua_int_resnum resnum )
{
  TIM_TypeDef *base = ( TIM_TypeDef* )str9_timer_data[ resnum ];

  return ( base->CR2 & TIM_IT_OC1 ) != 0;
}

static int int_tmr_match_set_status( elua_int_resnum resnum, int status )
{
  int previous = int_tmr_match_get_status( resnum );
  TIM_TypeDef *base = ( TIM_TypeDef* )str9_timer_data[ resnum ];
  
  TIM_ITConfig( base, TIM_IT_OC1, status == PLATFORM_CPU_ENABLE ? ENABLE : DISABLE );
  return previous;
}

static int int_tmr_match_get_flag( elua_int_resnum resnum, int clear )
{
  TIM_TypeDef *base = ( TIM_TypeDef* )str9_timer_data[ resnum ];
  int status = TIM_GetFlagStatus( base, TIM_FLAG_OC1 );

  if( clear )
    TIM_ClearFlag( base, TIM_FLAG_OC1 );
  return status;
}

// ****************************************************************************
// Interrupt initialization

void platform_int_init()
{
  int p = 1;

  // Initialize VIC
  VIC_DeInit();
  VIC0->DVAR = ( u32 )dummy_int_handler;
  VIC1->DVAR = ( u32 )dummy_int_handler;

  // Enablue WIU
  WIU_DeInit();
  
  // Initialize the WIU interrupt
  VIC_Config( WIU_ITLine, VIC_IRQ, p ++ );
  VIC_ITCmd( WIU_ITLine, ENABLE );
  // Enable interrupt generation on WIU
  WIU->PR = 0xFFFFFFFF;
  WIU->CTRL |= 2; 

#ifdef BUILD_CAN
  /* initialize the interrupt controller */
  VIC_Config(CAN_ITLine, VIC_IRQ, p ++ );
  /* enable global interrupt */
  VIC_ITCmd(CAN_ITLine, ENABLE);
#endif

#ifdef INT_TMR_MATCH
  VIC_Config( TIM0_ITLine, VIC_IRQ, 5 );
  VIC_Config( TIM1_ITLine, VIC_IRQ, 6 );
  VIC_Config( TIM2_ITLine, VIC_IRQ, 7 );
  VIC_Config( TIM3_ITLine, VIC_IRQ, 8 );
  VIC_ITCmd( TIM0_ITLine, ENABLE );
  VIC_ITCmd( TIM1_ITLine, ENABLE );
  VIC_ITCmd( TIM2_ITLine, ENABLE );
  VIC_ITCmd( TIM3_ITLine, ENABLE );
#endif
}

// ****************************************************************************
// Interrupt table
// Must have a 1-to-1 correspondence with the interrupt enum in platform_ints.h!

const elua_int_descriptor elua_int_table[ INT_ELUA_LAST ] = 
{
  { int_gpio_posedge_set_status, int_gpio_posedge_get_status, int_gpio_posedge_get_flag },
  { int_gpio_negedge_set_status, int_gpio_negedge_get_status, int_gpio_negedge_get_flag },
  { int_tmr_match_set_status, int_tmr_match_get_status, int_tmr_match_get_flag }
};

