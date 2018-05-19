// Filesystem implementation
#include "romfs.h"
#include <string.h>
#include <errno.h>
#include "romfiles.h"
#include <stdio.h>
#include <stdlib.h>
#include "ioctl.h"
#include <fcntl.h>
#include "platform.h"
#include "platform_conf.h"
#include "niffs.h"
#include "niffs_internal.h"

#if defined( BUILD_NIFFS ) 

//#define NFFS_DBG(...)              printf(__VA_ARGS__)
#ifndef NFFS_DBG
#define NFFS_DBG(...)
#endif

// FIXME: Needs to be adjusted to support non-uniform sector sizes
#define LAST_SECTOR_NUM ( platform_flash_get_num_sectors() - 1 )
#define LAST_SECTOR_END  ( INTERNAL_FLASH_SIZE - INTERNAL_FLASH_START_ADDRESS )

niffs fs;

#define NIFFS_BUF_SIZE           128
#define NIFFS_FILE_DESCS         4
static u8_t buf[NIFFS_BUF_SIZE];
static niffs_file_desc descs[NIFFS_FILE_DESCS];
static u8_t * niffs_pbase;
static u32_t niffs_max_size;
static u32_t niffs_total_sectors;
static u32_t niffs_paged_sectors;
static u32_t niffs_lin_sectors;

static int nffs_open_r( struct _reent *r, const char *path, int flags, int mode, void *pdata )
{
  u8 lflags = 0;
  int ret;
  
  lflags = NIFFS_O_RDONLY;

  //check for read/write flags and overwrite read only if so
  if(flags & O_RDWR) {
    lflags = NIFFS_O_RDWR;
    //printf("-S");
  }
  if(flags & O_WRONLY) {
    lflags = NIFFS_O_WRONLY;
    //printf("-W");
  }

  //add other flags
  if(flags & O_CREAT) {
    lflags |= NIFFS_O_CREAT;
    //printf("-C");
  }
  if(flags & O_TRUNC) {
    lflags |= NIFFS_O_TRUNC;
    //printf("-T");
  }
  if(flags & O_APPEND) {
    lflags |= NIFFS_O_APPEND;
    //printf("-A");
  }
  if(flags & O_EXCL) {
    lflags |= NIFFS_O_EXCL;
    //printf("-E");
  }

  // If file name starts with autorun or lin_, then put it on the linear filesystem
  if ((strncmp("lin_", path, 7) == 0) || (strncmp("autorun", path, 7) == 0)) {
    lflags |= NIFFS_O_LINEAR;
    //printf("-L\n");
  }
  ret = NIFFS_open(&fs, (char *)path, lflags, mode);
  NFFS_DBG("\nN_O:%s,%i,%i,%i,%i\n", path, ret, flags, lflags, mode);
  if(ret < 0)
  {
    r->_errno = ENOENT; //File does not exist
    return -1;
  }
  return ret;
}

static int nffs_close_r( struct _reent *r, int fd, void *pdata )
{
  return NIFFS_close(&fs, fd);
}

static _ssize_t nffs_write_r( struct _reent *r, int fd, const void* ptr, size_t len, void *pdata )
{
  return NIFFS_write(&fs, fd, (void *)ptr, len);
}

static _ssize_t nffs_read_r( struct _reent *r, int fd, void* ptr, size_t len, void *pdata )
{
  return NIFFS_read(&fs, fd, ptr, len);
}

static off_t nffs_lseek_r( struct _reent *r, int fd, off_t off, int whence, void *pdata )
{
  //      Ensure the whence arg here matches NIFFS_SEEK_SET, NIFFS_SEEK_CUR, NIFFS_SEEK_END?
  //      niffs adapts to the posix values so should work (Yes)
  //#define SEEK_SET        0       /* set file offset to offset */
  //#define SEEK_CUR        1       /* set file offset to current plus offset */
  //#define SEEK_END        2       /* set file offset to EOF plus offset */
  //#define NIFFS_SEEK_SET          (0)
  //#define NIFFS_SEEK_CUR          (1)
  //#define NIFFS_SEEK_END          (2)

  int res;

  //Return value will be greater than 0 if file seek occurs, or 0 IE NIFFS_OK if successful but didn't move, and
  //less than zero if an error occured.
  res = NIFFS_lseek(&fs, fd, off, whence);
  if(res >= NIFFS_OK)
  {
    //printf("Nl r:%i fd:%i o:%li w:%i\n", res, fd, off, whence);
    return res;
  } else {
    //printf("Nl FAIL r:%i fd:%i o:%li w:%i\n", res, fd, off, whence);
    return -1;
  }
}

