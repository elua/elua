// eLua "char device" buffering system

#ifndef __BUF_H__
#define __BUF_H__

#include "type.h"

// [TODO] the buffer data type is currently u8, is this OK?
typedef u8 t_buf_data;

// IDs of "bufferable" devices
enum
{
  BUF_ID_UART = 0,
  BUF_ID_ADC = 1,
  BUF_ID_FIRST = BUF_ID_UART,
  BUF_ID_LAST = BUF_ID_ADC,
  BUF_ID_TOTAL = BUF_ID_LAST - BUF_ID_FIRST + 1
};

// This structure describes a buffer
typedef struct 
{
  u8 logsize;
  u8 logdsize;
  volatile u16 wptr, rptr, count;
  t_buf_data *buf;
} buf_desc;

// Buffer sizes (there are power of 2 to speed up modulo operations)
enum
{
  BUF_SIZE_NONE = 0,
  BUF_SIZE_2,
  BUF_SIZE_4,
  BUF_SIZE_8,
  BUF_SIZE_16,
  BUF_SIZE_32,
  BUF_SIZE_64,
  BUF_SIZE_128,
  BUF_SIZE_256,
  BUF_SIZE_512,
  BUF_SIZE_1024,
  BUF_SIZE_2048,
  BUF_SIZE_4096,
  BUF_SIZE_8192,
  BUF_SIZE_16384,
  BUF_SIZE_32768
};

enum
{
  BUF_DSIZE_U8 = 0,
  BUF_DSIZE_U16,
  BUF_DSIZE_U32
};



// Buffer API
int buf_set(unsigned resid, unsigned resnum, u8 logsize, u8 logdsize);
int buf_is_enabled( unsigned resid, unsigned resnum );
unsigned buf_get_size( unsigned resid, unsigned resnum );
unsigned buf_get_count( unsigned resid, unsigned resnum );
int buf_write( unsigned resid, unsigned resnum, t_buf_data *data );
int buf_read( unsigned resid, unsigned resnum, t_buf_data *data );
void buf_flush( unsigned resid, unsigned resnum );

#endif
