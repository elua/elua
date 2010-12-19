// Type definitions for the remote file system

#ifndef __TYPE_H__
#define __TYPE_H__

typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef long s32;
typedef unsigned long u32;
typedef long long s64;
typedef unsigned long long u64;

#ifdef WIN32_BUILD

#include <windows.h>
typedef struct
{
  HANDLE hnd;
  OVERLAPPED o;
  OVERLAPPED o_wr;
  BOOL fWaitingOnRead;
  u8 databuf;
} SERIAL_DATA;
typedef SERIAL_DATA* ser_handler;
#define SER_HANDLER_INVALID   ( NULL )
typedef HANDLE sync_object;

#else // #ifdef WIN32_BUILD

// Assume POSIX here

typedef int ser_handler;
#define SER_HANDLER_INVALID   ( -1 )
typedef int sync_object;

#endif // #ifdef WIN32_BUILD
                                    
#endif // #ifndef __TYPE_H__
