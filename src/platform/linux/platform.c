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
#include "term.h"

// Platform specific includes
#include "monitor.h"
#include "descriptor_tables.h"
#include "kb.h"
#include "host.h"

// ****************************************************************************
// Terminal support code

#ifdef BUILD_TERM

static void i386_term_out( u8 data )
{
  monitor_put( data );
}

static int i386_term_in( int mode )
{
  if( mode == TERM_INPUT_DONT_WAIT )
    return -1;
  else
    return keyboard_getch();
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
  }
  return newdata;
}

#endif // #ifdef BUILD_TERM

// *****************************************************************************
// std functions
static void scr_write( int fd, char c )
{
  fd = fd;
  monitor_put( c );
}

static int kb_read( s32 to )
{
  int res;

  if( to != STD_INFINITE_TIMEOUT )
    return -1;
  else
  {
    while( ( res = keyboard_getch() ) >= TERM_FIRST_KEY );
    return res;
  }
}

// ****************************************************************************
// Platform initialization (low-level and full)

void *memory_start_address = 0;
void *memory_end_address = 0;

void platform_ll_init()
{
#if 0
  // Initialise all the ISRs and segmentation
  init_descriptor_tables();
#endif
    
  // Initialise the screen (by clearing it)
  monitor_clear();        

	// Initialise heap memory region.
	memory_start_address =
		host_mmap2(0, MEM_LENGTH, (PROT_READ|PROT_WRITE), (MAP_PRIVATE|MAP_ANONYMOUS), -1, 0);
	memory_end_address = memory_start_address + MEM_LENGTH;
}

int platform_init()
{ 
#if 0
  // We can start interrupts now
  asm volatile("sti");    
  
  // And install the keyboard handler
  keyboard_install();
#endif

	if(memory_start_address == MAP_FAILED) {
    monitor_write( "platform_init(): mmap failed\n" );
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
 
  // All done
  return PLATFORM_OK;
}

// ****************************************************************************
// "Dummy" UART functions

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  return 0;
}

void platform_uart_send( unsigned id, u8 data )
{
}

int platform_s_uart_recv( unsigned id, s32 timeout )
{
  return -1;
}

// ****************************************************************************
// "Dummy" timer functions

void platform_s_timer_delay( unsigned id, u32 delay_us )
{
}

u32 platform_s_timer_op( unsigned id, int op, u32 data )
{
  return 0;
}

// ****************************************************************************
// Allocator support

#if 0
u32 platform_get_lastmem()
{
  return lastmem;
}
#endif