// getaddr
static const char* nffs_getaddr_r( struct _reent *r, int fd, void *pdata )
{
  u8_t * ptrptr;
  u32_t len;
  NIFFS_read_ptr(&fs, fd, &ptrptr, &len);
  //printf("getaddr %p\n", (u32_t *)ptrptr);
  return (char*)ptrptr;
}

// Directory operations
//static u32 romfs_dir_data = 0;

static niffs_DIR niffs_d;
// opendir
static void* nffs_opendir_r( struct _reent *r, const char* dname, void *pdata )
{
  return NIFFS_opendir(&fs, (char *)dname, &niffs_d);
}
// TODO peter no clue what I am doing here, but could perhaps work
extern struct dm_dirent dm_shared_dirent;
extern char dm_shared_fname[ DM_MAX_FNAME_LENGTH + 1 ];
static struct dm_dirent *nffs_readdir_r( struct _reent *r, void *d, void *pdata )
{
  struct dm_dirent *pent = &dm_shared_dirent;
  struct niffs_dirent niffs_ent;
  struct niffs_dirent *p_niffs_ent = &niffs_ent;
  p_niffs_ent = NIFFS_readdir(&niffs_d, p_niffs_ent);
  if (p_niffs_ent) {
    pent->fname = dm_shared_fname;
    strncpy((char *)pent->fname, (char *)p_niffs_ent->name, UMIN(NIFFS_NAME_LEN, DM_MAX_FNAME_LENGTH));
    pent->fsize = p_niffs_ent->size;
    pent->flags = 0; // TODO peter don't know what elua uses this for
    pent->ftime = 0;
    if (p_niffs_ent->type) {
      // TODO peter placeholder
      // 0x00 = paged file
      // 0x01 = linear file
    }
  }
  return p_niffs_ent == NULL ? NULL : pent;
}
/*
NIFFS_readdir
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
    // Check if we're at the end of the FS
    if( romfsh_read8( off, pfsdata ) == WOFS_END_MARKER_CHAR )
      return NULL;

    // Read filename
    j = 0;
    while( ( dm_shared_fname[ j ++ ] = romfsh_read8( off ++, pfsdata ) ) != '\0' );
    pent->fname = dm_shared_fname;

    // Move to next aligned offset after name
    off = ( off + ROMFS_ALIGN - 1 ) & ~( ROMFS_ALIGN - 1 );

    // If WOFS, check if file is marked as deleted
    if( romfsh_is_wofs( pfsdata ) )
    {
      is_deleted = romfsh_read8( off, pfsdata ) == WOFS_FILE_DELETED;
      off += WOFS_DEL_FIELD_SIZE;
    }
    else
      is_deleted = 0;

    // Get file size file and pack into fsize
    pent->fsize = romfsh_read8( off, pfsdata ) + ( romfsh_read8( off + 1, pfsdata ) << 8 );
    pent->fsize += ( romfsh_read8( off + 2, pfsdata ) << 16 ) + ( romfsh_read8( off + 3, pfsdata ) << 24 );
    
    if( (( u64 )off + ( u64 )ROMFS_SIZE_LEN + ( u64 )pent->fsize) > ( u64 )pfsdata->max_size )
    {
      if( romfs_fs_is_flag_set( pfsdata, ROMFS_FS_FLAG_READY_WRITE ) &&
          !( romfs_fs_is_flag_set( pfsdata, ROMFS_FS_FLAG_WRITING ) && ( u64 )pent->fsize == 0xFFFFFFFF ) )
      {
        fprintf(stderr, "[ERROR] Read File too long, making filesystem readonly\n");
        romfs_fs_clear_flag( pfsdata, ROMFS_FS_FLAG_READY_WRITE );
      }
      return NULL;
    }
    
    // fill in file time & flags
    pent->ftime = 0;
    pent->flags = 0;

    // Jump offset ahead by length field & file size
    off += ROMFS_SIZE_LEN;
    off += pent->fsize;


    // If WOFS, also advance offset by deleted file field
    if( romfsh_is_wofs( pfsdata ) )
      off = ( off + ROMFS_ALIGN - 1 ) & ~( ROMFS_ALIGN - 1 );

    //If file security is enabled, don't return a matching filename as valid
#ifdef ROMFS_SECURE_FILENAMES_WITH_CHAR
    if( !is_deleted && ( romfs_security_lock == 0 || strstr( pent->fname, ROMFS_SECURE_FILENAMES_WITH_CHAR) == NULL ) )
      break;
#else
    if( !is_deleted )
      break;
#endif    
  }
  *( u32* )d = off;
  return pent;
}*/

