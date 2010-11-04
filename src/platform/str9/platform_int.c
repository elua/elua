// STR9 interrupt support

// Generic headers
#include "platform.h"
#include "platform_conf.h"
#include "elua_int.h"
#include "common.h"

// Platform-specific headers
#include "91x_vic.h"
#include "91x_wiu.h"

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

// External interrupt handlers
static void exint_irq_handler( int group )
{
  u32 bmask;
  u32 pr = WIU->PR;
  u32 mr = WIU->MR;
  u32 tr = WIU->TR;
  u32 shift = group << 3;
  unsigned i;

  // Check interrupt mask
  if( ( ( pr >> shift ) & 0xFF ) == 0 )
  {
    VIC1->VAR = 0xFF;
    return;
  }

  // Iterate through all the bits in the mask, queueing interrupts as needed
  for( i = 0, bmask = 1 << shift; i < 8; i ++, bmask <<= 1 )
    if( ( pr & bmask ) && ( mr & bmask ) )
    {
      // Enqueue interrupt
      if( tr & bmask )
        elua_int_add( INT_GPIO_POSEDGE, exint_src_to_gpio( shift + i ) );
      else
        elua_int_add( INT_GPIO_NEGEDGE, exint_src_to_gpio( shift + i ) );
      // Then clear it
      WIU->PR  = bmask;
    }

  // Clear interrupt source
  VIC1->VAR = 0xFF;
}

void EXTIT0_IRQHandler()
{
  exint_irq_handler( 0 );
}

void EXTIT1_IRQHandler()
{
  exint_irq_handler( 1 );
}

void EXTIT2_IRQHandler()
{
  exint_irq_handler( 2 );
}

void EXTIT3_IRQHandler()
{
  exint_irq_handler( 3 );
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
// Interrupt initialization

void platform_int_init()
{
  // Initialize VIC
  VIC_DeInit();
  VIC0->DVAR = ( u32 )dummy_int_handler;
  VIC1->DVAR = ( u32 )dummy_int_handler;

  // Enablue WIU
  WIU_DeInit();
  
  // Initialize all external interrupts
  VIC_Config( EXTIT0_ITLine, VIC_IRQ, 1 );
  VIC_Config( EXTIT1_ITLine, VIC_IRQ, 2 );
  VIC_Config( EXTIT2_ITLine, VIC_IRQ, 3 );
  VIC_Config( EXTIT3_ITLine, VIC_IRQ, 4 );
  VIC_ITCmd( EXTIT0_ITLine, ENABLE );
  VIC_ITCmd( EXTIT1_ITLine, ENABLE );
  VIC_ITCmd( EXTIT2_ITLine, ENABLE );
  VIC_ITCmd( EXTIT3_ITLine, ENABLE );

  // Enable interrupt generation on WIU
  WIU->CTRL |= 2; 
}

// ****************************************************************************
// Interrupt table
// Must have a 1-to-1 correspondence with the interrupt enum in platform_conf.h!

elua_int_descriptor elua_int_table[ INT_ELUA_LAST ] = 
{
  { int_gpio_posedge_set_status, int_gpio_posedge_get_status, int_gpio_posedge_get_flag },
  { int_gpio_negedge_set_status, int_gpio_negedge_get_status, int_gpio_negedge_get_flag }
};

