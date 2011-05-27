#ifndef __TYPE_H__
#define __TYPE_H__

#include "stm32f10x.h"

#ifndef NULL
#define NULL    ((void *)0)
#endif

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (1)
#endif

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef unsigned int   BOOL;

typedef unsigned long long u64;
typedef signed long long s64;

#endif
