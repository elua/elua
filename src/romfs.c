// Filesystem implementation
#include "romfs.h"
#include <string.h>
#include <errno.h>
#include "romfiles.h"
#include <stdio.h>
#include "ioctl.h"
#include <fcntl.h>
#include "platform.h"
#ifdef ELUA_SIMULATOR
#include "hostif.h"
#endif
#include "platform_conf.h"
#if defined( BUILD_ROMFS ) || defined( BUILD_WOFS )

#define TOTAL_MAX_FDS   8
// DO NOT CHANGE THE ROMFS ALIGNMENT.
// UNLESS YOU _LIKE_ TO WATCH THE WORLD BURN.
#define ROMFS_ALIGN     4

#define fsmin( x , y ) ( ( x ) < ( y ) ? ( x ) : ( y ) )

static FD fd_table[ TOTAL_MAX_FDS ];
static int romfs_num_fd;
#ifdef ELUA_CPU_LINUX
static int wofs_sim_fd;
#define WOFS_FNAME    "wofs.img"
#define WOFS_SIZE     (256 * 1024)
#endif

#define WOFS_END_MARKER_CHAR  0xFF
#define WOFS_DEL_FIELD_SIZE   ( ROMFS_ALIGN )
#define WOFS_FILE_DELETED     0xAA

// Length of the 'file size' field for both ROMFS/WOFS
#define ROMFS_SIZE_LEN        4

static int romfs_find_empty_fd(void)
{
  int i;
  
  for( i = 0; i < TOTAL_MAX_FDS; i ++ )
    if( fd_table[ i ].baseaddr == 0xFFFFFFFF &&
        fd_table[ i ].offset == 0xFFFFFFFF &&
        fd_table[ i ].size == 0xFFFFFFFF )
      return i;
  return -1;
}

static void romfs_close_fd( int fd )
{
  memset( fd_table + fd, 0xFF, sizeof( FD ) );
  fd_table[ fd ].flags = 0;
}

// Helper function: read a byte from the FS
static u8 romfsh_read8( u32 addr, const FSDATA *pfs )
{
  u8 temp;
  if( pfs->flags & ROMFS_FS_FLAG_DIRECT )
    return pfs->pbase[ addr ];
  pfs->readf( &temp, addr, 1, pfs );
  return temp;
}

// Helper function: return 1 if PFS reffers to a WOFS, 0 otherwise
static int romfsh_is_wofs( const FSDATA* pfs )
{
  return ( pfs->flags & ROMFS_FS_FLAG_WO ) != 0;
}

// Open the given file, returning one of FS_FILE_NOT_FOUND, FS_FILE_ALREADY_OPENED
// or FS_FILE_OK
static u8 romfs_open_file( const char* fname, FD* pfd, FSDATA *pfs, u32 *plast, u32 *pnameaddr )
{
  u32 i, j, n;
  char fsname[ DM_MAX_FNAME_LENGTH + 1 ];
  u32 fsize;
  int is_deleted;
  
  // Look for the file
  i = 0;
  while( 1 )
  {
    if( romfsh_read8( i, pfs ) == WOFS_END_MARKER_CHAR )
    {
      *plast = i;
      return FS_FILE_NOT_FOUND;
    }
    // Read file name
    n = i;
    for( j = 0; j < DM_MAX_FNAME_LENGTH; j ++ )
    {
      fsname[ j ] = romfsh_read8( i + j, pfs );
      if( fsname[ j ] == 0 )
         break;
    }
    // ' i + j' now points at the '0' byte
    j = i + j + 1;
    // Round to a multiple of ROMFS_ALIGN
    j = ( j + ROMFS_ALIGN - 1 ) & ~( ROMFS_ALIGN - 1 );
    // WOFS has an additional WOFS_DEL_FIELD_SIZE bytes before the size as an indication for "file deleted"
    if( romfsh_is_wofs( pfs ) )
    {
      is_deleted = romfsh_read8( j, pfs ) == WOFS_FILE_DELETED;
      j += WOFS_DEL_FIELD_SIZE;
    }
    else
      is_deleted = 0;
    // And read the size
    fsize = romfsh_read8( j, pfs ) + ( romfsh_read8( j + 1, pfs ) << 8 );
    fsize += ( romfsh_read8( j + 2, pfs ) << 16 ) + ( romfsh_read8( j + 3, pfs ) << 24 );
    j += ROMFS_SIZE_LEN;
    if( !strncasecmp( fname, fsname, DM_MAX_FNAME_LENGTH ) && !is_deleted )
    {
      // Found the file
      pfd->baseaddr = j;
      pfd->offset = 0;
      pfd->size = fsize;
      if( pnameaddr )
        *pnameaddr = n;
      return FS_FILE_OK;
    }
    // Move to next file
    i = j + fsize;
    // On WOFS, all file names must begin at a multiple of ROMFS_ALIGN
    if( romfsh_is_wofs( pfs ) )
      i = ( i + ROMFS_ALIGN - 1 ) & ~( ROMFS_ALIGN - 1 );
  }
  *plast = 0;
  return FS_FILE_NOT_FOUND;
}

