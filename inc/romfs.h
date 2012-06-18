// Read-only ROM filesystem

#ifndef __FS_H__
#define __FS_H__

#include "type.h"
#include "devman.h"

/*******************************************************************************
The Read-Only "filesystem" resides in a contiguous zone of memory, with the
following structure, repeated for each file:

Filename: ASCIIZ, max length is DM_MAX_FNAME_LENGTH defined here, empty if last file
File size: (4 bytes)
File data: (file size bytes)

*******************************************************************************/

enum
{
  FS_FILE_NOT_FOUND,
  FS_FILE_OK
};
 
// This is the function used to read a byte at the given address from the file
// system
typedef u8 ( *p_read_fs_byte )( u32 );

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

// Filesystem flags
#define ROMFS_FS_FLAG_DIRECT      0x01
#define ROMFS_FS_FLAG_WO          0x02

// File system descriptor
typedef struct
{
  u8 *pbase;
  u8 flags;
} FSDATA;
  
// FS functions
int romfs_init();

#endif

