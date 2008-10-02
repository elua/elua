// Platform-dependent functions

#include "platform.h"
#include "type.h"
#include "devman.h"
#include "genstd.h"
#include "stacks.h"
#include <reent.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "utils.h"

// Platform includes
#include "71x_lib.h"

#define CON_UART      1

// *****************************************************************************
// std functions

static void uart_send( int fd, char c )
{
  fd = fd;
  platform_uart_send( CON_UART, c );
}

static int uart_recv()
{
  return platform_uart_recv( CON_UART, 0, PLATFORM_UART_INFINITE_TIMEOUT );
}

// ****************************************************************************
// Platform initialization

// Clock initialization
static void clock_init()
{
  // Configure PCLK1 = RCLK / 2
  RCCU_PCLK1Config( RCCU_RCLK_2 );

  // Configure PCLK2 = RCLK / 2
  RCCU_PCLK2Config( RCCU_RCLK_2 );

  // Configure MCLK clock for the CPU, RCCU_DEFAULT = RCLK /1
  RCCU_MCLKConfig( RCCU_DEFAULT );
   
  // Configure the PLL1 ( * 20 , / 1 )
  RCCU_PLL1Config( RCCU_PLL1_Mul_20, RCCU_Div_1 );

  // Wait PLL to lock
  while(RCCU_FlagStatus( RCCU_PLL1_LOCK) == RESET );

  // Select PLL1_Output as RCLK clock
  RCCU_RCLKSourceConfig( RCCU_PLL1_Output );  
  
  // At this step the MCLK = 40MHz, PCLK1 = 20MHz and PCLK2 = 20MHz with an external oscilator equal to 4MHz  
}

int platform_init()
{
  // Initialize clocks
  clock_init();
  
  // Setup UART1 for operation
  platform_uart_setup( CON_UART, 115200, 8, PLATFORM_UART_PARITY_NONE, PLATFORM_UART_STOPBITS_1 );
  
  // Set the send/recv functions                          
  std_set_send_func( uart_send );
  std_set_get_func( uart_recv );
    
  return PLATFORM_OK;
} 

// ****************************************************************************
// UART

#define NUM_UARTS   4

static const u16 uart_rx_pins[ NUM_UARTS ] = { 0x0001 << 8, 0x0001 << 10, 0x0001 << 13, 0x0001 << 1 };
static const u16 uart_tx_pins[ NUM_UARTS ] = { 0x0001 << 9, 0x0001 << 11, 0x0001 << 14, 0x0001 << 0 };
static const UART_TypeDef *uart_periph[ NUM_UARTS ] = { UART0, UART1, UART2, UART3 };

int platform_uart_exists( unsigned id )
{
  return id < NUM_UARTS;
}

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{      
  UART_TypeDef* pport = ( UART_TypeDef* )uart_periph[ id ];
  UARTParity_TypeDef u_parity;
  UARTStopBits_TypeDef u_stopbits;
  UARTMode_TypeDef u_mode;
  
  // Configure GPIO
  GPIO_Config( GPIO0, uart_tx_pins[ id ], GPIO_AF_PP );
  GPIO_Config( GPIO0, uart_rx_pins[ id ], GPIO_IN_TRI_CMOS );  
  
  UART_Init( pport );
  
  // Enable port
  UART_OnOffConfig( pport, ENABLE ); 
  
  //  Disable FIFOs, reset FIFOs, disable loopback     
  UART_FifoConfig( pport, DISABLE ); 
  UART_FifoReset( pport, UART_RxFIFO ); 
  UART_FifoReset( pport, UART_TxFIFO ); 
  UART_LoopBackConfig( pport, DISABLE );
  
  // Check parity first
  if( parity == PLATFORM_UART_PARITY_EVEN )
    u_parity = UART_EVEN_PARITY;
  else if( parity == PLATFORM_UART_PARITY_ODD )
    u_parity = UART_ODD_PARITY;
  else
    u_parity = UART_NO_PARITY;
    
  // Then stop bits
  if( stopbits == PLATFORM_UART_STOPBITS_1 )
    u_stopbits = UART_1_StopBits;    
  else if( stopbits == PLATFORM_UART_STOPBITS_1 )
    u_stopbits = UART_1_5_StopBits;    
  else
    u_stopbits = UART_2_StopBits;
    
  // And finally databits
  if( databits == 7 && u_parity != UART_NO_PARITY )
    u_mode = UARTM_7D_P;
  else if( databits == 8 && u_parity != UART_NO_PARITY )
    u_mode = UARTM_8D_P;
  else
    u_mode = UARTM_8D;
    
  // Configure port and enable Rx
  UART_Config( pport, baud, u_parity, u_stopbits, u_mode );
  UART_RxConfig( pport, ENABLE );   
  
  // Return actual baud
  // [TODO] COMPUTE actual baud...
  return baud;
}

