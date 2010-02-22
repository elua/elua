/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#include "type.h"

#ifndef _INTEGER

typedef signed int		INT;
typedef unsigned int	UINT;

#ifndef __TYPE_H__
/* These types are assumed as 8-bit integer */
typedef signed char		CHAR;
typedef unsigned char	UCHAR;
typedef unsigned char	BYTE;

/* These types are assumed as 16-bit integer */
typedef signed short	SHORT;
typedef unsigned short	USHORT;
typedef unsigned short	WORD;
typedef unsigned short  WCHAR;

/* These types are assumed as 32-bit integer */
typedef signed long		LONG;
typedef unsigned long	ULONG;
typedef unsigned long	DWORD;

/* Boolean type */
typedef enum { FALSE = 0, TRUE } BOOL;
#else
/* Only define types not in type.h */
typedef signed char     CHAR;
typedef unsigned short  WCHAR;
typedef unsigned char   UCHAR;
typedef signed short    SHORT;
typedef unsigned short  USHORT;
typedef signed long     LONG;
typedef unsigned long   ULONG;
#endif

#define _INTEGER
#endif
