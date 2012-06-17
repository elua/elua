// Semihosting filesystem

#ifndef __SEMIFS_H__
#define __SEMIFS_H__

#include "type.h"
#include "devman.h"
#include <stdint.h>

// ARM Semihosting Commands
#define SYS_OPEN   (0x1)
#define SYS_CLOSE  (0x2)
#define SYS_WRITE  (0x5)
#define SYS_READ   (0x6)
#define SYS_ISTTY  (0x9)
#define SYS_SEEK   (0xa)
#define SYS_ENSURE (0xb)
#define SYS_FLEN   (0xc)

// Required Structures
// These structures provided by Simon Ford of mbed
typedef struct {
  uint8_t  hr;   /* Hours    [0..23]                  */
  uint8_t  min;  /* Minutes  [0..59]                  */
  uint8_t  sec;  /* Seconds  [0..59]                  */
  uint8_t  day;  /* Day      [1..31]                  */
  uint8_t  mon;  /* Month    [1..12]                  */
  uint16_t year; /* Year     [1980..2107]             */
} FTIME; // 7 bytes

typedef struct {       // File Search info record
  char  name[32];      // File - 32-bytes
  uint32_t  size;      // File size in bytes - 4-bytes
  uint16_t   fileID;   // System File Identification  - 2-bytes
  FTIME create_time;   // Date & time file was created
  FTIME write_time;    // Date & time of last write
} XFINFO;

typedef struct {
  char *pattern;
  XFINFO file_info;
} SEARCHINFO;


// FS functions
int semifs_init();

#endif
