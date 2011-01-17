// Common implementation: UART functions

#include "common.h"
#include "platform.h"
#include "platform_conf.h"
#include "buf.h"
#include "elua_int.h"
#include "sermux.h"

// ****************************************************************************
// UART functions

#ifdef BUILD_SERMUX
int uart_service_id_in = -1;
int uart_service_id_out = -1;
u8 uart_got_esc = 0;
int uart_last_sent = -1;
// [TODO] add interrupt support for virtual UARTs
#else // #ifdef BUILD_SERMUX
#define SERMUX_PHYS_ID        ( 0xFFFF )
#endif // #ifdef BUILD_SERMUX

// The platform UART functions
int platform_uart_exists( unsigned id )
{
#ifdef BUILD_SERMUX
  return id < NUM_UART || ( id >= SERMUX_SERVICE_ID_FIRST && id < SERMUX_SERVICE_ID_FIRST + SERMUX_NUM_VUART );
#else // #ifdef BUILD_SERMUX
  return id < NUM_UART;
#endif // #ifdef BUILD_SERMUX
}

// Helper function for buffers
static int cmn_recv_helper( unsigned id, s32 timeout )
{
#ifdef BUF_ENABLE_UART
  t_buf_data data;
  
  if( buf_is_enabled( BUF_ID_UART, id ) )
  {
    if( timeout == 0 )
    {
      if ( ( buf_read( BUF_ID_UART, id, &data ) ) == PLATFORM_UNDERFLOW )
        return -1;
    }
    else
    {
      while( ( buf_read( BUF_ID_UART, id, &data ) ) == PLATFORM_UNDERFLOW );
    }
    return ( int )data;
  }
  else
#endif // #ifdef BUF_ENABLE_UART
  return platform_s_uart_recv( id, timeout );
}

int platform_uart_recv( unsigned id, unsigned timer_id, s32 timeout )
{
  timer_data_type tmr_start, tmr_crt;
  int res;
  
  if( timeout == 0 )
    return cmn_recv_helper( id, timeout );
  else if( timeout == PLATFORM_UART_INFINITE_TIMEOUT )
    return cmn_recv_helper( id, timeout );
  else
  {
    // Receive char with the specified timeout
    tmr_start = platform_timer_op( timer_id, PLATFORM_TIMER_OP_START, 0 );
    while( 1 )
    {
      if( ( res = cmn_recv_helper( id, 0 ) ) >= 0 )
        break;
      tmr_crt = platform_timer_op( timer_id, PLATFORM_TIMER_OP_READ, 0 );
      if( platform_timer_get_diff_us( timer_id, tmr_crt, tmr_start ) >= timeout )
        break;
    }
    return res;
  }
}

static void cmn_rx_handler( int usart_id, u8 data )
{
#ifdef BUILD_SERMUX
  if( usart_id == SERMUX_PHYS_ID )
  {
    if( data != SERMUX_ESCAPE_CHAR )
    {
      if( ( data >= SERMUX_SERVICE_ID_FIRST ) && data < ( SERMUX_SERVICE_ID_FIRST + SERMUX_NUM_VUART ) )
        uart_service_id_in = data;
      else if( ( data == SERMUX_FORCE_SID_CHAR ) && ( uart_last_sent != -1 ) )
      {
        // Retransmit service ID and last char
        platform_s_uart_send( SERMUX_PHYS_ID, uart_service_id_out );
        if( uart_last_sent & SERMUX_ESC_MASK )
          platform_s_uart_send( SERMUX_PHYS_ID, SERMUX_ESCAPE_CHAR );
        platform_s_uart_send( SERMUX_PHYS_ID, uart_last_sent & 0xFF );
        uart_last_sent = -1;
      }
      else
      {
        // Check for an escaped char
        if( uart_got_esc )
        {
          data ^= SERMUX_ESCAPE_XOR_MASK;
          uart_got_esc = 0;
        }
        if( uart_service_id_in == -1 ) // request full restransmit if needed
          platform_s_uart_send( SERMUX_PHYS_ID, SERMUX_FORCE_SID_CHAR );
        else
          buf_write( BUF_ID_UART, uart_service_id_in, ( t_buf_data* )&data );
      }
    }
    else
      uart_got_esc = 1;
  }
  else
#endif // #ifdef BUILD_SERMUX
  buf_write( BUF_ID_UART, usart_id, ( t_buf_data* )&data );
}