static int romfs_open_r( struct _reent *r, const char *path, int flags, int mode, void *pdata )
{
  FD tempfs;
  int i;
  FSDATA *pfsdata = ( FSDATA* )pdata;
  int must_create = 0;
  int exists;
  u8 lflags = ROMFS_FILE_FLAG_READ;
  u32 firstfree, nameaddr;

  if( romfs_num_fd == TOTAL_MAX_FDS )
  {
    r->_errno = ENFILE;
    return -1;
  }
  // Does the file exist?
  exists = romfs_open_file( path, &tempfs, pfsdata, &firstfree, &nameaddr ) == FS_FILE_OK;
  // Now interpret "flags" to set file flags and to check if we should create the file
  if( flags & O_CREAT )
  {
    // If O_CREAT is specified with O_EXCL and the file already exists, return with error
    if( ( flags & O_EXCL ) && exists )
    {
      r->_errno = EEXIST;
      return -1;
    }
    // Otherwise create the file if it does not exist
    must_create = !exists;
  }
  if( ( flags & O_TRUNC ) && ( flags & ( O_WRONLY | O_RDWR ) ) && exists )
  {
    // The file exists, but it must be truncated
    // In the case of WOFS, this effectively means "create a new file"
    must_create = 1;
  }
  // ROMFS can't create files
  if( must_create && ( ( pfsdata->flags & ROMFS_FS_FLAG_WO ) == 0 ) )
  {
    r->_errno = EROFS;
    return -1;
  }
  // Decode access mode
  if( flags & O_WRONLY )
    lflags = ROMFS_FILE_FLAG_WRITE;
  else if( flags & O_RDWR )
    lflags = ROMFS_FILE_FLAG_READ | ROMFS_FILE_FLAG_WRITE;
  if( flags & O_APPEND )
    lflags |= ROMFS_FILE_FLAG_APPEND;
  // If a write access is requested when the file must NOT be created, this
  // is an error
  if( ( lflags & ( ROMFS_FILE_FLAG_WRITE | ROMFS_FILE_FLAG_APPEND ) ) && !must_create )
  {
    r->_errno = EACCES;
    return -1;
  }
  if( ( lflags & ( ROMFS_FILE_FLAG_WRITE | ROMFS_FILE_FLAG_APPEND ) ) && romfs_fs_is_flag_set( pfsdata, ROMFS_FS_FLAG_WRITING ) )
  {
    // At most one file can be opened in write mode at any given time on WOFS
    r->_errno = EROFS;
    return -1;
  }
  // Do we need to create the file ?
  if( must_create )
  {
    if( exists )
    {
      // Invalidate the file first by changing WOFS_DEL_FIELD_SIZE bytes before
      // the file length to WOFS_FILE_DELETED
      u8 tempb[] = { WOFS_FILE_DELETED, 0xFF, 0xFF, 0xFF };
      pfsdata->writef( tempb, tempfs.baseaddr - ROMFS_SIZE_LEN - WOFS_DEL_FIELD_SIZE, WOFS_DEL_FIELD_SIZE, pfsdata );
    }
    // Find the last available position by asking romfs_open_file to look for a file
    // with an invalid name
    romfs_open_file( "\1", &tempfs, pfsdata, &firstfree, NULL );
    // Is there enough space on the FS for another file?
    if( pfsdata->max_size - firstfree + 1 < strlen( path ) + 1 + WOFS_MIN_NEEDED_SIZE + WOFS_DEL_FIELD_SIZE )
    {
      r->_errno = ENOSPC;
      return -1;
    }

    // Make sure we can get a file descriptor before writing
    if( ( i = romfs_find_empty_fd() ) < 0 )
    {
      r->_errno = ENFILE;
      return -1;
    }

    // Write the name of the file
    pfsdata->writef( path, firstfree, strlen( path ) + 1, pfsdata );
    firstfree += strlen( path ) + 1; // skip over the name
    // Align to a multiple of ROMFS_ALIGN
    firstfree = ( firstfree + ROMFS_ALIGN - 1 ) & ~( ROMFS_ALIGN - 1 );
    firstfree += ROMFS_SIZE_LEN + WOFS_DEL_FIELD_SIZE; // skip over the size and the deleted flags area
    tempfs.baseaddr = firstfree;
    tempfs.offset = tempfs.size = 0;
    // Set the "writing" flag on the FS to indicate that there is a file opened in write mode
    romfs_fs_set_flag( pfsdata, ROMFS_FS_FLAG_WRITING );
  }
  else // File must exist (and was found in the previous 'romfs_open_file' call)
  {
    if( !exists )
    {
      r->_errno = ENOENT;
      return -1;
    }

    if( ( i = romfs_find_empty_fd() ) < 0 )
    {
      r->_errno = ENFILE;
      return -1;
    }
  }
  // Copy the descriptor information
  tempfs.flags = lflags;
  memcpy( fd_table + i, &tempfs, sizeof( FD ) );
  romfs_num_fd ++;
  return i;
}

