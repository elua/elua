// Read-only ROM filesystem

#ifndef __FS_H__
#define __FS_H__

#include "type.h"
#include "devman.h"
// Maximum length of a filename in the filesystem
#define MAX_FNAME_LENGTH      14

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
typedef u8 ( *p_read_fs_byte )( u16 );

// A small "FILE" structure
typedef struct 
{
  u16 baseaddr;
  u16 offset;
  u16 size;
  p_read_fs_byte p_read_func;
} FS;
  
// FS functions
DM_DEVICE* romfs_init();
u16 romfs_get_dir_entry(u16 offset, char *fname, int *fsize);

#endif
