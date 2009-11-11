// MMC filesystem

#ifndef __MMCFS_H__
#define __MMCFS_H__

#include "type.h"
#include "devman.h"
// Maximum length of a filename in the filesystem
#ifndef MAX_FNAME_LENGTH
#define MAX_FNAME_LENGTH      30
#endif

// FS functions
DM_DEVICE* mmcfs_init();

#endif
