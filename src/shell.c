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
#include "common.h"

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

// ----------------------------------------------------------------------------
// Helpers

// Helper: ask yes/no
// Returns 1 for yes, 0 for no
static int shellh_ask_yes_no( const char *prompt )
{
  int c;

  if( prompt )
    printf( "%s ", prompt );
  while( 1 )
  {
    c = term_getch( TERM_INPUT_WAIT );
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


// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
// 'lua' handler

static void shell_lua( int argc, char **argv )
{
  printf( "Press " SHELL_EOF_STRING " to exit Lua\n" );
  lua_main( argc, argv );
  clearerr( stdin );
}

// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
// 'ver' handler

static void shell_ver( int argc, char **argv )
{
  ( void )argc;
  ( void )argv;
  printf( "eLua version %s\n", ELUA_STR_VERSION );
  printf( "For more information visit www.eluaproject.net and wiki.eluaproject.net\n" );
}

// ----------------------------------------------------------------------------
// 'ls'/'dir' handler

// State for walkdir
typedef struct
{
  u32 dir_total;
  u32 total;
  u8 ndirs;
} SHELL_LS_STATE;

// 'ls' and 'dir' handler
// Syntax: ls [dir] [-R]
// directory walker callback
static int shellh_ls_walkdir_cb( const char *path, const struct dm_dirent *pent, void *pdata, int info )
{
  SHELL_LS_STATE *ps = ( SHELL_LS_STATE* )pdata;

  switch( info )
  {
    case CMN_FS_INFO_BEFORE_READDIR:
      ps->dir_total = 0;
      printf( "%s\n", path );
      if( ps->ndirs != 0xFF )
        ps->ndirs ++;
      break;

    case CMN_FS_INFO_INSIDE_READDIR:
      printf( "  %-30s", pent->fname );
      if( DM_DIRENT_IS_DIR( pent ) )
        printf( "<DIR>\n" );
      else
      {
        printf( "%u bytes\n", ( unsigned )pent->fsize );
        ps->dir_total += pent->fsize;
      }
      break;

    case CMN_FS_INFO_BEFORE_CLOSEDIR:
      printf( "Total on %s: %u bytes\n\n", path, ( unsigned )ps->dir_total );
      ps->total += ps->dir_total;
      break;

    case CMN_FS_INFO_OPENDIR_FAILED:
      printf( "WARNING: unable to open %s\n", path );
      break;
  }
  return 1;
}

static void shell_ls( int argc, char **argv )
{
  const DM_INSTANCE_DATA *pinst;
  unsigned i;
  int recursive = 0;
  char *pname = NULL;
  const char *crtname;
  SHELL_LS_STATE state;

  for( i = 1; i < argc; i ++ )
  {
    if( !strcmp( argv[ i ], "-R" ) )
      recursive = 1;
    else if( argv[ i ][ 0 ] == '/' && !pname )
      pname = argv[ i ];
    else
      printf( "Warning: ignoring argument '%s' of ls", argv[ i ] );
  }
  // Iterate through all devices, looking for the ones that can do "opendir"
  // or the ones that match 'pname' (if that is specified)
  for( i = 0; i < dm_get_num_devices(); i ++ )
  {  
    pinst = dm_get_instance_at( i );
    if( pinst->pdev->p_opendir_r == NULL || pinst->pdev->p_readdir_r == NULL || pinst->pdev->p_closedir_r == NULL )
      continue;
    if( pname && strncmp( pinst->name, pname, strlen( pinst->name ) ) )
      continue;
    crtname = pname ? pname : pinst->name;
    memset( &state, 0, sizeof( state ) );
    cmn_fs_walkdir( crtname, shellh_ls_walkdir_cb, &state, recursive );
    if( recursive && ( state.ndirs > 1 ) )
      printf( "Total on %s with all subdirectories: %u bytes\n\n", crtname, ( unsigned )state.total );
  }   
}

// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
// 'cp' handler

#ifdef BUILD_RFS
#define SHELL_COPY_BUFSIZE    ( ( 1 << RFS_BUFFER_SIZE ) - ELUARPC_WRITE_REQUEST_EXTRA )
#else
#define SHELL_COPY_BUFSIZE    256
#endif

// 'cp' flags
#define SHELL_CP_FLAG_RECURSIVE         1
#define SHELL_CP_FORCE_DESTINATION      2
#define SHELL_CP_ASK_CONFIRMATION       4
#define SHELL_CP_SIMULATE_ONLY          8

typedef struct
{
  const char *pdestdir;
  const char *psrcdir;
  u8 flags;
} SHELL_CP_STATE;

// Helper: copy one file to another file
// Return 1 for success, 0 for error
static int shellh_cp_one_file( const char *psrcname, const char *pdestname, int flags )
{
  FILE *fps = NULL, *fpd = NULL;
  int res = 0;
  char *buf = NULL;
  u32 datalen, datawrote, total = 0;

  // If operation confirmation is enabled, ask the user first
  if( flags & SHELL_CP_ASK_CONFIRMATION )
  {
    printf( "Copy '%s' to '%s' ? [y/n] ", psrcname, pdestname );
    if( shellh_ask_yes_no( NULL ) == 0 )
      goto done;
  }
  // Open source file
  if( ( fps = fopen( psrcname, "r" ) ) == NULL )
  {
    printf( "Error: unable to open source file '%s'\n", psrcname );
    goto done;
  }
  // If the destination exists and we need to ask for confirmation, do it now
  if( ( flags & SHELL_CP_FORCE_DESTINATION ) == 0 )
  {
    if( ( fpd = fopen( pdestname, "r" ) ) != NULL )
    {
      fclose( fpd );
      fpd = NULL;
      printf( "Destination '%s' already exists, are you sure you want to overwrite it ? [y/n] ", pdestname );
      if( shellh_ask_yes_no( NULL ) == 0 )
        goto done;
    }
  }
  // Allocate buffer
  if( ( buf = ( char* )malloc( SHELL_COPY_BUFSIZE ) ) == NULL )
  {
    printf( "ERROR: unable to allocate buffer for copy operation.\n" );
    goto done;
  }
  printf( "Copying '%s' to '%s' ... ", psrcname, pdestname );
  if( ( flags & SHELL_CP_SIMULATE_ONLY ) == 0 )
  {
    // Open destination file 
    if( ( fpd = fopen( pdestname, "w" ) ) == NULL )
    {
      printf( "ERROR: unable to open '%s' for writing.\n", pdestname );
      goto done;
    }
    // Do the actual copy
    while( 1 )
    {
      datalen = fread( buf, 1, SHELL_COPY_BUFSIZE, fps );
      datawrote = fwrite( buf, 1, datalen, fpd );
      if( datawrote < datalen )
      {
        printf( "Copy error (no space left on target?)\n" );
        goto done;
      }
      total += datalen;
      if( datalen < SHELL_COPY_BUFSIZE )
        break;
    }
    fflush( fpd );
  }
  printf( "done (%u bytes).\n", ( unsigned )total );
done:
  if( fps )
    fclose( fps );
  if( fpd )
    fclose( fpd );
  if( buf )
    free( buf );
  return res;
}

// copy callback
static int shellh_cp_walkdir_cb( const char *path, const struct dm_dirent *pent, void *pdata, int info )
{
  SHELL_CP_STATE *ps = ( SHELL_CP_STATE* )pdata;
  char *tmp = NULL, *tmp2 = NULL;
  int res = 1;
  DM_DIR *d = NULL;

  switch( info )
  {
    case CMN_FS_INFO_BEFORE_READDIR:
      if( strstr( path, ps->psrcdir ) != path )
      {
        printf( "ERROR: unable to handle directory '%s' (internal error?), aborting.\n", path );
        goto done_err;
      }
      // Need to create this directory if it does not exist
      if( ( tmp = ( char* )cmn_fs_path_join( ps->pdestdir, path + strlen( ps->psrcdir ), NULL ) ) == NULL )
      {
        printf( "Not enough memory.\n" );
        goto done_err;
      }
      if( ( d = dm_opendir( tmp ) ) != NULL )
        goto done;
      printf( "Creating directory %s ... ", tmp );
      if( ( ps->flags & SHELL_CP_SIMULATE_ONLY ) == 0 )
      {
        if( mkdir( tmp, 0 ) == -1 )
        {
          printf( "ERROR! (aborting).\n" );
          goto done_err;
        }
        else
          printf( "done.\n" );
      }
      else
        printf( "done.\n" );
      goto done;

    case CMN_FS_INFO_INSIDE_READDIR:
      if( !DM_DIRENT_IS_DIR( pent ) )
      {
        if( strstr( path, ps->psrcdir ) != path )
        {
          printf( "ERROR: unable to handler directory '%s' (internal error?), aborting.\n", path );
          goto done_err;
        }
        if( ( tmp = cmn_fs_path_join( path, pent->fname, NULL ) ) == NULL )
        {
          printf( "Not enough memory.\n" );
          goto done_err;
        }
        if( ( tmp2 = cmn_fs_path_join( ps->pdestdir, path + strlen( ps->psrcdir ), pent->fname, NULL ) ) == NULL )
        {
          printf( "Not enough memory.\n" );
          goto done_err;
        }
        shellh_cp_one_file( tmp, tmp2, ps->flags );
      }
      goto done;

    case CMN_FS_INFO_OPENDIR_FAILED:
      printf( "ERROR: unable to read directory '%s', aborting.\n", path );
      goto done_err;

    default:
      goto done;
  }
done_err:
  res = 0;
done:
  if( tmp )
    free( tmp );
  if( tmp2 )
    free( tmp2 );
  if( d )
    dm_closedir( d );
  return res;
}

static void shell_cp( int argc, char **argv )
{
  const char *srcpath = NULL, *dstpath = NULL;
  unsigned i, flags = 0;
  int srctype, dsttype;
  char *srcdir = NULL, *dstdir = NULL, *tmp = NULL;
  const char *srcfile, *dstfile;
  SHELL_CP_STATE state;

  if( argc < 3 )
  {
    printf( "Usage: cp <source> <destination> [-R] [-f] [-c]\n" );
    printf( "  [-R]: recursive\n" );
    printf( "  [-f]: force destination override (default is to ask confirmation).\n" );
    printf( "  [-c]: confirm each file copy.\n" );
    printf( "  [-s]: simulate only (no actual operation).\n" );
    return;
  }
  for( i = 1; i < argc; i ++ )
  {
    if( !strcmp( argv[ i ], "-R" ) )
      flags |= SHELL_CP_FLAG_RECURSIVE;
    else if( !strcmp( argv[ i ], "-f" ) )
      flags |= SHELL_CP_FORCE_DESTINATION;
    else if( !strcmp( argv[ i ], "-c" ) )
      flags |= SHELL_CP_ASK_CONFIRMATION;
    else if( !strcmp( argv[ i ], "-s" ) )
      flags |= SHELL_CP_SIMULATE_ONLY;
    else if( argv[ i ][ 0 ] == '/' )
    {
      if( !srcpath )
        srcpath = argv[ i ];
      else if( !dstpath )
        dstpath = argv[ i ];
      else
        printf( "WARNING: ignoring argument '%s'\n", argv[ i ] );
    }
    else
      printf( "WARNING: ignoring argument '%s'\n", argv[ i ] );
  }
  if( !srcpath || !dstpath )
  {
    printf( "Source and/or destination not specified.\n" );
    return;
  }
  srctype = cmn_fs_get_type( srcpath );
  if( ( dsttype = cmn_fs_get_type( dstpath ) ) == CMN_FS_TYPE_PATTERN )
  {
    printf( "Invalid destination '%s'.\n", dstpath );
    goto done;
  }
  if( srctype == CMN_FS_TYPE_ERROR || dsttype == CMN_FS_TYPE_ERROR )
  {
    printf( "Invalid source and/or destination.\n" );
    return;
  }
  srcdir = cmn_fs_split_path( srcpath, &srcfile );
  dstdir = cmn_fs_split_path( dstpath, &dstfile );
  // Check valid source/destination combinations
  if( srctype == CMN_FS_TYPE_FILE )
  {
    if( dsttype == CMN_FS_TYPE_FILE ) // direct file-to-file copy
    {
      shellh_cp_one_file( srcpath, dstpath, flags );
      goto done;
    }
    else if( dsttype == CMN_FS_TYPE_DIR ) // copy to destination dir with the same name
    {
      if( ( tmp = cmn_fs_path_join( dstdir, srcfile, NULL ) ) == NULL )
      {
        printf( "Not enough memory.\n" );
        goto done;
      }
      shellh_cp_one_file( srcpath, tmp, flags );
      goto done;
    }
    else
    {
      printf( "Invalid arguments.\n" );
      goto done;
    }
  }
  else
  {
    if( dsttype == CMN_FS_TYPE_FILE )
    {
      printf( "Invalid destination '%s'.\n", dstpath );
      goto done;
    }
    memset( &state, 0, sizeof( state ) );
    state.flags = flags;
    state.pdestdir = dstdir;
    state.psrcdir = srcdir;
    cmn_fs_walkdir( srcpath, shellh_cp_walkdir_cb, &state, flags & SHELL_CP_FLAG_RECURSIVE );
  }
done:
  if( srcdir )
    free( srcdir );
  if( dstdir )
    free( dstdir );
  if( tmp )
    free( tmp );
}

// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
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

