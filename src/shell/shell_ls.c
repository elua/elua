// Shell: 'ls' implementation

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

// State for walkdir
typedef struct
{
  u32 dir_total;
  u32 total;
  u8 ndirs;
} SHELL_LS_STATE;

const char shell_help_ls[] = "[<path>] [-R]\n"
  "  [<path>]: path to list.\n"
  "  [-R]: recursive\n";
const char shell_help_summary_ls[] = "lists files and directories";

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

    case CMN_FS_INFO_AFTER_CLOSEDIR:
      printf( "Total on %s: %u bytes\n\n", path, ( unsigned )ps->dir_total );
      ps->total += ps->dir_total;
      break;

    case CMN_FS_INFO_OPENDIR_FAILED:
      printf( "WARNING: unable to open %s\n", path );
      break;
  }
  return 1;
}

void shell_ls( int argc, char **argv )
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
      printf( "Warning: ignoring argument '%s' of ls\n", argv[ i ] );
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

#else // #ifdef BUILD_ADVANCED_SHELL

const char shell_help_ls[] = "list files and directories";
const char shell_help_summary_ls[] = "lists files and directories";

void shell_ls( int argc, char **argv )
{
  const DM_INSTANCE_DATA *pinst;
  unsigned dev, i;
  DM_DIR *d;
  struct dm_dirent *ent;
  u32 total;

  ( void )argc;
  ( void )argv;
  // Iterate through all devices, looking for the ones that can do "opendir"
  for( dev = 0; dev < dm_get_num_devices(); dev ++ )
  {
    pinst = dm_get_instance_at( dev );
    if( pinst->pdev->p_opendir_r == NULL || pinst->pdev->p_readdir_r == NULL || pinst->pdev->p_closedir_r == NULL )
      continue;
    d = dm_opendir( pinst->name );
    if( d )
    {
      total = 0;
      printf( "\n%s", pinst->name );
      while( ( ent = dm_readdir( d ) ) != NULL )
      {
        printf( "\n%s", ent->fname );
        for( i = strlen( ent->fname ); i <= DM_MAX_FNAME_LENGTH; i++ )
          printf( " " );
        if( ent->flags & DM_DIRENT_FLAG_DIR )
          printf( "<DIR>" );
        else
        {
          printf( "%u bytes", ( unsigned )ent->fsize );
          total = total + ent->fsize;
        }
      }
      printf( "\n\nTotal on %s: %u bytes\n", pinst->name, ( unsigned )total );
      dm_closedir( d );
    }
  }
  printf( "\n" );
}


#endif // #ifdef BUILD_ADVANCED_SHELL