void platform_uart_send( unsigned id, u8 data )
{
  UART_TypeDef* pport = ( UART_TypeDef* )uart_periph[ id ];
  
  UART_ByteSend( pport, &data );
}

int platform_uart_recv( unsigned id, unsigned timer_id, int timeout )
{
  timer_data_type tmr_start, tmr_crt;
  int res;
  UART_TypeDef* pport = ( UART_TypeDef* )uart_periph[ id ];    
  
  if( timeout == 0 )
  {
    // Return data only if already available
    if( UART_FlagStatus( pport ) & UART_RxBufNotEmpty )
      return UART_ByteReceive( pport );
    else
      return -1;
  }
  else if( timeout == PLATFORM_UART_INFINITE_TIMEOUT )
  {
    // Wait for dataa
    return UART_ByteReceive( pport );
  }
  else
  {
    // Receive char with the specified timeout
    tmr_start = platform_timer_op( timer_id, PLATFORM_TIMER_OP_START,0 );
    while( 1 )
    {
      if( UART_FlagStatus( pport ) & UART_RxBufNotEmpty  )
      {
        res = UART_ByteReceive( pport );
        break;
      }
      else
        res = -1;
      tmr_crt = platform_timer_op( timer_id, PLATFORM_TIMER_OP_READ, 0 );
      if( platform_timer_get_diff_us( timer_id, tmr_crt, tmr_start ) >= timeout )
        break;
    }
    return res;    
  }
}

// ****************************************************************************
// Timer

int platform_timer_exists( unsigned id )
{
  return 0;
}

// Helper: get timer clock
static u32 platform_timer_get_clock( unsigned id )
{
  return 0;
}

// Helper: set timer clock
static u32 platform_timer_set_clock( unsigned id, u32 clock )
{
  return 0;
}

void platform_timer_delay( unsigned id, u32 delay_us )
{
}
      
u32 platform_timer_op( unsigned id, int op, u32 data )
{
  u32 res = 0;

  switch( op )
  {
    case PLATFORM_TIMER_OP_START:
      break;
      
    case PLATFORM_TIMER_OP_READ:
      break;
      
    case PLATFORM_TIMER_OP_GET_MAX_DELAY:
      break;
      
    case PLATFORM_TIMER_OP_GET_MIN_DELAY:
      break;      
      
    case PLATFORM_TIMER_OP_SET_CLOCK:
      break;
      
    case PLATFORM_TIMER_OP_GET_CLOCK:
      break;
  }
  return res;
}

u32 platform_timer_get_diff_us( unsigned id, timer_data_type end, timer_data_type start )
{
  timer_data_type temp;
  u32 freq;
    
  freq = platform_timer_get_clock( id );
  if( start < end )
  {
    temp = end;
    end = start;
    start = temp;
  }
  return ( ( u64 )( start - end ) * 1000000 ) / freq;
}

// ****************************************************************************
// Allocator support

extern char end[];

void* platform_get_first_free_ram( unsigned id )
{
  return id > 0 ? NULL : ( void* )end;
}

#define SRAM_ORIGIN 0x20000000
#define SRAM_SIZE 0x10000

void* platform_get_last_free_ram( unsigned id )
{
  return id > 0 ? NULL : ( void* )( SRAM_ORIGIN + SRAM_SIZE - STACK_SIZE_TOTAL - 1 );
}