// closedir
static int nffs_closedir_r( struct _reent *r, void *d, void *pdata )
{
  *( u32* )d = 0;
  return 0;
}

// unlink
static int nffs_unlink_r( struct _reent *r, const char *path, void *pdata )
{
  return NIFFS_remove(&fs, (char *)path);
}

static const DM_DEVICE niffs_device = 
{
  nffs_open_r,         // open
  nffs_close_r,        // close
  nffs_write_r,        // write
  nffs_read_r,         // read
  nffs_lseek_r,        // lseek
  nffs_opendir_r,      // opendir
  nffs_readdir_r,      // readdir TODO peter, made an attempt at this
  nffs_closedir_r,     // closedir
  nffs_getaddr_r,      // getaddr
  NULL,                // mkdir
  nffs_unlink_r,       // unlink
  NULL,                // rmdir
  NULL                 // rename // TODO peter, this exists in niffs also if you want it
};

static int platform_hal_erase_f(u8_t *addr, u32_t len) {
  // flash boundary checks
  if ((intptr_t)addr < (intptr_t)niffs_pbase ||
      (intptr_t)addr + len > (intptr_t)niffs_pbase + niffs_max_size) {
    NFFS_DBG("N_E: OOB %li,%li\n", (u32_t)addr, len);
    return -1; // TODO peter better err code?
  }
/*  if (addr < &_flash[0]) return ERR_NIFFS_TEST_BAD_ADDR;
  if (addr+len > &_flash[0] + EMUL_SECTORS * EMUL_SECTOR_SIZE) return ERR_NIFFS_TEST_BAD_ADDR;
  if ((addr - &_flash[0]) % EMUL_SECTOR_SIZE) return ERR_NIFFS_TEST_BAD_ADDR;
  if (len != EMUL_SECTOR_SIZE) return ERR_NIFFS_TEST_BAD_ADDR;*/
  platform_flash_erase_sector(((u32_t)addr)/INTERNAL_FLASH_SECTOR_SIZE); // == PLATFORM_OK;
  NFFS_DBG("N_E:%li,%li\n", (u32_t)addr, len);
  return NIFFS_OK;
}

static int platform_hal_write_f(u8_t *addr, const u8_t *src, u32_t len) {
  if ((intptr_t)addr < (intptr_t)niffs_pbase ||
      (intptr_t)addr + len > (intptr_t)niffs_pbase + niffs_max_size) {
    NFFS_DBG("N_W: OOB %li,%li\n", (u32_t)addr, len);
    return -1; // TODO peter better err code?
  }
  //TODO: Write actual data to addresses here
  //  toaddr += ( u32 )pfsdata->pbase;
  //platform_flash_write( const void *from, u32 toaddr, u32 size )
  platform_flash_write( src, (u32_t)addr, len );
  NFFS_DBG("N_W:%li,%li,%li\n", (u32_t)addr, (u32_t)src, len);
  return NIFFS_OK;
}

int nffs_check_and_mount()
{
  //Check filesystem, and format if it has not been formatted or corrupted...
  NIFFS_unmount(&fs);
  if(NIFFS_chk(&fs) == ERR_NIFFS_NOT_A_FILESYSTEM)
  {
    NFFS_DBG("chk NOT FS\n");
    NIFFS_format(&fs);
  } else
    NFFS_DBG("chk OK\n");

  if(NIFFS_mount(&fs))
  {
    printf("FILE SYSTEM MOUNT FAIL\n");
    return 0;
  }
  else
  {
    NFFS_DBG("MNT OK\n");
    return 1;
  }
}

