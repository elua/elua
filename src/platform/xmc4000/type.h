#ifndef __TYPE_H__
#define __TYPE_H__

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed long s32;
typedef unsigned long u32;
typedef signed long long s64;
typedef unsigned long long u64;

#ifndef NULL
#define NULL    ((void *)0)
#endif

typedef enum BOOLType{
	FALSE,
	TRUE
}BOOLType;

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef unsigned int   BOOL;

#endif

