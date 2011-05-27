// eLua shell

#include "shell.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "xmodem.h"
#include "version.h"
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

#include "platform_conf.h"
#ifdef BUILD_SHELL

// Shell alternate ' ' char
#define SHELL_ALT_SPACE           '\x07'

// EOF is different in UART mode and TCP/IP mode
#ifdef BUILD_CON_GENERIC
  #define SHELL_EOF_STRING        "CTRL+Z"
#else
  #define SHELL_EOF_STRING        "CTRL+D"
#endif

// Shell command handler function
typedef void( *p_shell_handler )( char* args );

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
static void shell_help( char* args )
{
  args = args;
  printf( "Shell commands:\n" );
  printf( "  exit        - exit from this shell\n" );
  printf( "  help        - print this help\n" );
  printf( "  ls or dir   - lists filesystems files and sizes\n" );
  printf( "  cat or type - lists file contents\n" );
  printf( "  lua [args]  - run Lua with the given arguments\n" );
  printf( "  recv        - receive a file via XMODEM and execute it\n" );
  printf( "  cp <src> <dst> - copy source file 'src' to 'dst'\n" );
  printf( "  ver         - print eLua version\n" );
}

// 'lua' handler
static void shell_lua( char* args )
{
  int nargs = 0;
  char* lua_argv[ SHELL_MAX_LUA_ARGS + 2 ];
  char *p, *prev, *temp;

  lua_argv[ 0 ] = "lua";
  // Process "args" if needed
  if( *args )
  {
    prev = args;
    p = strchr( args, ' ' );
    while( p )
    {
      if( nargs == SHELL_MAX_LUA_ARGS )
      {
        printf( "Too many arguments to 'lua' (maxim %d)\n", SHELL_MAX_LUA_ARGS );
        return;
      }
      *p = 0;
      lua_argv[ nargs + 1 ] = temp = prev;
      nargs ++;
      prev = p + 1;
      p = strchr( p + 1, ' ' );
      // If the argument is quoted, remove the quotes and transform the 'alternate chars' back to space
      if( *temp == '\'' || *temp == '"' )
      {
        temp ++;
        lua_argv[ nargs ] = temp;
        while( *temp )
        {
          if( *temp == SHELL_ALT_SPACE )
            *temp = ' ';
          temp ++;
        }
        *( temp - 1 ) = '\0';
      }
    }
  }
  lua_argv[ nargs + 1 ] = NULL;
  printf( "Press " SHELL_EOF_STRING " to exit Lua\n" );
  lua_main( nargs + 1, lua_argv );
  clearerr( stdin );
}

// 'recv' handler
static void shell_recv( char* args )
{
  args = args;

#ifndef BUILD_XMODEM
  printf( "XMODEM support not compiled, unable to recv\n" );
  return;
#else // #ifndef BUILD_XMODEM

  char *p;
  long actsize;
  lua_State* L;

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
  
  // Execute
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
  free( shell_prog );
  shell_prog = NULL;
#endif // #ifndef BUILD_XMODEM
}

// 'ver' handler
static void shell_ver( char* args )
{
  args = args;
  printf( "eLua version %s\n", ELUA_STR_VERSION );
  printf( "For more information visit www.eluaproject.net and wiki.eluaproject.net\n" );
}

// 'ls' and 'dir' handler
static void shell_ls( char* args )
{
  const DM_DEVICE *pdev;
  unsigned dev, i;
  DM_DIR *d;
  struct dm_dirent *ent;
  u32 total;

  // Iterate through all devices, looking for the ones that can do "opendir"
  for( dev = 0; dev < dm_get_num_devices(); dev ++ )
  {  
    pdev = dm_get_device_at( dev );
    if( pdev->p_opendir_r == NULL || pdev->p_readdir_r == NULL || pdev->p_closedir_r == NULL )
      continue;
    d = dm_opendir( pdev->name );
    if( d )
    {
      total = 0;
      printf( "\n%s", pdev->name );
      while( ( ent = dm_readdir( d ) ) != NULL )
      {
        printf( "\n%s", ent->fname );
        for( i = strlen( ent->fname ); i <= DM_MAX_FNAME_LENGTH; i++ )
          printf( " " );
        printf( "%u bytes", ( unsigned )ent->fsize );
        total = total + ent->fsize;
      }
      printf( "\n\nTotal on %s: %u bytes\n", pdev->name, ( unsigned )total );
      dm_closedir( d );
    }
  }   
  printf( "\n" );
}

