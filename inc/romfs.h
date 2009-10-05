// Read-only ROM filesystem

#ifndef __FS_H__
#define __FS_H__

#include "type.h"
#include "devman.h"
// Maximum length of a filename in the filesystem
#define MAX_FNAME_LENGTH      30

/*******************************************************************************
The Read-Only "filesystem" resides in a contiguous zone of memory, with the
following structure, repeated for each file:

Filename: ASCIIZ, max length is MAX_FNAME_LENGTH defined here, empty if last file
File size: (2 bytes)
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
  u16 size;
  p_read_fs_byte p_read_func;
} FS;
  
// FS functions
DM_DEVICE* romfs_init();
u32 romfs_get_dir_entry( u32 offset, char *fname, u16 *fsize );

#endif

