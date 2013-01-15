// Advanced shell: 'rm' implementation

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

#ifdef BUILD_ADVANCED_SHELL

const char shell_help_rm[] = "<filemask> [-R] [-c] [-s]\n"
  "  <filemask>: file, directory or file mask to remove.\n"
  "  -R: remove recursively.\n"
  "  -c: confirm each remove.\n"
  "  -s: simulate only (no actual operation).\n";
const char shell_help_summary_rm[] = "remove files";

// helper: remove a single file and/or directory
static void shellh_rm_one( const char* path, int flags )
{
  int ftype = cmn_fs_get_type( path );
  int res = 0;

  if( flags & SHELL_F_ASK_CONFIRMATION )
  {
     printf( "Are you sure you want to remove %s ? [y/n] ", path );
     if( shellh_ask_yes_no( NULL ) == 0 )
       return;
  }
  if( ( flags & SHELL_F_SIMULATE_ONLY ) == 0 )
  {
    if( ftype == CMN_FS_TYPE_FILE )
      res = unlink( path );
    else if( ftype == CMN_FS_TYPE_DIR )
      res = rmdir( path );
    else
    {
      printf( "WARNING: invalid argument '%s'\n", path );
      return;
    }
  }
  if( res )
  {
    if( ( flags & SHELL_F_SILENT ) == 0 )
      printf( "WARNING: unable to remove %s\n", path );
  }
  else
    printf( "Removed %s\n", path );
}

static int shellh_rm_walkdir_cb( const char *path, const struct dm_dirent *pent, void *pdata, int info )
{
  u8 *pflags = ( u8* )pdata;
  char *tmp = NULL;
  int res = 1;

  switch( info )
  {
    case CMN_FS_INFO_BEFORE_READDIR:
      goto done;

    case CMN_FS_INFO_INSIDE_READDIR:
      if( ( tmp = cmn_fs_path_join( path, pent->fname, NULL ) ) == NULL )
      {
        printf( "Not enough memory.\n" );
        goto done_err;
      }
      if( cmn_fs_get_type( tmp ) == CMN_FS_TYPE_FILE )
        shellh_rm_one( tmp, *pflags );
      goto done;

    case CMN_FS_INFO_OPENDIR_FAILED:
      printf( "ERROR: unable to read directory '%s', aborting.\n", path );
      goto done_err;

    case CMN_FS_INFO_DIRECTORY_DONE:
      if( ( *pflags & SHELL_F_RECURSIVE ) && !cmn_fs_is_root_dir( path ) )
        shellh_rm_one( path, *pflags | SHELL_F_SILENT );
      goto done;

    default:
      goto done;
  }
done_err:
  res = 0;
done:
  if( tmp )
    free( tmp );
  return res;
}

void shell_adv_rm( int argc, char **argv )
{ 
  const char *fmask = NULL;
  unsigned i, flags = 0;
  int masktype;

  if( argc < 2 )
  {
     SHELL_SHOW_HELP( rm );
     return;
  }
  for( i = 1; i < argc; i ++ )
  {
    if( argv[ i ][ 0 ] == '/' )
    {
      if( !fmask )
        fmask = argv[ i ];
      else
        printf( "Warning: ignoring argument '%s'\n", argv[ i ] );
    }
    else if( !strcmp( argv[ i ], "-R" ) )
      flags |= SHELL_F_RECURSIVE;
    else if( !strcmp( argv[ i ], "-c" ) )
      flags |= SHELL_F_ASK_CONFIRMATION;
    else if( !strcmp( argv[ i ], "-s" ) )
      flags |= SHELL_F_SIMULATE_ONLY;
    else
      printf( "Warning: ignoring argument '%s'\n", argv[ i ] );
  }
  if( !fmask )
  {
    printf( "rm target not specified.\n" );
    return;
  }
  masktype = cmn_fs_get_type( fmask );
  if( masktype == CMN_FS_TYPE_FILE )
    shellh_rm_one( fmask, flags );
  else if( masktype == CMN_FS_TYPE_ERROR || masktype == CMN_FS_TYPE_FILE_NOT_FOUND || masktype == CMN_FS_TYPE_DIR_NOT_FOUND || masktype == CMN_FS_TYPE_UNKNOWN_NOT_FOUND )
    printf( "Invalid argument '%s'.\n", fmask );
  else if( masktype == CMN_FS_TYPE_DIR && ( ( flags & SHELL_F_RECURSIVE ) == 0 ) )
    printf( "'%s': unable to remove directory (use '-R').\n", fmask );
  else
    cmn_fs_walkdir( fmask, shellh_rm_walkdir_cb, &flags, flags & SHELL_F_RECURSIVE );
}

#else // #ifdef BUILD_ADVANCED_SHELL

const char shell_help_rm[] = "";
const char shell_help_summary_rm[] = "";

void shell_adv_rm( int argc, char **argv )
{
  shellh_not_implemented_handler( argc, argv );
}

#endif // #ifdef BUILD_ADVANCED_SHELL

