// Advanced shell: 'cp' and 'mv' implementation

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "shell.h"
#include "common.h"
#include "type.h"
#include "platform_conf.h"

#ifdef BUILD_ADVANCED_SHELL

typedef struct
{
  const char *pdestdir;
  const char *psrcdir;
  const char *dloc;
  u8 flags;
} SHELL_CP_STATE;

const char shell_help_cp[] = "<source> <destination> [-R] [-f] [-c] [-s]\n"
  "  <source>: source file/directory/file mask.\n"
  "  <destination>: destination file/directory.\n"
  "  [-R]: recursive\n"
  "  [-f]: force destination override (default is to ask confirmation).\n"
  "  [-c]: confirm each operation.\n"
  "  [-s]: simulate only (no actual operation).\n";
const char shell_help_summary_cp[] = "copy files";

#define shell_help_mv         shell_help_cp
const char shell_help_summary_mv[] = "move/rename files";

int shellh_cp_or_mv_file( const char *psrc, const char *pdest, int flags )
{
  if( flags & SHELL_F_MOVE )
  {
    if( rename( psrc, pdest ) == -1 )
    {
      if( errno != EXDEV )
      {
        printf( "Unable to rename '%s'.\n", psrc );
        return 0;
      }
    }
    else
    {
      printf( "Moved '%s' to '%s'.\n", psrc, pdest );
      return 1;
    }
  }
  if( !shellh_cp_file( psrc, pdest, flags ) )
    return 0;
  if( flags & SHELL_F_MOVE )
  {
    if( unlink( psrc ) == -1 )
    {
      printf( "Unable to remove original file '%s'.\n", psrc );
      return 0;
    }
  }
  return 1;
}

// copy/move callback
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
      if( ( tmp = ( char* )cmn_fs_path_join( ps->pdestdir, path + strlen( ps->psrcdir ) - strlen( ps->dloc ), NULL ) ) == NULL )
      {
        printf( "Not enough memory.\n" );
        goto done_err;
      }
      if( ( d = dm_opendir( tmp ) ) != NULL )
        goto done;
      printf( "Creating directory %s ... ", tmp );
      if( ( ps->flags & SHELL_F_SIMULATE_ONLY ) == 0 )
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
          printf( "ERROR: unable to handle directory '%s' (internal error?), aborting.\n", path );
          goto done_err;
        }
        if( ( tmp = cmn_fs_path_join( path, pent->fname, NULL ) ) == NULL )
        {
          printf( "Not enough memory.\n" );
          goto done_err;
        }
        if( ( tmp2 = cmn_fs_path_join( ps->pdestdir, path + strlen( ps->psrcdir ) - strlen( ps->dloc ), pent->fname, NULL ) ) == NULL )
        {
          printf( "Not enough memory.\n" );
          goto done_err;
        }
        shellh_cp_or_mv_file( tmp, tmp2, ps->flags );
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

static void shellh_adv_cp_mv_common( int argc, char **argv, int is_move )
{
  const char *srcpath = NULL, *dstpath = NULL;
  unsigned i, flags = is_move ? SHELL_F_MOVE : 0;
  int srctype, dsttype;
  char *srcdir = NULL, *dstdir = NULL, *tmp = NULL;
  const char *srcfile, *dstfile;
  SHELL_CP_STATE state;

  if( argc < 3 )
  {
    if( is_move )
      SHELL_SHOW_HELP( mv );
    else
      SHELL_SHOW_HELP( cp );
    return;
  }
  for( i = 1; i < argc; i ++ )
  {
    if( !strcmp( argv[ i ], "-R" ) )
      flags |= SHELL_F_RECURSIVE;
    else if( !strcmp( argv[ i ], "-f" ) )
      flags |= SHELL_F_FORCE_DESTINATION;
    else if( !strcmp( argv[ i ], "-c" ) )
      flags |= SHELL_F_ASK_CONFIRMATION;
    else if( !strcmp( argv[ i ], "-s" ) )
      flags |= SHELL_F_SIMULATE_ONLY;
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
  if( srctype == CMN_FS_TYPE_ERROR || srctype == CMN_FS_TYPE_FILE_NOT_FOUND ||
      srctype == CMN_FS_TYPE_DIR_NOT_FOUND || srctype == CMN_FS_TYPE_UNKNOWN_NOT_FOUND || 
      dsttype == CMN_FS_TYPE_ERROR || dsttype == CMN_FS_TYPE_DIR_NOT_FOUND )
  {
    printf( "%d %d\n", srctype, dsttype );
    printf( "Invalid source and/or destination.\n" );
    return;
  }
  srcdir = cmn_fs_split_path( srcpath, &srcfile );
  dstdir = cmn_fs_split_path( dstpath, &dstfile );
  // Check valid source/destination combinations
  if( srctype == CMN_FS_TYPE_FILE )
  {
    if( dsttype == CMN_FS_TYPE_FILE || dsttype == CMN_FS_TYPE_FILE_NOT_FOUND || dsttype == CMN_FS_TYPE_UNKNOWN_NOT_FOUND ) // direct file-to-file operation
    {
      shellh_cp_or_mv_file( srcpath, dstpath, flags );
      goto done;
    }
    else if( dsttype == CMN_FS_TYPE_DIR ) // copy/move to destination dir with the same name
    {
      if( ( tmp = cmn_fs_path_join( dstdir, srcfile, NULL ) ) == NULL )
      {
        printf( "Not enough memory.\n" );
        goto done;
      }
      shellh_cp_or_mv_file( srcpath, tmp, flags );
      goto done;
    }
    else
    {
      printf( "Invalid destination.\n" );
      goto done;
    }
  }
  else
  {
    if( dsttype == CMN_FS_TYPE_FILE || dsttype == CMN_FS_TYPE_FILE_NOT_FOUND )
    {
      printf( "Invalid destination '%s'.\n", dstpath );
      goto done;
    }
    memset( &state, 0, sizeof( state ) );
    state.dloc = NULL;
    if( ( flags & SHELL_F_RECURSIVE ) != 0 )
      state.dloc = strrchr( srcdir, '/' );
    if( state.dloc == NULL )
      state.dloc = "";
    state.flags = flags;
    state.pdestdir = dstdir;
    state.psrcdir = srcdir;
    cmn_fs_walkdir( srcpath, shellh_cp_walkdir_cb, &state, flags & SHELL_F_RECURSIVE );
  }
done:
  if( srcdir )
    free( srcdir );
  if( dstdir )
    free( dstdir );
  if( tmp )
    free( tmp );
}

void shell_cp( int argc, char **argv )
{
  shellh_adv_cp_mv_common( argc, argv, 0 );
}

void shell_adv_mv( int argc, char **argv )
{
  shellh_adv_cp_mv_common( argc, argv, 1 );
}

#else // #ifdef BUILD_ADVANCED_SHELL

const char shell_help_cp[] = "cp <source> <destination>\n"
  "  <source>: source file/directory/file mask.\n"
  "  <destination>: destination file/directory.\n";
const char shell_help_summary_cp[] = "copy files";

const char shell_help_mv[] = "";
const char shell_help_summary_mv[] = "";

void shell_cp( int argc, char **argv )
{
  if( argc != 3 )
  {
    SHELL_SHOW_HELP( cp );
    return;
  }
  shellh_cp_file( argv[ 1 ], argv[ 2 ], 0 );
}

void shell_adv_mv( int argc, char **argv )
{
  shellh_not_implemented_handler( argc, argv );
}

#endif // #ifdef BUILD_ADVANCED_SHELL

