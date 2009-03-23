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
#include "term.h"
#include "platform_conf.h"

// Validate eLua configuratin options
#include "validate.h"

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
  
  // Register the ROM filesystem
  dm_register( romfs_init() );  

  // Autorun: if "autorun.lua" is found in the ROM file system, run it first
  if( ( fp = fopen( "/rom/autorun.lua", "r" ) ) != NULL )
  {
    fclose( fp );
    char* lua_argv[] = { "lua", "/rom/autorun.lua", NULL };
    lua_main( 2, lua_argv );    
  }
  
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
