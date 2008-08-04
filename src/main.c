#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type.h"
#include "devman.h"
#include "platform.h"
#include "romfs.h"
#include "xmodem.h"
#include "shell.h"
#include "build.h"
#include "lua.h"

extern char etext[];
extern char end[];

// ****************************************************************************
// XMODEM support code

// Maximum file size that can be received via XMODEM
// Should be a multiple of 128
#define XMODEM_MAX_FILE_SIZE      1024

#ifdef BUILD_XMODEM

#define XMODEM_UART_ID          0
#define XMODEM_TIMER_ID         0

static void xmodem_send( u8 data )
{
  platform_uart_send( XMODEM_UART_ID, data );
}

static int xmodem_recv( u32 timeout )
{
  return platform_uart_recv( XMODEM_UART_ID, XMODEM_TIMER_ID, timeout );
}

#else // #ifdef BUILD_XMODEM

static void xmodem_send( u8 data )
{
  data = data;
}

static int xmodem_recv( u32 timeout )
{
  timeout = timeout;
  return -1;
}

#endif // #ifdef BUILD_XMODEM

// ****************************************************************************
//  Program entry point

int main( void )
{
  // Initialize platform first
  if( platform_init() != PLATFORM_OK )
  {
    // This should never happen
    while( 1 );
  }
    
  // Initialize device manager
  dm_init();
  
  // And register the ROM filesystem
  dm_register( romfs_init() );  
  
  // Initialize XMODEM
  xmodem_init( xmodem_send, xmodem_recv );    
  
  printf( ".text ends at %p, first free RAM is at %p, last free ram is at %p\r\n", etext, platform_get_first_free_ram(), platform_get_last_free_ram() );
    
  // Run the shell
  if( shell_init( XMODEM_MAX_FILE_SIZE ) == 0 )
  {
    printf( "Unable to initialize the eLua shell!\n" );
    // Start Lua directly
    char* lua_argv[] = { "lua", NULL };
    lua_main( 1, lua_argv );
  }
  else
    shell_start();

  while( 1 );
}
