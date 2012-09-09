// eLua shell

#include "shell.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "xmodem.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "platform.h"
#include "elua_net.h"
#include "devman.h"
#include "buf.h"
#include "remotefs.h"
#include "eluarpc.h"
#include "linenoise.h"
#include "term.h"
#include "romfs.h"
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined( USE_GIT_REVISION )
#include "git_version.h"
#else
#include "version.h"
#endif

#include "platform_conf.h"
#ifdef BUILD_SHELL

// Shell alternate ' ' char
#define SHELL_ALT_SPACE           '\x07'
#define SHELL_MAX_ARGS            10

// EOF is different in UART mode and TCP/IP mode
#ifdef BUILD_CON_GENERIC
  #define SHELL_EOF_STRING        "CTRL+Z"
#else
  #define SHELL_EOF_STRING        "CTRL+D"
#endif

// Shell command handler function
typedef void( *p_shell_handler )( int argc, char **argv );

// Command/handler pair structure
typedef struct
{
  const char* cmd;
  p_shell_handler handler_func;
} SHELL_COMMAND;

// Shell data
static char* shell_prog;

// ****************************************************************************
// Shell functions

// 'help' handler
static void shell_help( int argc, char **argv )
{
  ( void )argc;
  ( void )argv;
  printf( "Shell commands:\n" );
  printf( "  exit        - exit from this shell\n" );
  printf( "  help        - print this help\n" );
  printf( "  ls or dir   - lists filesystems files and sizes\n" );
  printf( "  cat or type - lists file contents\n" );
  printf( "  lua [args]  - run Lua with the given arguments\n" );
  printf( "  recv [path] - receive a file via XMODEM. If path is given save it there, otherwise run it.\n");
  printf( "  cp <src> <dst> - copy source file 'src' to 'dst'\n" );
  printf( "  wofmt       - format the internal WOFS\n" );
  printf( "  ver         - print eLua version\n" );
}

// 'lua' handler
static void shell_lua( int argc, char **argv )
{
  printf( "Press " SHELL_EOF_STRING " to exit Lua\n" );
  lua_main( argc, argv );
  clearerr( stdin );
}

// 'recv' handler
static void shell_recv( int argc, char **argv )
{
#ifndef BUILD_XMODEM
  printf( "XMODEM support not compiled, unable to recv\n" );
#else // #ifndef BUILD_XMODEM

  char *p;
  long actsize;
  lua_State* L;

  if( argc > 2 )
  {
    printf( "Usage: recv [path]\n" );
    return;
  }

  if( ( shell_prog = malloc( XMODEM_INITIAL_BUFFER_SIZE ) ) == NULL )
  {
    printf( "Unable to allocate memory\n" );
    return;
  }
  printf( "Waiting for file ... " );
  if( ( actsize = xmodem_receive( &shell_prog ) ) < 0 )
  {
    free( shell_prog );
    shell_prog = NULL;
    if( actsize == XMODEM_ERROR_OUTOFMEM )
      printf( "file too big\n" );
    else
      printf( "XMODEM error\n" );
    return;
  }
  // Eliminate the XMODEM padding bytes
  p = shell_prog + actsize - 1;
  while( *p == '\x1A' )
    p --;
  p ++;
  printf( "done, got %u bytes\n", ( unsigned )( p - shell_prog ) );          
  
  // we've received an argument, try saving it to a file
  if( argc == 2 )
  {
    FILE *foutput = fopen( argv[ 1 ], "w" );
    size_t file_sz = p - shell_prog;
    if( foutput == NULL )
    {
      printf( "unable to open file %s\n", argv[ 1 ] );
      free( shell_prog );
      shell_prog = NULL;
      return;
    }
    if( fwrite( shell_prog, sizeof( char ), file_sz, foutput ) == file_sz )
      printf( "received and saved as %s\n", argv[ 1 ] );
    else
      printf( "unable to save file %s (no space left on target?)\n", argv[ 1 ] );
    fclose( foutput );
  }
  else // no arg, running the file with lua.
  {
    if( ( L = lua_open() ) == NULL )
    {
      printf( "Unable to create Lua state\n" );
      free( shell_prog );
      shell_prog = NULL;
      return;
    }
    luaL_openlibs( L );
    if( luaL_loadbuffer( L, shell_prog, p - shell_prog, "xmodem" ) != 0 )
      printf( "Error: %s\n", lua_tostring( L, -1 ) );
    else
      if( lua_pcall( L, 0, LUA_MULTRET, 0 ) != 0 )
        printf( "Error: %s\n", lua_tostring( L, -1 ) );
    lua_close( L );
  }
  free( shell_prog );
  shell_prog = NULL;
#endif // #ifndef BUILD_XMODEM
}

