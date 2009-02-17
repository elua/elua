#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "type.h"
#include "devman.h"
#include "platform.h"
#include "xmodem.h"
#include "shell.h"
#include "lua.h"
#include "term.h"
#include "platform_conf.h"

// Validate eLua configuratin options
#include "validate.h"

#include "mmcfs.h"
#include "romfs.h"
#if defined(BUILD_MMCFS)
  #define FS_AUTORUN          "/mmc/autorun.lua"
#endif
#if defined(BUILD_ROMFS) && !defined(FS_AUTORUN)
  #define FS_AUTORUN          "/rom/autorun.lua"
#endif

extern char etext[];

// ****************************************************************************
// XMODEM support code

#ifdef BUILD_XMODEM

static void xmodem_send( u8 data )
{
  platform_uart_send( CON_UART_ID, data );
}

static int xmodem_recv( u32 timeout )
{
  return platform_uart_recv( CON_UART_ID, XMODEM_TIMER_ID, timeout );
}

#endif // #ifdef BUILD_XMODEM

// ****************************************************************************
// Terminal support code

#ifdef BUILD_TERM

static void term_out( u8 data )
{
  platform_uart_send( CON_UART_ID, data );
}

static int term_in( int mode )
{
  if( mode == TERM_INPUT_DONT_WAIT )
    return platform_uart_recv( CON_UART_ID, TERM_TIMER_ID, 0 );
  else
    return platform_uart_recv( CON_UART_ID, TERM_TIMER_ID, PLATFORM_UART_INFINITE_TIMEOUT );
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
    if( platform_uart_recv( CON_UART_ID, TERM_TIMER_ID, TERM_TIMEOUT ) == -1 )
      return KC_ESC;
    if( ( c = platform_uart_recv( CON_UART_ID, TERM_TIMER_ID, TERM_TIMEOUT ) ) == -1 )
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
    platform_uart_recv( CON_UART_ID, TERM_TIMER_ID, TERM_TIMEOUT );
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

  // Register the MMC filesystem
  dm_register( mmcfs_init() );

  // Register the ROM filesystem
  dm_register( romfs_init() );

#ifdef BUILD_XMODEM
  // Initialize XMODEM
  xmodem_init( xmodem_send, xmodem_recv );
#endif

#ifdef BUILD_TERM
  // Initialize terminal
  term_init( TERM_LINES, TERM_COLS, term_out, term_in, term_translate );
#endif

#ifdef FS_AUTORUN
  // Autorun: if "autorun.lua" is found in the file system, run it first
  if( ( fp = fopen( FS_AUTORUN, "r" ) ) != NULL )
  {
    fclose( fp );
    char* lua_argv[] = { "lua", FS_AUTORUN, NULL };
    lua_main( 2, lua_argv );
  }
#endif

  // Run the shell
  if( shell_init() == 0 )
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
