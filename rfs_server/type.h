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

// Define serial port "handle" type for each platform
// [TODO] for now, only UNIX is supported
#ifdef WIN32_BUILD
#include <windows.h>
typedef HANDLE ser_handler;
#else // assume POSIX here
typedef int ser_handler;
#endif
                                    
#endif
