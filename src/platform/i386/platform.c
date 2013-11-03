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

static int kb_read( timer_data_type to )
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
static u32 lastmem;
static memory_map_t* mmap;
extern void monitor_write_hex(u32int n);
u32 platform_ll_init( struct multiboot_info *mboot_ptr )
{
  // Initialise all the ISRs and segmentation
  init_descriptor_tables();
    
  // Initialise the screen (by clearing it)
  monitor_clear();        
  
  // Look in the GRUB memory map for the space starting at 1M and locate
  // the stack at the end of this space
  mmap = ( memory_map_t* )mboot_ptr->mmap_addr;
  while( (unsigned long)mmap < mboot_ptr->mmap_addr + mboot_ptr->mmap_length) {
    if(mmap->base_addr_low == 0x00100000)
    {
      if(mmap->length_high)
        lastmem = 0x00100000 + 0xFFFFFFF0;
      else
        lastmem = 0x00100000 + ( ( mmap->length_low >> 4 ) << 4 );
      break;
    }
    mmap = (memory_map_t*) ( (unsigned int)mmap + mmap->size + sizeof(unsigned int) );
  }    
  if( lastmem == 0 )
  {
    monitor_write( "Unable to set stack, system halted!\n" );
    while( 1 );
  }  
  
  return lastmem;
}

int platform_init()
{ 
  // We can start interrupts now
  asm volatile("sti");    
  
  // And install the keyboard handler
  keyboard_install();
  
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

// ****************************************************************************
// Allocator support

u32 platform_get_lastmem( void )
{
  return lastmem;
}
