// Read-only ROM filesystem

#ifndef __FS_H__
#define __FS_H__

#include "type.h"
#include "devman.h"

/*******************************************************************************
The Read-Only "filesystem" resides in a contiguous zone of memory, with the
following structure (repeated for each file):

Filename: ASCIIZ, max length is DM_MAX_FNAME_LENGTH, first byte is 0xFF if last file
File size: (4 bytes), aligned to ROMFS_ALIGN bytes 
File data: (file size bytes)

The WOFS (Write Once File System) uses much of the ROMFS functions, thuss it is
also implemented in romfs.c. It resides in a contiguous zone of memory, with a
structure that is quite similar with ROMFS' structure (repeated for each file):

Filename: ASCIIZ, max length is DM_MAX_FNAME_LENGTH, first byte is 0xFF if last file.
          WOFS filenames always begin at an address which is a multiple of ROMFS_ALIGN.
File deleted flag: (WOFS_DEL_FIELD_SIZE bytes), aligned to ROMFS_ALIGN bytes
File size: (4 bytes), aligned to ROMFS_ALIGN bytes
File data: (file size bytes)

*******************************************************************************/

enum
{
  FS_FILE_NOT_FOUND,
  FS_FILE_OK
};
 
// ROMFS/WOFS functions
typedef u32 ( *p_fs_read )( void *to, u32 fromaddr, u32 size, const void *pdata );
typedef u32 ( *p_fs_write )( const void *from, u32 toaddr, u32 size, const void *pdata );

// File flags
#define ROMFS_FILE_FLAG_READ      0x01
#define ROMFS_FILE_FLAG_WRITE     0x02
#define ROMFS_FILE_FLAG_APPEND    0x04

// A small "FILE" structure
typedef struct 
{
  u32 baseaddr;
  u32 offset;
  u32 size;
  u8 flags;
} FD;

// WOFS constants
// The miminum size we need in order to create another file
// This size will be added to the size of the filename when creating a new file
// to ensure that there's enough space left on the device
// This comes from the size of the file length field (4) + the maximum number of
// bytes needed to align this field (3) + a single 0xFF byte which marks the end
// of the filesystem (1) + the maximum number of bytes needed to align the contents 
// of a file (3)
#define WOFS_MIN_NEEDED_SIZE      11   

// Filesystem flags
#define ROMFS_FS_FLAG_DIRECT      0x01    // direct mode (the file is mapped in a memory area directly accesible by the CPU)
#define ROMFS_FS_FLAG_WO          0x02    // this FS is actually a WO (Write-Once) FS
#define ROMFS_FS_FLAG_WRITING     0x04    // for WO only: there is already a file opened in write mode

// File system descriptor
typedef struct
{
  u8 *pbase;                      // pointer to FS base in memory (only for ROMFS_FS_FLAG_DIRECT)
  u8 flags;                       // flags (see above)
  p_fs_read readf;                // pointer to read function (for non-direct mode FS)
  p_fs_write writef;              // pointer to write function (only for ROMFS_FS_FLAG_WO)
  u32 max_size;                   // maximum size of the FS (in bytes)
} FSDATA;

#define romfs_fs_set_flag( p, f )     p->flags |= ( f )
#define romfs_fs_clear_flag( p, f )   p->flags &= ( u8 )~( f )
#define romfs_fs_is_flag_set( p, f )  ( ( p->flags & ( f ) ) != 0 )

// FS functions
int romfs_init();
int wofs_format();

#endif

