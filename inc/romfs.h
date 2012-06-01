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

// A small "FILE" structure
typedef struct 
{
  u32 baseaddr;
  u32 offset;
  u32 size;
  p_read_fs_byte p_read_func;
} FS;
  
// FS functions
const DM_DEVICE* romfs_init();

#endif

