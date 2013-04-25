// Shell: 'cat' implementation

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

const char shell_help_cat[] = "<file> [<file2>] ... [<filen>]"
  "  <file>: the file to list.\n"
  "  [<file2>] ... [<filen>]: other files to list.\n";
const char shell_help_summary_cat[] = "list the contents of a file";

void shell_cat( int argc, char **argv )
{
  FILE *fp;
  int c;
  unsigned i;

  if( argc < 2 )
  {
    shellh_show_help( argv[ 0 ], shell_help_cat );
    return;
  }
  for( i = 1; i < argc; i ++ )
  {
    if( ( fp = fopen( argv[ i ] , "rb" ) ) != NULL )
    {
      c = fgetc( fp );
      while( c != EOF )
      {
        printf("%c", (char) c );
        c = fgetc( fp );
      }
      fclose ( fp );
    }
     else
      printf( "Unable to open '%s'\n", argv[ i ] );
  }
}    


