// Common file system related operations

#include "common.h"
#include <stdio.h>
#include "devman.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define SAFE_CLOSEDIR( dir ) \
  do { \
    dm_closedir( dir ); \
    dir = NULL; \
  } while( 0 )

// Returns 1 if 'str' matches the given pattern, 0 otherwise
// '?' matches a single char
// "*" matches one or more chars, NON-GREEDY
static int cmn_pattern_matches( const char *str, const char *pattern )
{
  int pidx, sidx;

  pidx = sidx = 0;
  while( pattern[ pidx ] != 0 )
  {
    if( pattern[ pidx ] == '*' )
    {
      pidx ++;
      // Eat all '?' chars after the '*'
      while( pattern[ pidx ] == '?' )
        pidx ++;
      if( pattern[ pidx ] == 0 ) // '*' in the last position matches everything
        return 1;
      while( str[ sidx ] != pattern[ pidx ] && str[ sidx ] != 0 )
        sidx ++;
      if( str[ sidx ++ ] == 0 )
        return 0;
    }
    else if( pattern[ pidx ] == '?' )
    {
      if( str[ sidx ++ ] == 0 )
        return 0;
    }
    else
    {
      if( pattern[ pidx ] != str[ sidx ++ ] )
        return 0;
    }
    pidx ++;
  }
  return str[ sidx ] == 0;
}

// Splits a path into "directory" and "filemask" name
// Returns a pointer to the directory name. The pointer is dynamically
// allocated and it MUST be freed by the caller!
// Also returns a pointer to the file mask in "pmask" as a side effect
// Returns NULL on error
char *cmn_fs_split_path( const char *path, const char **pmask )
{
  static const char *all_mask = "*";
  FILE *fp;

  if( !path || strlen( path ) < 2 || path[ 0 ] != '/' )
    return NULL;
  // /fs -> always a directory ('/'), mask is *
  if( strchr( path + 1, '/' ) == NULL )
  {
    *pmask = all_mask;
    return strdup( path );
  }
  // /fs/dir1/dir2/dirn/ -> always a directory (note the final '/' ), mask is '*'
  if( path[ strlen( path ) - 1 ] == '/' )
  {
    *pmask = all_mask;
    return strndup( path, strlen( path ) - 1 );
  }
  // At this point, we don't know if 'path' reffers to a file or to a directory
  // So try to open it. If it can be opened, it is a file. Otherwise it is a directory.
  // But first check for '*' or '?' chars in path (which means it is definitely a file spec)
  *pmask = strrchr( path, '/' ) + 1;
  if( strchr( path, '*' ) || strchr( path, '?' ) )
    return strndup( path, *pmask - path - 1 );
  if( ( fp = fopen( path, "r" ) ) != NULL )
  {
    fclose( fp );
    return strndup( path, *pmask - path - 1 );
  }
  // It is a path, so return it as-is
  *pmask = all_mask;
  return strdup( path );
}

// Returns the type of the path given as argument
// This can be either CMN_FS_TYPE_DIR, CMN_FS_TYPE_FILE or CMN_FS_TYPE_PATTERN
// Note that CMN_FS_TYPE_DIR is NOT actually checked. A name with a '/' in the
// last position or a name with a single '/' (for example /rom/ and /rom respectively)
// is assumed to be a directory. Otherwise, if the 'path' is not a mask and also
// not a regular file, it is assumed to be a directory
int cmn_fs_get_type( const char *path )
{  
  FILE *fp;
  DM_DIR *d;

  if( !path || strlen( path ) < 2 || path[ 0 ] != '/' )
    return CMN_FS_TYPE_ERROR;
  // /fs -> always a directory ('/'), mask is *
  if( strchr( path + 1, '/' ) == NULL )
    return CMN_FS_TYPE_DIR;
  // /fs/dir1/dir2/dirn/ -> always a directory (note the final '/' ), mask is '*'
  if( path[ strlen( path ) - 1 ] == '/' )
  {
    if( ( d = dm_opendir( path ) ) == NULL )
      return CMN_FS_TYPE_DIR_NOT_FOUND;
    dm_closedir( d );
    return CMN_FS_TYPE_DIR;
  }
  // At this point, we don't know if 'path' reffers to a file or to a directory
  // So try to open it. If it can be opened, it is a file. Otherwise it is a directory.
  // But first check for '*' or '?' chars in path (which means it is definitely a file spec)
  if( strchr( path, '*' ) || strchr( path, '?' ) )
    return CMN_FS_TYPE_PATTERN;
  if( ( fp = fopen( path, "r" ) ) != NULL )
  {
    fclose( fp );
    return CMN_FS_TYPE_FILE;
  }
  if( ( d = dm_opendir( path ) ) == NULL )
    return CMN_FS_TYPE_UNKNOWN_NOT_FOUND;
  dm_closedir( d );
  return CMN_FS_TYPE_DIR;
}

