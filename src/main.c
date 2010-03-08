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
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "term.h"
#include "platform_conf.h"
#include "elua_rfs.h"
#ifdef ELUA_SIMULATOR
#include "hostif.h"
#endif

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


#ifdef ELUA_BOOT_RPC

#ifndef RPC_UART_ID
  #define RPC_UART_ID     CON_UART_ID
#endif

#ifndef RPC_TIMER_ID
  #define RPC_TIMER_ID    CON_TIMER_ID
#endif

#ifndef RPC_UART_SPEED
  #define RPC_UART_SPEED  CON_UART_SPEED
#endif

void boot_rpc( void )
{
  lua_State *L = lua_open();
  luaL_openlibs(L);  /* open libraries */
  
  // Set up UART for 8N1 w/ adjustable baud rate
  platform_uart_setup( RPC_UART_ID, RPC_UART_SPEED, 8, PLATFORM_UART_PARITY_NONE, PLATFORM_UART_STOPBITS_1 );
  
  // Start RPC Server
  lua_getglobal( L, "rpc" );
  lua_getfield( L, -1, "server" );
  lua_pushnumber( L, RPC_UART_ID );
  lua_pushnumber( L, RPC_TIMER_ID );
  lua_pcall( L, 2, 0, 0 );
}
#endif

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

  // Register the MMC filesystem
  dm_register( mmcfs_init() );

  // Register the Semihosting filesystem
  dm_register( semifs_init() );

  // Register the remote filesystem
  dm_register( remotefs_init() );

  // Autorun: if "autorun.lua" is found in the file system, run it first
  if( ( fp = fopen( FS_AUTORUN, "r" ) ) != NULL )
  {
    fclose( fp );
    char* lua_argv[] = { "lua", FS_AUTORUN, NULL };
    lua_main( 2, lua_argv );
  }
  
#ifdef ELUA_BOOT_RPC
  boot_rpc();
#else
  
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
#endif // #ifdef ELUA_BOOT_RPC

#ifdef ELUA_SIMULATOR
  hostif_exit(0);
  return 0;
#else
  while( 1 );
#endif
}
