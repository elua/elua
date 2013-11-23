// MMC filesystem

#ifndef __MMCFS_H__
#define __MMCFS_H__

#include "type.h"
#include "devman.h"

// FS functions
int mmcfs_init( void );
void elua_mmc_init( void );
DWORD get_fattime (void);

#endif
