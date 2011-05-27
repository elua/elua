// AVR32 interrupt support

#include "platform_conf.h"
#if defined( BUILD_C_INT_HANDLERS ) || defined( BUILD_LUA_INT_HANDLERS )

// Generic headers
#include "platform.h"
#include "elua_int.h"
#include "common.h"

// Platform includes
#ifdef FORLM3S9B92
  #define TARGET_IS_TEMPEST_RB1

  #include "lm3s9b92.h"
#elif FORLM3S8962
  #include "lm3s8962.h"
#elif FORLM3S6965
  #include "lm3s6965.h"
#elif FORLM3S6918
  #include "lm3s6918.h"
#endif

#include "rom.h"
#include "rom_map.h"
#include "hw_ints.h"
#include "uart.h"
#include "interrupt.h"

// ****************************************************************************
// Interrupt handlers

// ----------------------------------------------------------------------------
// UART_RX interrupt

extern const u32 uart_base[]; 
static const int uart_int_mask = UART_INT_RX | UART_INT_RT;
 
static void uart_common_rx_handler( int resnum )
{
  MAP_UARTIntClear( uart_base[ resnum ], uart_int_mask );
  while( MAP_UARTCharsAvail( uart_base[ resnum ] ) )  
    cmn_int_handler( INT_UART_RX, resnum );  
}

void uart0_handler()
{
  uart_common_rx_handler( 0 );
}

void uart1_handler()
{
  uart_common_rx_handler( 1 );
}

void uart2_handler()
{
  uart_common_rx_handler( 2 );
}

// ****************************************************************************
// Interrupt: INT_UART_RX

static int int_uart_rx_get_status( elua_int_resnum resnum )
{  
  return ( MAP_UARTIntStatus( uart_base[ resnum ], false ) & uart_int_mask ) == uart_int_mask ? 1 : 0;
}

static int int_uart_rx_set_status( elua_int_resnum resnum, int status )
{
  int prev = int_uart_rx_get_status( resnum );
  
  if( status == PLATFORM_CPU_ENABLE )
    MAP_UARTIntEnable( uart_base[ resnum ], uart_int_mask );
  else
    MAP_UARTIntDisable( uart_base[ resnum ], uart_int_mask );  
  return prev;
}

static int int_uart_rx_get_flag( elua_int_resnum resnum, int clear )
{

  int flag = ( UARTIntStatus( uart_base[ resnum ], false ) & uart_int_mask ) == uart_int_mask ? 1 : 0;
  
  if( clear )
    MAP_UARTIntClear( uart_base[ resnum ], uart_int_mask ); 
  return flag;  
}

// ****************************************************************************
// Interrupt initialization

void platform_int_init()
{
  IntEnable( INT_UART0 );
  IntEnable( INT_UART1 );
  IntEnable( INT_UART2 );    
}

// ****************************************************************************
// Interrupt table
// Must have a 1-to-1 correspondence with the interrupt enum in platform_conf.h!

const elua_int_descriptor elua_int_table[ INT_ELUA_LAST ] = 
{
  { int_uart_rx_set_status, int_uart_rx_get_status, int_uart_rx_get_flag }
};

#endif // #if defined( BUILD_C_INT_HANDLERS ) || defined( BUILD_LUA_INT_HANDLERS )
