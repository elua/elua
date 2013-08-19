// eLua shell

#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "platform.h"
#include "elua_net.h"
#include "linenoise.h"
#include "term.h"
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "common.h"

#include "platform_conf.h"
#ifdef BUILD_SHELL

#if defined( USE_GIT_REVISION )
#include "git_version.h"
#else
#include "version.h"
#endif

// Shell alternate ' ' char
#define SHELL_ALT_SPACE           '\x07'
#define SHELL_MAX_ARGS            10

// External shell function declaration
#define SHELL_FUNC( func )        extern void func( int argc, char **argv )

// Shell data
char* shell_prog;

// Extern implementations of shell functions
SHELL_FUNC( shell_ls );
SHELL_FUNC( shell_cp );
SHELL_FUNC( shell_adv_mv );
SHELL_FUNC( shell_adv_rm );
SHELL_FUNC( shell_recv );
SHELL_FUNC( shell_help );
SHELL_FUNC( shell_cat );
SHELL_FUNC( shell_lua );
SHELL_FUNC( shell_ver );
SHELL_FUNC( shell_mkdir );
SHELL_FUNC( shell_wofmt );

// ----------------------------------------------------------------------------
// Helpers

// Helper: ask yes/no
// Returns 1 for yes, 0 for no
int shellh_ask_yes_no( const char *prompt )
{
  int c;

  if( prompt )
    printf( "%s ", prompt );
  while( 1 )
  {
#ifdef BUILD_TERM
    c = term_getch( TERM_INPUT_WAIT );
#else
    c = platform_uart_recv( CON_UART_ID, 0 /*UNUSED*/, PLATFORM_TIMER_INF_TIMEOUT );
#endif

    if( c == 'y' || c == 'Y' )
    {
      printf( "y\n" );
      return 1;
    }
    if( c == 'n' || c == 'N' )
    {
      printf( "n\n" );
      return 0;
    }
  }
  // Will never get here
  return 0;
}

#ifdef BUILD_RFS
#define SHELL_COPY_BUFSIZE    ( ( 1 << RFS_BUFFER_SIZE ) - ELUARPC_WRITE_REQUEST_EXTRA )
#else
#define SHELL_COPY_BUFSIZE    256
#endif

// Dummy log function
static int shellh_dummy_printf( const char *fmt, ... )
{
  return 0;
}

typedef int ( *p_logf )( const char *fmt, ... );

// Helper: copy one file to another file
// Return 1 for success, 0 for error
int shellh_cp_file( const char *psrcname, const char *pdestname, int flags )
{
  int fds = -1, fdd = -1;
  int res = 0;
  char *buf = NULL;
  ssize_t datalen, datawrote;
  u32 total = 0;
  p_logf plog = ( flags & SHELL_F_SILENT ) ? shellh_dummy_printf : printf;

  if( !strcasecmp( psrcname, pdestname ) )
  {
    plog( "Cannot copy '%s' into itself.\n", psrcname );
    goto done;
  }
  // If operation confirmation is enabled, ask the user first
  if( flags & SHELL_F_ASK_CONFIRMATION )
  {
    printf( "Copy '%s' to '%s' ? [y/n] ", psrcname, pdestname );
    if( shellh_ask_yes_no( NULL ) == 0 )
      goto done;
  }
  // Open source file
  if( ( fds = open( psrcname, O_RDONLY, 0 ) ) == -1 )
  {
    plog( "Error: unable to open source file '%s'\n", psrcname );
    goto done;
  }
  // If the destination exists and we need to ask for confirmation, do it now
  if( ( flags & SHELL_F_FORCE_DESTINATION ) == 0 )
  {
    if( ( fdd = open( pdestname, O_RDONLY, 0 ) ) != -1 )
    {
      close( fdd );
      fdd = -1;
      printf( "Destination '%s' already exists, are you sure you want to overwrite it ? [y/n] ", pdestname );
      if( shellh_ask_yes_no( NULL ) == 0 )
        goto done;
    }
  }
  // Allocate buffer
  if( ( buf = ( char* )malloc( SHELL_COPY_BUFSIZE ) ) == NULL )
  {
    plog( "ERROR: unable to allocate buffer for copy operation.\n" );
    goto done;
  }
  plog( "Copying '%s' to '%s' ... ", psrcname, pdestname );
  if( ( flags & SHELL_F_SIMULATE_ONLY ) == 0 )
  {
    // Open destination file
    if( ( fdd = open( pdestname, O_WRONLY | O_CREAT | O_TRUNC, 0 ) ) == -1 )
    {
      plog( "ERROR: unable to open '%s' for writing.\n", pdestname );
      goto done;
    }
    // Do the actual copy
    while( 1 )
    {
      if( ( datalen = read( fds, buf, SHELL_COPY_BUFSIZE ) ) == -1 )
      {
        plog( "Error reading source file '%s'.\n", psrcname );
        goto done;
      }
      if( ( datawrote = write( fdd, buf, datalen ) ) == -1 )
      {
        plog( "Error writing destination file '%s'.\n", pdestname );
        goto done;
      }
      if( datawrote < datalen )
      {
        plog( "Copy error (no space left on target?)\n" );
        goto done;
      }
      total += datalen;
      if( datalen < SHELL_COPY_BUFSIZE )
        break;
    }
  }
  plog( "done (%u bytes).\n", ( unsigned )total );
  res = 1;
done:
  if( fds != -1 )
    close( fds );
  if( fdd != -1 )
    close( fdd );
  if( buf )
    free( buf );
  return res;
}

