// Shell: 'ver' implementation

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

#if defined( USE_GIT_REVISION )
#include "git_version.h"
#else
#include "version.h"
#endif

const char shell_help_ver[] = "\n"
  "This displays the git revision of the tree used to build eLua or an official version number if applicable.\n";
const char shell_help_summary_ver[] = "show version information";

void shell_ver( int argc, char **argv )
{
  if( argc != 1 )
  {
    SHELL_SHOW_HELP( ver );
    return;
  }
  printf( "eLua version %s\n", ELUA_STR_VERSION );
  printf( "For more information visit www.eluaproject.net and wiki.eluaproject.net\n" );
}

