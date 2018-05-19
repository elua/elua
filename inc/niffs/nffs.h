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

#ifndef NFFS_H_
#define NFFS_H_

int nffs_init( void );
int nffs_format(s32_t linear_bytes);
int nffs_mount();
int nffs_unmount();
int nffs_check();
int nffs_info(s32_t *total, s32_t *used, u8_t *overflow, s32_t *lin_total, s32_t *lin_used, s32_t *lin_free);

#endif /* NFFS_H_ */
