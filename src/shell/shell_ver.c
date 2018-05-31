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

#ifndef SHELL_HELP_VER_STRING
#define SHELL_HELP_VER_STRING "\nThis displays the git revision of the tree used to build eLua or an official version number if applicable.\n"
#endif

#ifndef SHELL_HELP_SUMMARY_STRING
#define SHELL_HELP_SUMMARY_STRING "show version information"
#endif

#ifndef SHELL_HELP_LINE1_STRING
#define SHELL_HELP_LINE1_STRING "eLua version %s\n"
#endif

#ifndef SHELL_HELP_LINE2_STRING
#define SHELL_HELP_LINE2_STRING "For more information visit www.eluaproject.net and wiki.eluaproject.net\n"
#endif

const char shell_help_ver[] = SHELL_HELP_VER_STRING;
const char shell_help_summary_ver[] = SHELL_HELP_SUMMARY_STRING;

void shell_ver( int argc, char **argv )
{
  if( argc != 1 )
  {
    SHELL_SHOW_HELP( ver );
    return;
  }
  printf( SHELL_HELP_LINE1_STRING, ELUA_STR_VERSION );
  printf( SHELL_HELP_LINE2_STRING );
}

