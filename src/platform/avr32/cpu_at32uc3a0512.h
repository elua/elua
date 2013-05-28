// AT32UC3A0512 CPU configuration

#ifndef __CPU_AT32UC3A0512_H__
#define __CPU_AT32UC3A0512_H__

// Use AT32UC3A0128 as base, change only what's different
#include "cpu_at32uc3a0128.h"

#undef RAM_SIZE
#define RAM_SIZE 0x10000

#endif // #ifndef __CPU_AT32UC3A0512_H__

