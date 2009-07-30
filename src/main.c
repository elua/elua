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

#ifdef ELUA_SIMULATOR
  hostif_exit(0);
  return 0;
#else
  while( 1 );
#endif
}

