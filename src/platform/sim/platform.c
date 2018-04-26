// Platform-dependent functions

#include "platform.h"
#include "platform_conf.h"
#include "type.h"
#include "devman.h"
#include "genstd.h"
#include <reent.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "term.h"

// Platform specific includes
#include "hostif.h"

// ****************************************************************************
// Terminal support code

#ifdef BUILD_TERM

static void i386_term_out( u8 data )
{
  hostif_putc( data );
}

static int i386_term_in( int mode )
{
  if( mode == TERM_INPUT_DONT_WAIT )
    return -1;
  else
    return hostif_getch();
}

static int i386_term_translate( int data )
{
  int newdata = data;

  if( data == 0 )
    return KC_UNKNOWN;
  else switch( data )
  {
    case '\n':
      newdata = KC_ENTER;
      break;

    case '\t':
      newdata = KC_TAB;
      break;

    case '\b':
      newdata = KC_BACKSPACE;
      break;

    case 0x1B:
      newdata = KC_ESC;
      break;
  }
  return newdata;
}

#endif // #ifdef BUILD_TERM

// *****************************************************************************
// std functions
static void scr_write( int fd, char c )
{
  fd = fd;
  hostif_putc( c );
}

static int kb_read( timer_data_type to )
{
  int res;

  if( to != STD_INFINITE_TIMEOUT )
    return -1;
  else
  {
    while( ( res = hostif_getch() ) >= TERM_FIRST_KEY );
    return res;
  }
}

// ****************************************************************************
// Platform initialization (low-level and full)

void *memory_start_address = 0;
void *memory_end_address = 0;

void platform_ll_init( void )
{
  // Initialise heap memory region.
  memory_start_address = hostif_getmem( SIM_MEM_SIZE );
  memory_end_address = memory_start_address + SIM_MEM_SIZE;
}

int platform_init()
{
  char memdata[80];
  if( memory_start_address == NULL ) 
  {
    hostif_putstr( "platform_init(): mmap failed\n" );
    return PLATFORM_ERR;
  }

  // Set the std input/output functions
  // Set the send/recv functions                          
  std_set_send_func( scr_write );
  std_set_get_func( kb_read );       

  // Set term functions
#ifdef BUILD_TERM  
  term_init( TERM_LINES, TERM_COLS, i386_term_out, i386_term_in, i386_term_translate );
#endif

  term_clrscr();
  term_gotoxy( 1, 1 );
  // Show memory information
  snprintf( memdata, 80, "RAM size is %u bytes (%uKB)\r\n", (unsigned)SIM_MEM_SIZE, (unsigned)SIM_MEM_SIZE / 1024 );
  hostif_putstr( memdata );

  // All done
  return PLATFORM_OK;
}

// ****************************************************************************
// "Dummy" UART functions

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  return 0;
}

void platform_s_uart_send( unsigned id, u8 data )
{
}

int platform_s_uart_recv( unsigned id, timer_data_type timeout )
{
  return -1;
}

int platform_s_uart_set_flow_control( unsigned id, int type )
{
  return PLATFORM_ERR;
}

// ****************************************************************************
// "Dummy" timer functions

void platform_s_timer_delay( unsigned id, timer_data_type delay_us )
{
}

timer_data_type platform_s_timer_op( unsigned id, int op, timer_data_type data )
{
  return 0;
}

timer_data_type platform_timer_read_sys( void )
{
  return hostif_gettime();
}

// ****************************************************************************
// "Dummy" CPU functions


int platform_cpu_set_global_interrupts( int status )
{
  return 0;
}

int platform_cpu_get_global_interrupts( void )
{
  return 0;
}