// 'ver' handler
static void shell_ver( int argc, char **argv )
{
  ( void )argc;
  ( void )argv;
  printf( "eLua version %s\n", ELUA_STR_VERSION );
  printf( "For more information visit www.eluaproject.net and wiki.eluaproject.net\n" );
}

// ls helper: does the recursive path walk for '-R'
static u32 shell_ls_helper( const char *crtname, int recursive, int *phasdirs )
{
  DM_DIR *d;
  u32 total = 0;
  struct dm_dirent *ent;
  unsigned i;
  char *fullname;
  int ndirs = 0;

  if( ( d = dm_opendir( crtname ) ) != NULL )
  {
    total = 0;
    printf( "\n%s", crtname );
    while( ( ent = dm_readdir( d ) ) != NULL )
    {
      printf( "\n%s", ent->fname );
      for( i = strlen( ent->fname ); i <= DM_MAX_FNAME_LENGTH; i++ )
        printf( " " );
      if( ent->flags & DM_DIRENT_FLAG_DIR )
      {
        printf( "<DIR>" );
        ndirs = ndirs + 1;
        if( phasdirs )
          *phasdirs = 1;
      }
      else
      {
        printf( "%u bytes", ( unsigned )ent->fsize );
        total = total + ent->fsize;
      }
    }
    dm_closedir( d );
    printf( "\nTotal on %s: %u bytes\n", crtname, ( unsigned )total );
    if( recursive && ( ndirs > 0 ) )
    {
      if( ( d = dm_opendir( crtname ) ) != NULL )
      {
        while( ( ent = dm_readdir( d ) ) != NULL )
        {
          if( ent->flags & DM_DIRENT_FLAG_DIR )
          {
            if( asprintf( &fullname, "%s/%s", crtname, ent->fname ) > 0 )
            {
              total += shell_ls_helper( fullname, 1, phasdirs );
              free( fullname );
            }
            else
              printf( "ERROR: unable to open directory '%s/%s' (not enough memory?)\n", crtname, ent->fname );
          }
        }
        dm_closedir( d );
      }
    }
  }
  else
    printf( "Error: unable to open directory '%s'", crtname );
  return total;
}

// 'ls' and 'dir' handler
// Syntax: ls [dir] [-R] 
static void shell_ls( int argc, char **argv )
{
  const DM_INSTANCE_DATA *pinst;
  unsigned dev;
  u32 i;
  int recursive = 0, hasdirs;
  char *pname = NULL;
  const char *crtname;

  for( i = 1; i < argc; i ++ )
  {
    if( !strcmp( argv[ i ], "-R" ) )
      recursive = 1;
    else if( argv[ i ][ 0 ] == '/' )
      pname = argv[ i ];
    else
      printf( "Warning: ignoring argument '%s' of ls", argv[ i ] );
  }
  // Iterate through all devices, looking for the ones that can do "opendir"
  // or the ones that match 'pname' (if that is specified)
  for( dev = 0; dev < dm_get_num_devices(); dev ++ )
  {  
    pinst = dm_get_instance_at( dev );
    if( pinst->pdev->p_opendir_r == NULL || pinst->pdev->p_readdir_r == NULL || pinst->pdev->p_closedir_r == NULL )
      continue;
    if( pname && strncmp( pinst->name, pname, strlen( pinst->name ) ) )
      continue;
    crtname = pname ? pname : pinst->name;
    hasdirs = 0;
    i = shell_ls_helper( crtname, recursive, &hasdirs );
    if( recursive && hasdirs )
      printf( "\nTotal on %s with all subdirectories: %u bytes\n", crtname, ( unsigned )i );
  }   
  printf( "\n" );
}

