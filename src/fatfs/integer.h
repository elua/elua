/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#ifndef _INTEGER

/* These types must be 16-bit, 32-bit or larger integer */
typedef int				INT;
typedef unsigned int	UINT;

#ifndef __TYPE_H__
/* These types must be 8-bit integer */
typedef signed char		CHAR;
typedef unsigned char	UCHAR;
typedef unsigned char	BYTE;

/* These types must be 16-bit integer */
typedef short			SHORT;
typedef unsigned short	USHORT;
typedef unsigned short	WORD;

/* These types must be 32-bit integer */
typedef long			LONG;
typedef unsigned long	ULONG;
typedef unsigned long	DWORD;

/* Boolean type */
typedef enum { FALSE = 0, TRUE } BOOL;
#else
/* Only define types not in type.h */
typedef signed char             CHAR;
typedef unsigned char   UCHAR;
typedef signed short    SHORT;
typedef unsigned short  USHORT;
typedef signed long             LONG;
typedef unsigned long   ULONG;
#endif

#define _INTEGER
#endif
