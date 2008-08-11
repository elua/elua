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
#include "lpc288x.h"
#include "target.h"

// *****************************************************************************
// std functions

static void uart_send( int fd, char c )
{
}

static int uart_recv()
{
  return 0;
}

// ****************************************************************************
// Platform initialization

int platform_init()
{
  // Initialize CPU
  lpc288x_init();
  
  // Initialize UART  
  // Set the send/recv functions                          
  std_set_send_func( uart_send );
  std_set_get_func( uart_recv );  
     
  return PLATFORM_OK;
} 

// ****************************************************************************
// PIO functions

int platform_pio_has_port( unsigned port )
{
  return 0;
}

const char* platform_pio_get_prefix( unsigned port )
{
  return NULL;
}

int platform_pio_has_pin( unsigned port, unsigned pin )
{
  return 0;
}

pio_type platform_pio_op( unsigned port, pio_type pinmask, int op )
{
  return 0;
}

// ****************************************************************************
// UART

int platform_uart_exists( unsigned id )
{
  return 0;
}

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  return 0;
}

void platform_uart_send( unsigned id, u8 data )
{
}

int platform_uart_recv( unsigned id, unsigned timer_id, int timeout )
{
  return -1;
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
  return 0;
}

u32 platform_timer_get_diff_us( unsigned id, timer_data_type end, timer_data_type start )
{
  return 0;
}

// ****************************************************************************
// Platform data functions

const char* platform_pd_get_platform_name()
{
  return "LPC288x";
}

const char* platform_pd_get_cpu_name()
{
  return "LPC2888";
}

u32 platform_pd_get_cpu_frequency()
{
  return 0;
}

// ****************************************************************************
// Allocator support

extern char end[];

void* platform_get_first_free_ram()
{
  return ( void* )end;
}

#define SRAM_ORIGIN 0x00400000
#define SRAM_SIZE 0x10000

void* platform_get_last_free_ram()
{
  return ( void* )( SRAM_ORIGIN + SRAM_SIZE - STACK_SIZE_TOTAL );
}