// 'cat' and 'type' handler
static void shell_cat( int argc, char **argv )
{
  FILE *fp;
  int c;
  unsigned i;

  if( argc < 2 )
  {
    printf( "Usage: cat (or type) <filename1> [<filename2> ...]\n" );
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

// 'copy' handler
#ifdef BUILD_RFS
#define SHELL_COPY_BUFSIZE    ( ( 1 << RFS_BUFFER_SIZE ) - ELUARPC_WRITE_REQUEST_EXTRA )
#else
#define SHELL_COPY_BUFSIZE    256
#endif
static void shell_cp( int argc, char **argv )
{
  FILE *fps = NULL, *fpd = NULL;
  void *buf = NULL;
  size_t datalen, datawrote, total = 0;

  if( argc != 3 )
  {
    printf( "Usage: cp <source> <destination>\n" );
    return;
  }
  if( ( fps = fopen( argv[ 1 ], "rb" ) ) == NULL )
    printf( "Unable to open %s for reading\n", argv[ 1 ] );
  else
  {
    if( ( fpd = fopen( argv[ 2 ], "wb" ) ) == NULL )
      printf( "Unable to open %s for writing\n", argv[ 2 ] );
    else
    {
      // Alloc memory
      if( ( buf = malloc( SHELL_COPY_BUFSIZE ) ) == NULL )
        printf( "Not enough memory\n" );
      else
      {
        // Do the actual copy
        while( 1 )
        {
          datalen = fread( buf, 1, SHELL_COPY_BUFSIZE, fps );
          datawrote = fwrite( buf, 1, datalen, fpd );
          if( datawrote < datalen )
          {
            printf( "Copy error (no space left on target?)\n" );
            break;
          }
          total += datalen;
          if( datalen < SHELL_COPY_BUFSIZE )
            break;
        }
        fflush( fpd );
        printf( "%u bytes copied\n", ( unsigned int )total );
      }
    }
  }
  if( fps )
    fclose( fps );
  if( fpd )
    fclose( fpd );
  if( buf )
    free( buf );
}

// 'wofmt' handler
static void shell_wofmt( int argc, char **argv )
{
#ifndef BUILD_WOFS
  printf( "WOFS not enabled.\n" );
#else // #ifndef BUILD_WOFS
  int c;

  printf( "Formatting the internal WOFS will DESTROY ALL THE FILES FROM WOFS.\n" );
  while( 1 )
  {
    printf( "Are you sure you want to continue? [y/n] " );
    c = term_getch( TERM_INPUT_WAIT );
    printf( "%c\n", isprint( c ) ? c : ' ' );
    c = tolower( c );
    if( c == 'n' )
      return;
    else if( c == 'y' )
      break;
  }
  printf( "Formatting ... " );
  if( !wofs_format() )
  {
    printf( "\ni*** ERROR ***: unable to erase the internal flash. WOFS might be compromised.\n" );
    printf( "It is advised to re-flash the eLua image.\n" );
  }
  else
    printf( " done.\n" );
#endif // #ifndef BUILD_WOFS
}

// mkdir handler
static void shell_mkdir( int argc, char **argv )
{
  if( argc != 2 )
  {
    printf( "Usage: mkdir <directory>\n" );
    return;
  }
  if( mkdir( argv[ 1 ], 0 ) != 0 )
    printf( "Error creating directory '%s'\n", argv[ 1 ] );
}

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
  { NULL, NULL }
};

// Execute the eLua "shell" in an infinite loop
void shell_start()
{
  char cmd[ SHELL_MAXSIZE + 1 ];
  char *p, *temp;
  const SHELL_COMMAND* pcmd;
  int i, inside_quotes;
  char quote_char;
  int argc;
  char *argv[ SHELL_MAX_ARGS ];

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
      continue;
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
      continue;

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
      continue;

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
        if( pcmd->handler_func )
          pcmd->handler_func( argc, argv );
        break;
      }
      i ++;
    }
    // Check for 'exit' command
    if( pcmd->cmd && !pcmd->handler_func )
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