static int romfs_close_r( struct _reent *r, int fd, void *pdata )
{
  FD* pfd = fd_table + fd;
  FSDATA *pfsdata = ( FSDATA* )pdata;
  u8 temp[ ROMFS_SIZE_LEN ];

  if( pfd->flags & ( ROMFS_FILE_FLAG_WRITE | ROMFS_FILE_FLAG_APPEND ) )
  {
    // Write back the size
    temp[ 0 ] = pfd->size & 0xFF;
    temp[ 1 ] = ( pfd->size >> 8 ) & 0xFF;
    temp[ 2 ] = ( pfd->size >> 16 ) & 0xFF;
    temp[ 3 ] = ( pfd->size >> 24 ) & 0xFF;
    pfsdata->writef( temp, pfd->baseaddr - ROMFS_SIZE_LEN, ROMFS_SIZE_LEN, pfsdata );
    // Clear the "writing" flag on the FS instance to allow other files to be opened
    // in write mode
    romfs_fs_clear_flag( pfsdata, ROMFS_FS_FLAG_WRITING );
  }
  romfs_close_fd( fd );
  romfs_num_fd --;
  return 0;
}

static _ssize_t romfs_write_r( struct _reent *r, int fd, const void* ptr, size_t len, void *pdata )
{
  FD* pfd = fd_table + fd;
  FSDATA *pfsdata = ( FSDATA* )pdata;

  if( ( pfd->flags & ( ROMFS_FILE_FLAG_WRITE | ROMFS_FILE_FLAG_APPEND ) ) == 0 )
  {
    r->_errno = EINVAL;
    return -1;
  }
  // Append mode: set the file pointer to the end
  if( pfd->flags & ROMFS_FILE_FLAG_APPEND )
    pfd->offset = pfd->size;
  // Only write at the end of the file!
  if( pfd->offset != pfd->size )
    return 0;
  // Check if we have enough space left on the device. Always keep 1 byte for the final 0xFF
  // and ROMFS_ALIGN - 1 bytes for aligning the contents of the file data in the worst case
  // scenario (so ROMFS_ALIGN bytes in total)
  if( pfd->baseaddr + pfd->size + len > pfsdata->max_size - ROMFS_ALIGN )
    len = pfsdata->max_size - ( pfd->baseaddr + pfd->size ) - ROMFS_ALIGN;
  pfsdata->writef( ptr, pfd->offset + pfd->baseaddr, len, pfsdata );
  pfd->offset += len;
  pfd->size += len;
  return len;
}

static _ssize_t romfs_read_r( struct _reent *r, int fd, void* ptr, size_t len, void *pdata )
{
  FD* pfd = fd_table + fd;
  long actlen = fsmin( len, pfd->size - pfd->offset );
  FSDATA *pfsdata = ( FSDATA* )pdata;

  if( ( pfd->flags & ROMFS_FILE_FLAG_READ ) == 0 )
  {
    r->_errno = EBADF;
    return -1;
  }
  if( pfsdata->flags & ROMFS_FS_FLAG_DIRECT )
    memcpy( ptr, pfsdata->pbase + pfd->offset + pfd->baseaddr, actlen );
  else
    actlen = pfsdata->readf( ptr, pfd->offset + pfd->baseaddr, actlen, pfsdata );
  pfd->offset += actlen;
  return actlen;
}

// lseek
static off_t romfs_lseek_r( struct _reent *r, int fd, off_t off, int whence, void *pdata )
{
  FD* pfd = fd_table + fd;
  u32 newpos = 0;
  
  switch( whence )
  {
    case SEEK_SET:
      newpos = off;
      break;
      
    case SEEK_CUR:
      newpos = pfd->offset + off;
      break;
      
    case SEEK_END:
      newpos = pfd->size + off;
      break;
      
    default:
      return -1;
  }    
  if( newpos > pfd->size )
    return -1;
  pfd->offset = newpos;
  return newpos;
}

