// Shell: 'wofmt' implementation

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "shell.h"
#include "common.h"
#include "type.h"
#include "platform_conf.h"
#include "romfs.h"

#ifdef BUILD_WOFS

const char shell_help_wofmt[] = "\n"
  "Formats the WOFS, initializing it to a blank state.\n";
const char shell_help_summary_wofmt[] = "WOFS format";

void shell_wofmt( int argc, char **argv )
{
  if( argc != 1 )
  {
    SHELL_SHOW_HELP( wofmt );
    return;
  }
  printf( "Formatting the internal WOFS will DESTROY ALL THE FILES FROM WOFS.\n" );
  if( shellh_ask_yes_no( "Are you sure you want to continue? [y/n] " ) == 0 )
    return;
  printf( "Formatting ..." );
  if( !wofs_format() )
  {
    printf( "\ni*** ERROR ***: unable to erase the internal flash. WOFS might be compromised.\n" );
    printf( "It is advised to re-flash the eLua image.\n" );
  }
  else
    printf( " done.\n" );
}

#else // #ifdef BUILD_WOFS

const char shell_help_wofmt[] = "";
const char shell_help_summary_wofmt[] = "";

void shell_wofmt( int argc, char **argv )
{
  shellh_not_implemented_handler( argc, argv );
}

#endif // #ifdef BUILD_WOFS

