// Shell: 'help' implementation

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

// External help text declaration
#define SHELL_HELP( topic )    extern const char shell_help_##topic[];\
  extern const char shell_help_summary_##topic[]
#define SHELL_INFO( cmd ) { #cmd, shell_help_summary_##cmd, shell_help_##cmd }
#define SHELL_INFO_ALIAS( cmd, alias ) { #cmd, shell_help_summary_##alias, shell_help_##alias }

// Help data
typedef struct
{
  const char *cmd;
  const char *help_summary;
  const char *help_full;
} SHELL_HELP_DATA;

SHELL_HELP( cp );
SHELL_HELP( rm );
SHELL_HELP( ls );
SHELL_HELP( recv );
SHELL_HELP( cat );
SHELL_HELP( lua );
SHELL_HELP( ver );
SHELL_HELP( mkdir );
SHELL_HELP( wofmt );
// 'mv' is special, as it uses the main help text from 'cp'
extern const char shell_help_summary_mv[];

// 'Help' help data is local for ovious reasons
static const char shell_help_help[] = "[<command>]\n"
  "  [<command>] - the command to get help on.\n"
  "Without arguments it shows a summary of all the shell commands.\n";
static const char shell_help_summary_help[] = "shell help";

// Also put the help for 'exit' here
static const char shell_help_exit[] = "\n"
  "Exits the shell.\n";
static const char shell_help_summary_exit[] = "exit the shell";

static const SHELL_HELP_DATA shell_help_data[] = 
{
  SHELL_INFO( help ),
  SHELL_INFO( lua ),
  SHELL_INFO( ls ),
  SHELL_INFO_ALIAS( dir, ls ),
  SHELL_INFO( cat ),
  SHELL_INFO_ALIAS( type, cat ),
  SHELL_INFO( recv ),
  SHELL_INFO( cp ),
  // Yes, 'mv' is still special
  { "mv", shell_help_summary_mv, shell_help_cp },
  SHELL_INFO( rm ),
  SHELL_INFO( ver ),
  SHELL_INFO( mkdir ),
  SHELL_INFO( wofmt ),
  SHELL_INFO( exit ),
  { NULL, NULL, NULL }
};

void shell_help( int argc, char **argv )
{
  const SHELL_HELP_DATA *ph;

  if( argc > 2 )
  {
    SHELL_SHOW_HELP( help );
    return;
  }
  ph = shell_help_data;
  if( argc == 1 )
  {
    // List commands and their summary
    // It is assumed that a command with an empty summary does not 
    // actually exist (helpful for conditional compilation)
    printf( "Shell commands:\n" );
    while( 1 )
    {
      if( ph->cmd == NULL )
        break;
      if( strlen( ph->help_summary ) > 0 )
        printf( "  %-6s - %s\n", ph->cmd, ph->help_summary );
      ph ++;
    }
    printf( "For more information use 'help <command>'.\n" );
  }
  else
  {
    while( 1 )
    {
      if( ph->cmd == NULL )
        break;
      if( !strcmp( ph->cmd, argv[ 1 ] ) && strlen( ph->help_summary ) > 0 )
      {
        printf( "%s - %s\nUsage: %s %s", ph->cmd, ph->help_summary, ph->cmd, ph->help_full );
        return;
      }
      ph ++;
    }
    printf( "Unknown command '%s'.\n", argv[ 1 ] );
  }
}

