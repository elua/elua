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
#include "term.h"

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
// Terminal support code

#define TERMINAL_LINES        25
#define TERMINAL_COLS         80

#ifdef BUILD_TERMINAL

#define TERM_UART_ID          0
#define TERM_TIMER_ID         0

typedef void ( *p_term_out )( u8 );
// Terminal input function
typedef int ( *p_term_in )( int );
// Terminal translate input function
typedef int ( *p_term_translate )( u8 );

static void term_out( u8 data )
{
  platform_uart_send( TERM_UART_ID, data );
}

static int term_in( int mode )
{
  if( mode == TERM_INPUT_WAIT )
    return platform_uart_recv( TERM_UART_ID, TERM_TIMER_ID, 0 );
  else
    return platform_uart_recv( TERM_UART_ID, TERM_TIMER_ID, PLATFORM_UART_INFINITE_TIMEOUT );
}

static int term_translate( u8 data )
{
  int c;
  
  if( data == 0x1B ) // escape sequence
  {
    // The second character is '[', the next one is relevant for us
    if( term_in( TERM_INPUT_DONT_WAIT ) == -1 )
      return -1;
    if( ( c = term_in( TERM_INPUT_DONT_WAIT ) ) == -1 )
      return -1;
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
    // CR/LF sequence, read the second char
    if( term_in( TERM_INPUT_DONT_WAIT ) == -1 )
      return -1;
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

#else // #ifdef BUILD_TERMINAL

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

#endif // #ifdef BUILD_TERMINAL

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
  
  // Register the ROM filesystem
  dm_register( romfs_init() );  
  
  // Initialize XMODEM
  xmodem_init( xmodem_send, xmodem_recv );    
  
  // Initialize terminal
  term_init( TERMINAL_LINES, TERMINAL_COLS, term_out, term_in, term_translate );
  
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