int nffs_create( s32_t linear_bytes )
{
  niffs_pbase = ( u8* )platform_flash_get_first_free_block_address( NULL );
  niffs_max_size = INTERNAL_FLASH_SIZE - ( ( u32 )niffs_pbase - INTERNAL_FLASH_START_ADDRESS ) - (INTERNAL_FLASH_SECTOR_SIZE);
  niffs_total_sectors = niffs_max_size / INTERNAL_FLASH_SECTOR_SIZE;

  if(linear_bytes == -1)
  {
    //Look to see if we have a valid magic byte and if so, get the linear page size
    //It is stored in the lower 2 bytes of the magic number (abra)
    niffs_sector_hdr *shdr = (niffs_sector_hdr *)niffs_pbase;
    NFFS_DBG("%08lX %08lX\n", 
      shdr->abra,
      _NIFFS_SECT_LINEAR_SIZE(shdr->abra));

    niffs_lin_sectors = 1;
    //Get linear sectors stored on flash...if it's set...if not, assume zero linear sectors
    //If not, format the file system with no linear page area. User will format it if needed
    if ((shdr->abra >> 24) == (_NIFFS_SECT_MAGIC_BYTES))
      niffs_lin_sectors = _NIFFS_SECT_LINEAR_SIZE(shdr->abra);
  } else {
    //We were told to create a linear space, calculate it here
    //Division drops the decimal, so we need to divide and add 1 sector
    niffs_lin_sectors = (linear_bytes / INTERNAL_FLASH_SECTOR_SIZE) + 1;
    //Make sure we leave at least 1 sector for paged sectors
    if(niffs_lin_sectors > (niffs_total_sectors - 1))
      niffs_lin_sectors = niffs_total_sectors - 1;
  }

  niffs_paged_sectors = niffs_total_sectors - niffs_lin_sectors;
  printf("NIFFS: LIN %li PAGE %li\n", niffs_lin_sectors, niffs_paged_sectors);

  NIFFS_init(&fs, niffs_pbase, niffs_paged_sectors, INTERNAL_FLASH_SECTOR_SIZE, NIFFS_BUF_SIZE,
      buf, sizeof(buf),
      descs, NIFFS_FILE_DESCS,
      platform_hal_erase_f, platform_hal_write_f, niffs_lin_sectors);

  //If given a specific linear byte size, format flash
  if(linear_bytes != -1)
    NIFFS_format(&fs);

  //Check filesystem, and format if it has not been formatted or corrupted...
  return nffs_check_and_mount();
}

int nffs_init( void )
{
  nffs_create(-1);

  dm_register( "/f", &fs, &niffs_device );

  return dm_register( NULL, NULL, NULL );
}

int nffs_format(s32_t linear_bytes)
{
  NIFFS_unmount(&fs);
  nffs_create(linear_bytes);  
  NFFS_DBG("N_I:%li,%li,%li,%li\n", (u32_t)niffs_pbase, niffs_max_size, niffs_paged_sectors, niffs_lin_sectors);
  return 1;
}

int nffs_mount()
{
  return NIFFS_mount(&fs);
}

int nffs_unmount()
{
  return NIFFS_unmount(&fs);
}

int nffs_info(s32_t *total, s32_t *used, u8_t *overflow, s32_t *lin_total, s32_t *lin_used, s32_t *lin_free)
{
  niffs_sector_hdr *shdr = (niffs_sector_hdr *)_NIFFS_SECTOR_2_ADDR(&fs, 0);
  printf("%08lX %08lX %08lX\n", 
    shdr->abra, 
    _NIFFS_SECT_MAGIC(&fs),
    _NIFFS_SECT_LINEAR_SIZE(shdr->abra));

  nffs_check_and_mount();

  niffs_info info;
  int res = NIFFS_info(&fs, &info);
  if (res == NIFFS_OK) {
    *total = info.total_bytes;
    *used = info.used_bytes;
    *overflow = info.overflow;
    *lin_total = info.lin_total_sectors * INTERNAL_FLASH_SECTOR_SIZE;
    *lin_used = info.lin_used_sectors * INTERNAL_FLASH_SECTOR_SIZE;
    *lin_free = info.lin_max_conseq_free * INTERNAL_FLASH_SECTOR_SIZE;
  }

  return res;
}

#else // #if defined( BUILD_ROMFS ) || defined( BUILD_WOFS )

int nffs_init( void )
{
  return dm_register( NULL, NULL, NULL );
}

#endif // #if defined( BUILD_NIFFS ) 