// 'cat' and 'type' handler
static void shell_cat( char *args )
{
  FILE *fp;
  int c;
  char *p;

// *args has an appended space. Replace it with the string terminator.
//  *(strchr( args, ' ' )) = 0;
  if ( *args )
    while ( *args ) 
    {
      p = strchr( args, ' ' );
      *p = 0;
      if( ( fp = fopen( args , "rb" ) ) != NULL )
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
        printf( "File %s not found\n", args );
      args = p + 1;
    }      
  else
      printf( "Usage: cat (or type) <filename1> [<filename2> ...]\n" );
}    

// 'copy' handler
#ifdef BUILD_RFS
#define SHELL_COPY_BUFSIZE    ( ( 1 << RFS_BUFFER_SIZE ) - ELUARPC_WRITE_REQUEST_EXTRA )
#else
#define SHELL_COPY_BUFSIZE    256
#endif
static void shell_cp( char *args )
{
  char *p1, *p2;
  int res = 0;
  FILE *fps = NULL, *fpd = NULL;
  void *buf = NULL;
  size_t datalen, total = 0;

  if( *args )
  {
    p1 = strchr( args, ' ' );
    if( p1 )
    {
      *p1 = 0;
      p2 = strchr( p1 + 1, ' ' );
      if( p2 )
      {
        *p2 = 0;
        // First file is at args, second one at p1 + 1
        if( ( fps = fopen( args, "rb" ) ) == NULL )
          printf( "Unable to open %s for reading\n", args );
        else
        {
          if( ( fpd = fopen( p1 + 1, "wb" ) ) == NULL )
            printf( "Unable to open %s for writing\n", p1 + 1 );
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
                fwrite( buf, 1, datalen, fpd );
                total += datalen;
                if( datalen < SHELL_COPY_BUFSIZE )
                  break;
              }
              printf( "%u bytes copied\n", ( unsigned int )total );
              res = 1;
            }
          }
        } 
      }
    }
  }
  if( !res )
    printf( "Syntax error.\n" );
  if( fps )
    fclose( fps );
  if( fpd )
    fclose( fpd );
  if( buf )
    free( buf );
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

  printf( SHELL_WELCOMEMSG, ELUA_STR_VERSION );
  while( 1 )
  {
    while( linenoise_getline( LINENOISE_ID_SHELL, cmd, SHELL_MAXSIZE, SHELL_PROMPT ) == -1 )
    {
      printf( "\n" );
      clearerr( stdin );
    }
    if( strlen( cmd ) == 0 )
      continue;
    linenoise_addhistory( LINENOISE_ID_SHELL, cmd );
    if( cmd[ strlen( cmd ) - 1 ] != '\n' )
      strcat( cmd, "\n" );

    // Change '\r' and '\n' chars to ' ' to ease processing
    p = cmd;
    while( *p )
    {
      if( *p == '\r' || *p == '\n' )
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
    if( strlen( cmd ) == 1 )
      continue;

    // Look for the first ' ' to separate the command from its args
    temp = cmd;
    if( *temp == ' ' )
      temp ++;
    if( ( p = strchr( temp, ' ' ) ) == NULL )
    {
      printf( SHELL_ERRMSG );
      continue;
    }
    *p = 0;
    i = 0;
    while( 1 )
    {
      pcmd = shell_commands + i;
      if( pcmd->cmd == NULL )
      {
        printf( SHELL_ERRMSG );
        break;
      }
      if( !strcasecmp( pcmd->cmd, temp ) )
      {
        // Special case: the "exit" command has a NULL handler
        if( pcmd->handler_func )
          pcmd->handler_func( p + 1 );
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