// Join the given elements in a single PATH
// The memory for the final buffer is dynamically allocated and MUST be 
// freed by the caller
char *cmn_fs_path_join( const char *first, ... )
{
  unsigned total = 0;
  va_list ap;
  const char *tmp = first;
  char *res;

  va_start( ap, first );
  // Get total length first
  while( tmp )
  {
    if( strlen( tmp ) > 0 )
    {
      total += strlen( tmp ) + ( lastchar( tmp ) == '/' ? 0 : 1 );
      if( firstchar( tmp ) == '/' )
        total --;
    }
    tmp = va_arg( ap, const char* );
  }
  va_end( ap );
  if( total == 0 || ( ( res = ( char* )malloc( total + 2 ) ) == NULL ) )
    return NULL;
  strcpy( res, "/" );
  va_start( ap, first );
  tmp = first;
  while( tmp )
  {
    if( strlen( tmp ) > 0 )
    {
      strcat( res, tmp + ( firstchar( tmp ) == '/' ? 1 : 0 ) );
      if( lastchar( res ) != '/' )
        strcat( res, "/" );
    }
    tmp = va_arg( ap, const char* );
  }
  res[ strlen( res ) - 1 ] = '\0';
  va_end( ap );
  return res;
}

// This is a generic file/directory walker. It is used by all shell
// operations that have something to do with files (ls, cp, rm...)
// It receives an initial path, which may contain also a pattern specification
// (see cmn_pattern_matches above). If "recursive" is true, it will
// recursively find all files and directories underneath 'path' that match
// the optional pattern in 'path'. When a file is called, the "callback" is
// called. The callback is a function with the following signature:
//   void walker_cb( const char *path, const struct dm_entry *ent, void *pdata, int info );
// 'path' is the full path of the file/directory found by the walker
// 'ent' is the directory entry (as defined in devman.h)
// 'pdata' is the callback state, passed to the callback function exactly
//   as given to the walker function.
// 'info' gives more information about why/where the callback was called:
//   CMN_FS_INFO_BEFORE_READDIR
//   CMN_FS_INFO_INSIDE_READDIR
//   CMN_FS_INFO_AFTER_CLOSEDIR
//   CMN_FS_INFO_DIRECTORY_DONE
//   CMN_FS_INFO_MEMORY_ERROR -> called after a memory allocation failed
//   CMN_FS_INFO_OPENDIR_FAILED
//   CMN_FS_INFO_READDIR_FAILED
// The callback can return 0 (stop walking the directory and calling the callback)
// or 1 (keep on calling the callback). If a 0 was returned, this will also be the
// result returned by the walker, otherwise 1 is returned
static int cmn_fs_actual_walkdir( const char *path, const char *pattern, p_cmn_fs_walker_cb cb, void *pdata, int recursive )
{
  DM_DIR *d;
  struct dm_dirent *ent;
  int hasdirs, isdir;
  char *fullname;

  if( ( d = dm_opendir( path ) ) != NULL )
  {
    if( cb( path, NULL, pdata, CMN_FS_INFO_BEFORE_READDIR ) == 0 )
      goto abort;
    while( ( ent = dm_readdir( d ) ) != NULL )
    {
      isdir = ( ent->flags & DM_DIRENT_FLAG_DIR ) != 0;
      if( !cmn_pattern_matches( ent->fname, pattern ) )
        continue;
      if( cb( path, ent, pdata, CMN_FS_INFO_INSIDE_READDIR ) == 0 )
        goto abort;
      if( isdir )
        hasdirs = 1;
    }
    SAFE_CLOSEDIR( d );
    if( cb( path, ent, pdata, CMN_FS_INFO_AFTER_CLOSEDIR ) == 0 )
      goto abort;
    if( recursive && hasdirs )
    {
      if( ( d = dm_opendir( path ) ) != NULL )
      {
        while( ( ent = dm_readdir( d ) ) != NULL )
        {
          if( ent->flags & DM_DIRENT_FLAG_DIR )
          {
            if( ( fullname = cmn_fs_path_join( path, ent->fname, NULL ) ) != NULL )
            {
              hasdirs = cmn_fs_actual_walkdir( fullname, pattern, cb, pdata, 1 );
              free( fullname );
              if( hasdirs == 0 )
                goto abort;
            }
            else
              if( cb( path, NULL, pdata, CMN_FS_INFO_MEMORY_ERROR ) == 0 )
                goto abort;
          }
        }
        SAFE_CLOSEDIR( d );
      }
      else
        if( cb( path, NULL, pdata, CMN_FS_INFO_OPENDIR_FAILED ) == 0 )
          return 0;
    }
    if( cb( path, ent, pdata, CMN_FS_INFO_DIRECTORY_DONE ) == 0 )
      goto abort;
  }
  else
    if( cb( path, NULL, pdata, CMN_FS_INFO_OPENDIR_FAILED ) == 0 )
      return 0;
  return 1;
abort:
  if( d )
    dm_closedir( d );
  return 0;
}

int cmn_fs_walkdir( const char *path, p_cmn_fs_walker_cb cb, void *pdata, int recursive )
{
  char *actpath;
  const char *pattern;

  if( ( actpath = cmn_fs_split_path( path, &pattern ) ) == NULL )
    return 0;
  cmn_fs_actual_walkdir( actpath, pattern, cb, pdata, recursive );
  free( actpath );
  return 1;
}

int cmn_fs_is_root_dir( const char *path )
{
  if( !path )
    return 0;
  if( path[ 0 ] != '/' )
    return 0;
  if( ( path = strchr( path + 1, '/' ) ) == NULL )
    return 1;
  if( *( path + 1 ) == 0 )
    return 1;
  return 0;
}

// Check if the directory in the given argument (if applicable)
// can be opened
int cmn_fs_check_directory( const char *path )
{
  DM_DIR *d;

  if( ( d = dm_opendir( path ) ) == NULL )
    return 0;
  dm_closedir( d );
  return 1;
}