// Directory operations
static u32 romfs_dir_data = 0;

// opendir
static void* romfs_opendir_r( struct _reent *r, const char* dname, void *pdata )
{
  if( !dname || strlen( dname ) == 0 || ( strlen( dname ) == 1 && !strcmp( dname, "/" ) ) )
  {
    romfs_dir_data = 0;
    return &romfs_dir_data;
  }
  return NULL;
}

// readdir
extern struct dm_dirent dm_shared_dirent;
extern char dm_shared_fname[ DM_MAX_FNAME_LENGTH + 1 ];
static struct dm_dirent* romfs_readdir_r( struct _reent *r, void *d, void *pdata )
{
  u32 off = *( u32* )d;
  struct dm_dirent *pent = &dm_shared_dirent;
  unsigned j;
  FSDATA *pfsdata = ( FSDATA* )pdata;
  int is_deleted;
 
  while( 1 )
  {
    if( romfsh_read8( off, pfsdata ) == WOFS_END_MARKER_CHAR )
      return NULL;
    j = 0;
    while( ( dm_shared_fname[ j ++ ] = romfsh_read8( off ++, pfsdata ) ) != '\0' );
    pent->fname = dm_shared_fname;
    off = ( off + ROMFS_ALIGN - 1 ) & ~( ROMFS_ALIGN - 1 );
    if( romfsh_is_wofs( pfsdata ) )
    {
      is_deleted = romfsh_read8( off, pfsdata ) == WOFS_FILE_DELETED;
      off += WOFS_DEL_FIELD_SIZE;
    }
    else
      is_deleted = 0;
    pent->fsize = romfsh_read8( off, pfsdata ) + ( romfsh_read8( off + 1, pfsdata ) << 8 );
    pent->fsize += ( romfsh_read8( off + 2, pfsdata ) << 16 ) + ( romfsh_read8( off + 3, pfsdata ) << 24 );
    pent->ftime = 0;
    pent->flags = 0;
    off += ROMFS_SIZE_LEN;
    off += pent->fsize;
    if( romfsh_is_wofs( pfsdata ) )
      off = ( off + ROMFS_ALIGN - 1 ) & ~( ROMFS_ALIGN - 1 );
    if( !is_deleted )
      break;
  }
  *( u32* )d = off;
  return pent;
}

// closedir
static int romfs_closedir_r( struct _reent *r, void *d, void *pdata )
{
  *( u32* )d = 0;
  return 0;
}

// getaddr
static const char* romfs_getaddr_r( struct _reent *r, int fd, void *pdata )
{
  FD* pfd = fd_table + fd;
  FSDATA *pfsdata = ( FSDATA* )pdata;

  if( pfsdata->flags & ROMFS_FS_FLAG_DIRECT )
    return ( const char* )pfsdata->pbase + pfd->baseaddr;
  else
    return NULL;
}

// ****************************************************************************
// Our ROMFS device descriptor structure
// These functions apply to both ROMFS and WOFS

static const DM_DEVICE romfs_device = 
{
  romfs_open_r,         // open
  romfs_close_r,        // close
  romfs_write_r,        // write
  romfs_read_r,         // read
  romfs_lseek_r,        // lseek
  romfs_opendir_r,      // opendir
  romfs_readdir_r,      // readdir
  romfs_closedir_r,     // closedir
  romfs_getaddr_r,      // getaddr
  NULL,                 // mkdir
  NULL,                 // unlink
  NULL,                 // rmdir
  NULL                  // rename
};

// ****************************************************************************
// ROMFS instance descriptor

static const FSDATA romfs_fsdata =
{
  ( u8* )romfiles_fs,
  ROMFS_FS_FLAG_DIRECT,
  NULL,
  NULL,
  sizeof( romfiles_fs )
};

// ****************************************************************************
// WOFS functions and instance descriptor for the simulator (testing)

#if defined( ELUA_CPU_LINUX ) && defined( BUILD_WOFS )
static u32 sim_wofs_read( void *to, u32 fromaddr, u32 size, const void *pdata )
{
  hostif_lseek( wofs_sim_fd, ( long )fromaddr, SEEK_SET );
  return hostif_read( wofs_sim_fd, to, size );
}

static u32 sim_wofs_write( const void *from, u32 toaddr, u32 size, const void *pdata )
{
  hostif_lseek( wofs_sim_fd, ( long )toaddr, SEEK_SET );
  return hostif_write( wofs_sim_fd, from, size );
}

