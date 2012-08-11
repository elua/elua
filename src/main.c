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
#ifdef BUILD_UMON
#if defined( FORLM3S9B92 )
  #define TARGET_IS_TEMPEST_RB1

  #include "lm3s9b92.h"
#elif defined( FORLM3S9D92 )
  #define TARGET_IS_FIRESTORM_RA2

  #include "lm3s9d92.h"
#elif defined( FORLM3S8962 )
  #include "lm3s8962.h"
#elif defined( FORLM3S6965 )
  #include "lm3s6965.h"
#elif defined( FORLM3S6918 )
  #include "lm3s6918.h"
#endif
#include "umon.h"
#include "umon_conf.h"
#include "hw_memmap.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "compilerdefs.h"
#endif

// Validate eLua configuratin options
#include "validate.h"

#include "mmcfs.h"
#include "romfs.h"
#include "semifs.h"

// Define here your autorun/boot files, 
// in the order you want eLua to search for them
char *boot_order[] = {
#if defined(BUILD_MMCFS)
  "/mmc/autorun.lua",
  "/mmc/autorun.lc",
#endif
#if defined(BUILD_ROMFS)
  "/rom/autorun.lua",
  "/rom/autorun.lc",
#endif
};

extern char etext[];


#ifdef ELUA_BOOT_RPC

#ifndef RPC_UART_ID
  #define RPC_UART_ID     CON_UART_ID
#endif

#ifndef RPC_TIMER_ID
  #define RPC_TIMER_ID    PLATFORM_TIMER_SYS_ID
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
// uMON initialization

#ifdef BUILD_UMON

static void umon_send( char ) NO_INSTRUMENT;
static void umon_send( char c )
{
  MAP_UARTCharPut( UART0_BASE, c );
}

u32 umon_call_stack[ UMON_STACK_TRACE_ENTRIES * 8 ];

#endif // #ifdef BUILD_UMON

// ****************************************************************************
//  Program entry point

int main( void )
{
  int i;
  FILE* fp;

  // Initialize platform first
  if( platform_init() != PLATFORM_OK )
  {
    // This should never happen
    while( 1 );
  }

#ifdef BUILD_UMON
  umon_set_output_func( umon_send );
#endif

  // Initialize device manager
  dm_init();

  // Register the ROM filesystem
  romfs_init();

  // Register the MMC filesystem
  mmcfs_init();

  // Register the Semihosting filesystem
  semifs_init();

  // Register the remote filesystem
  remotefs_init();

  // Search for autorun files in the defined order and execute the 1st if found
  for( i = 0; i < sizeof( boot_order ) / sizeof( *boot_order ); i++ )
  {
    if( ( fp = fopen( boot_order[ i ], "r" ) ) != NULL )
    {
      fclose( fp );
      char* lua_argv[] = { "lua", boot_order[i], NULL };
      lua_main( 2, lua_argv );
      break; // autoruns only the first found
    }
  }

#ifdef ELUA_BOOT_RPC
  boot_rpc();
#else
  
  // Run the shell
  if( shell_init() == 0 )
  {
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