// 'Not implemented' handler for shell comands
void shellh_not_implemented_handler( int argc, char **argv )
{
  printf( SHELL_ERRMSG );
}

// Shows the help for the given command
void shellh_show_help( const char *cmd, const char *helptext )
{
  printf( "Usage: %s %s", cmd, helptext );
}

// ****************************************************************************
// Public interface

// Insert shell commands here
static const SHELL_COMMAND shell_commands[] =
{
  { "help", shell_help },
  { "lua", shell_lua },
  { "recv", shell_recv },
  { "ver", shell_ver },
  { "exit", NULL },
  { "ls", shell_ls },
  { "dir", shell_ls },
  { "cat", shell_cat },
  { "type", shell_cat },
  { "cp", shell_cp },
  { "wofmt", shell_wofmt },
  { "mkdir", shell_mkdir },
  { "rm", shell_adv_rm },
  { "mv", shell_adv_mv },
  { NULL, NULL }
};

// Executes the given shell command
// 'interactive_mode' is 1 if invoked directly from the interactive shell,
// 0 otherwise
// Returns a pointer to the shell_command that was executed, NULL for error
const SHELL_COMMAND* shellh_execute_command( char* cmd, int interactive_mode )
{  
  char *p, *temp;
  const SHELL_COMMAND* pcmd;
  int i, inside_quotes;
  char quote_char;
  int argc;
  char *argv[ SHELL_MAX_ARGS ];

  if( strlen( cmd ) == 0 )
    return NULL;

  if( cmd[ strlen( cmd ) - 1 ] != '\n' )
    strcat( cmd, "\n" );

  // Change '\r', '\n' and '\t' chars to ' ' to ease processing
  p = cmd;
  while( *p )
  {
    if( *p == '\r' || *p == '\n' || *p == '\t' )
      *p = ' ';
    p ++;
  }

  // Transform ' ' characters inside a '' or "" quoted string in
  // a 'special' char. We do this to let the user execute something
  // like "lua -e 'quoted string'" without disturbing the quoted
  // string in any way.
  for( i = 0, inside_quotes = 0, quote_char = '\0'; i < strlen( cmd ); i ++ )
    if( ( cmd[ i ] == '\'' ) || ( cmd[ i ] == '"' ) )
    {
      if( !inside_quotes )
      {
        inside_quotes = 1;
        quote_char = cmd[ i ];
      }
      else
      {
        if( cmd[ i ] == quote_char )
        {
          inside_quotes = 0;
          quote_char = '\0';
        }
      }
    }
    else if( ( cmd[ i ] == ' ' ) && inside_quotes )
      cmd[ i ] = SHELL_ALT_SPACE;
  if( inside_quotes )
  {
    printf( "Invalid quoted string\n" );
    return NULL;
  }

  // Transform consecutive sequences of spaces into a single space
  p = strchr( cmd, ' ' );
  while( p )
  {
    temp = p + 1;
    while( *temp && *temp == ' ' )
      memmove( temp, temp + 1, strlen( temp ) );
    p = strchr( p + 1, ' ' );
  }
  if( !strcmp( cmd, " " ) )
    return NULL;

  // Skip over the initial space char if it exists
  p = cmd;
  if( *p == ' ' )
    p ++;

  // Add a final space if it does not exist
  if( p[ strlen( p ) - 1 ] != ' ' )
    strcat( p, " " );

  // Compute argc/argv
  for( argc = 0; argc < SHELL_MAX_ARGS; argc ++ )
    argv[ argc ] = NULL;
  argc = 0;
  while( ( temp = strchr( p, ' ' ) ) != NULL )
  {
    *temp = 0;
    if( argc == SHELL_MAX_ARGS )
    {
      printf( "Error: too many arguments\n" );
      argc = -1;
      break;
    }
    argv[ argc ++ ] = p;
    p = temp + 1;
  }

  if( argc == -1 )
    return NULL;

  // Additional argument processing happens here
  for( i = 0; i < argc; i ++ )
  {
    p = argv[ i ];
    // Put back spaces if needed
    for( inside_quotes = 0; inside_quotes < strlen( argv[ i ] ); inside_quotes ++ )
    {
      if( p[ inside_quotes ] == SHELL_ALT_SPACE )
        argv[ i ][ inside_quotes ] = ' ';
    }
    // Remove quotes
    if( ( p[ 0 ] == '\'' || p [ 0 ] == '"' ) && ( p[ 0 ] == p[ strlen( p ) - 1 ] ) )
    {
      argv[ i ] = p + 1;
      p[ strlen( p ) - 1 ] = '\0';
    }
  }

  // Match user command with shell's commands
  i = 0;
  while( 1 )
  {
    pcmd = shell_commands + i;
    if( pcmd->cmd == NULL )
    {
      printf( SHELL_ERRMSG );
      break;
    }
    if( !strcasecmp( pcmd->cmd, argv[ 0 ] ) )
    {
      // Special case: the "exit" command has a NULL handler
      // Special case: "lua" is not allowed in non-interactive mode
      if( pcmd->handler_func && ( interactive_mode || strcasecmp( pcmd->cmd, "lua" ) ) )
        pcmd->handler_func( argc, argv );
      break;
    }
    i ++;
  }

  // Special case: "exit" is not allowed in non-interactive mode
  if( !interactive_mode && !strcasecmp( pcmd->cmd, "exit" ) )
    return NULL;
  return pcmd;
}