// This must NOT be a const!
static FSDATA wofs_sim_fsdata =
{
  NULL,
  ROMFS_FS_FLAG_WO,
  sim_wofs_read,
  sim_wofs_write,
  WOFS_SIZE
};

// WOFS formatting function
// Returns 1 if OK, 0 for error
int wofs_format( void )
{
  unsigned i;

  hostif_lseek( wofs_sim_fd, 0, SEEK_SET );
  u8 temp = WOFS_END_MARKER_CHAR;
  for( i = 0; i < WOFS_SIZE; i ++ )
    hostif_write( wofs_sim_fd, &temp, 1 );
  return 1;
}

#endif // #ifdef ELUA_CPU_LINUX

// ****************************************************************************
// WOFS functions and instance descriptor for real hardware

#if defined( BUILD_WOFS ) && !defined( ELUA_CPU_LINUX )
static u32 sim_wofs_write( const void *from, u32 toaddr, u32 size, const void *pdata )
{
  const FSDATA *pfsdata = ( const FSDATA* )pdata;

  toaddr += ( u32 )pfsdata->pbase;
  return platform_flash_write( from, toaddr, size );
}

// This must NOT be a const!
static FSDATA wofs_fsdata =
{
  NULL,
  ROMFS_FS_FLAG_WO | ROMFS_FS_FLAG_DIRECT,
  NULL,
  sim_wofs_write,
  0
};

// WOFS formatting function
// Returns 1 if OK, 0 for error
int wofs_format( void )
{
  u32 sect_first, sect_last;
  FD tempfd;

  platform_flash_get_first_free_block_address( &sect_first );
  // Get the first free address in WOFS. We use this address to compute the last block that we need to
  // erase, instead of simply erasing everything from sect_first to the last Flash page. 
  romfs_open_file( "\1", &tempfd, &wofs_fsdata, &sect_last, NULL );
  sect_last = platform_flash_get_sector_of_address( sect_last + ( u32 )wofs_fsdata.pbase );
  while( sect_first <= sect_last )
    if( platform_flash_erase_sector( sect_first ++ ) == PLATFORM_ERR )
      return 0;
  return 1;
}

#endif // #ifdef BUILD_WOFS

// Initialize both ROMFS and WOFS as needed
int romfs_init( void )
{
  unsigned i;

  for( i = 0; i < TOTAL_MAX_FDS; i ++ )
  {
    memset( fd_table + i, 0xFF, sizeof( FD ) );
    fd_table[ i ].flags = 0;
  }
#if defined( ELUA_CPU_LINUX ) && defined( BUILD_WOFS )
  // Initialize and register WOFS for the simulator
  wofs_sim_fd = hostif_open( WOFS_FNAME, 2, 0666 ); // try to open directly first
  if( -1 == wofs_sim_fd )
  {
    wofs_sim_fd = hostif_open( WOFS_FNAME, 66, 0666) ; // 66 == O_RDWR | O_CREAT
    u8 temp = WOFS_END_MARKER_CHAR;
    for( i = 0; i < WOFS_SIZE; i ++ )
      hostif_write( wofs_sim_fd, &temp, 1 );
    printf( "SIM_WOFS: creating WOFS file\n" );
    hostif_close( wofs_sim_fd );
    wofs_sim_fd = hostif_open( WOFS_FNAME, 2, 0666 );
  }
  dm_register( "/wo", ( void* )&wofs_sim_fsdata, &romfs_device );
#endif // #if defined( ELUA_CPU_LINUX ) && defined( BUILD_WOFS )
#if defined( BUILD_WOFS ) && !defined( ELUA_CPU_LINUX )
  // Get the start address and size of WOFS and register it
  wofs_fsdata.pbase = ( u8* )platform_flash_get_first_free_block_address( NULL );
  wofs_fsdata.max_size = INTERNAL_FLASH_SIZE - ( ( u32 )wofs_fsdata.pbase - INTERNAL_FLASH_START_ADDRESS );
  dm_register( "/wo", &wofs_fsdata, &romfs_device );
#endif // ifdef BUILD_WOFS
#ifdef BUILD_ROMFS
  // Register the ROM filesystem
  dm_register( "/rom", ( void* )&romfs_fsdata, &romfs_device );
#endif // #ifdef BUILD_ROMFS
  return 0;
}

#else // #if defined( BUILD_ROMFS ) || defined( BUILD_WOFS )

int romfs_init( void )
{
  return dm_register( NULL, NULL, NULL );
}

#endif // #if defined( BUILD_ROMFS ) || defined( BUILD_WOFS )

