#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "type.h"
#include "devman.h"
#include "platform.h"
#include "romfs.h"
#include "xmodem.h"
#include "shell.h"
#include "build.h"
#include "lua.h"
#include "term.h"

extern char etext[];

// ****************************************************************************
// XMODEM support code

// Maximum file size that can be received via XMODEM
// Should be a multiple of 128
#define XMODEM_MAX_FILE_SIZE    4096

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
// Terminal support code

#define TERMINAL_LINES        25
#define TERMINAL_COLS         80

#ifdef BUILD_TERM

#define TERM_UART_ID          0
#define TERM_TIMER_ID         0
#define TERM_TIMEOUT          100000

static void term_out( u8 data )
{
  platform_uart_send( TERM_UART_ID, data );
}

static int term_in( int mode )
{
  if( mode == TERM_INPUT_DONT_WAIT )
    return platform_uart_recv( TERM_UART_ID, TERM_TIMER_ID, 0 );
  else
    return platform_uart_recv( TERM_UART_ID, TERM_TIMER_ID, PLATFORM_UART_INFINITE_TIMEOUT );
}

static int term_translate( u8 data )
{
  int c;
  
  if( isprint( data ) )
    return data;
  else if( data == 0x1B ) // escape sequence
  {
    // If we don't get a second char, we got a simple "ESC", so return KC_ESC
    // If we get a second char it must be '[', the next one is relevant for us
    if( platform_uart_recv( TERM_UART_ID, TERM_TIMER_ID, TERM_TIMEOUT ) == -1 )
      return KC_ESC;
    if( ( c = platform_uart_recv( TERM_UART_ID, TERM_TIMER_ID, TERM_TIMEOUT ) ) == -1 )
      return KC_UNKNOWN;
    switch( c )
    {
      case 0x41:
        return KC_UP;
      case 0x42:
        return KC_DOWN;
      case 0x43:
        return KC_RIGHT;
      case 0x44:
        return KC_LEFT;               
    }
  }
  else if( data == 0x0D )
  {
    // CR/LF sequence, read the second char (LF) if applicable
    platform_uart_recv( TERM_UART_ID, TERM_TIMER_ID, TERM_TIMEOUT );
    return KC_ENTER;
  }
  else
  {
    switch( data )
    {
      case 0x09:
        return KC_TAB;
      case 0x16:
        return KC_PAGEDOWN;
      case 0x15:
        return KC_PAGEUP;
      case 0x05:
        return KC_END;
      case 0x01:
        return KC_HOME;
      case 0x7F:
      case 0x08:
        return KC_BACKSPACE;
    }
  }
  return KC_UNKNOWN;
}

#else // #ifdef BUILD_TERM

static void term_out( u8 data )
{
  data = data;
}

static int term_in( int mode )
{
  mode = mode;
  return -1;
}

static int term_translate( u8 data )
{
  data = data;
  return -1;
}

#endif // #ifdef BUILD_TERM

// ****************************************************************************
//  Program entry point

int main( void )
{
  FILE* fp;
  
  // Initialize platform first
  if( platform_init() != PLATFORM_OK )
  {
    // This should never happen
    while( 1 );
  }
  
  // Initialize device manager
  dm_init();
  
  // Register the ROM filesystem
  dm_register( romfs_init() );  
  
  // Initialize XMODEM
  xmodem_init( xmodem_send, xmodem_recv );    
  
  // Initialize terminal
  term_init( TERMINAL_LINES, TERMINAL_COLS, term_out, term_in, term_translate );
  
  printf( ".text ends at %p\n", etext );
  
  // Autorun: if "autorun.lua" is found in the ROM file system, run it first
  if( ( fp = fopen( "/rom/autorun.lua", "r" ) ) != NULL )
  {
    fclose( fp );
    char* lua_argv[] = { "lua", "/rom/autorun.lua", NULL };
    lua_main( 2, lua_argv );    
  }
  
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