// Execute the eLua "shell" in an infinite loop
void shell_start()
{
  char cmd[ SHELL_MAXSIZE + 1 ];
  const SHELL_COMMAND *pcmd;
#ifdef BUILD_UIP
  int i;
#endif

  printf( SHELL_WELCOMEMSG, ELUA_STR_VERSION );
  while( 1 )
  {
    while( linenoise_getline( LINENOISE_ID_SHELL, cmd, SHELL_MAXSIZE - 1, SHELL_PROMPT ) == -1 )
    {
      printf( "\n" );
      clearerr( stdin );
    }
    if( strlen( cmd ) == 0 )
      continue;
    linenoise_addhistory( LINENOISE_ID_SHELL, cmd );
    pcmd = shellh_execute_command( cmd, 1 );
    // Check for 'exit' command
    if( pcmd && pcmd->cmd && !pcmd->handler_func )
#ifdef BUILD_UIP
    {
      if( ( i = elua_net_get_telnet_socket() ) != -1 )
        elua_net_close( i );
    }
#else
      break;
#endif
  }
  // Shell exit point
  if( shell_prog )
    free( shell_prog );
}

// Initialize the shell, returning 1 for OK and 0 for error
int shell_init()
{
  shell_prog = NULL;
  return 1;
}

#else // #ifdef BUILD_SHELL

int shell_init()
{
  return 0;
}

void shell_start()
{
}

#endif // #ifdef BUILD_SHELL