// Send: version with and without mux
void platform_uart_send( unsigned id, u8 data ) 
{
#ifdef BUILD_SERMUX
  if( id >= SERMUX_SERVICE_ID_FIRST && id < SERMUX_SERVICE_ID_FIRST + SERMUX_NUM_VUART )
  {
    if( id != uart_service_id_out )
      platform_s_uart_send( SERMUX_PHYS_ID, id );
    uart_last_sent = data;
    if( data == SERMUX_ESCAPE_CHAR || data == SERMUX_FORCE_SID_CHAR || ( data >= SERMUX_SERVICE_ID_FIRST && data <= SERMUX_SERVICE_ID_LAST ) )
    {
      platform_s_uart_send( SERMUX_PHYS_ID, SERMUX_ESCAPE_CHAR );
      platform_s_uart_send( SERMUX_PHYS_ID, data ^ SERMUX_ESCAPE_XOR_MASK );
      uart_last_sent = SERMUX_ESC_MASK | ( data ^ SERMUX_ESCAPE_XOR_MASK );
    }
    else
      platform_s_uart_send( SERMUX_PHYS_ID, data );
    uart_service_id_out = id;
  }
  else
#endif // #ifdef BUILD_SERMUX
  platform_s_uart_send( id, data );
}

#ifdef BUF_ENABLE_UART
static elua_int_c_handler prev_uart_rx_handler;

static void cmn_uart_rx_inthandler( elua_int_resnum resnum )
{   
  if( buf_is_enabled( BUF_ID_UART, resnum ) || resnum == SERMUX_PHYS_ID )
    cmn_rx_handler( resnum, platform_s_uart_recv( resnum, 0 ) );
  
  // Chain to previous handler
  if( prev_uart_rx_handler != NULL )
    prev_uart_rx_handler( resnum );  
}
#endif // #ifdef BUF_ENABLE_UART

int platform_uart_set_buffer( unsigned id, unsigned log2size )
{
  if( id == SERMUX_PHYS_ID ) // mere mortals aren't allowed to mess with VUART physical interface buffering
    return PLATFORM_ERR;
#ifdef BUF_ENABLE_UART
  if( log2size == 0 )
  {
    if( id >= SERMUX_SERVICE_ID_FIRST ) // Virtual UARTs need buffers no matter what
      return PLATFORM_ERR; 
    // Disable buffering
    buf_set( BUF_ID_UART, id, BUF_SIZE_NONE, BUF_DSIZE_U8 );
  }  
  else
  {
    // Enable buffering
    if( buf_set( BUF_ID_UART, id, log2size, BUF_DSIZE_U8 ) == PLATFORM_ERR )
      return PLATFORM_ERR;
    if( id >= SERMUX_SERVICE_ID_FIRST ) // No need for aditional setup on virtual UARTs
      return PLATFORM_OK;    
    // Enable UART RX interrupt 
    if( platform_cpu_set_interrupt( INT_UART_RX, id, PLATFORM_CPU_ENABLE ) != PLATFORM_INT_OK )
      return PLATFORM_ERR;
    // Setup our C handler
    if( elua_int_get_c_handler( INT_UART_RX ) != cmn_uart_rx_inthandler )
      prev_uart_rx_handler = elua_int_set_c_handler( INT_UART_RX, cmn_uart_rx_inthandler );
  }
  return PLATFORM_OK;
#else // BUF_ENABLE_UART
  return PLATFORM_ERR;
#endif // BUF_ENABLE_UART
}

#ifdef BUILD_SERMUX
// Setup the serial multiplexer
void cmn_uart_setup_sermux()
{
  // Enable UART RX interrupt 
  if( platform_cpu_set_interrupt( INT_UART_RX, SERMUX_PHYS_ID, PLATFORM_CPU_ENABLE ) == PLATFORM_INT_OK )
  {
    // Setup our C handler
    if( elua_int_get_c_handler( INT_UART_RX ) != cmn_uart_rx_inthandler )
      prev_uart_rx_handler = elua_int_set_c_handler( INT_UART_RX, cmn_uart_rx_inthandler );
  }
  else // We don't have a choice but to get stuck here, as we can't print an error anyway, since the console most likely lives on a virtual UART
    while( 1 );      
}
#endif // #ifdef BUILD_SERMUX

int platform_uart_set_flow_control( unsigned id, int type )
{ 
  if( id >= SERMUX_SERVICE_ID_FIRST )
    return PLATFORM_ERR;
  return platform_s_uart_set_flow_control( id, type );
}

