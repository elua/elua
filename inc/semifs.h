// Semihosting filesystem

#ifndef __SEMIFS_H__
#define __SEMIFS_H__

#include "type.h"
#include "devman.h"

// ARM Semihosting Commands
#define SYS_OPEN   (0x1)
#define SYS_CLOSE  (0x2)
#define SYS_WRITE  (0x5)
#define SYS_READ   (0x6)
#define SYS_ISTTY  (0x9)
#define SYS_SEEK   (0xa)
#define SYS_ENSURE (0xb)
#define SYS_FLEN   (0xc)

// FS functions
const DM_DEVICE* semifs_init();

#endif
