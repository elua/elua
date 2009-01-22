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
  BUF_ID_FIRST = BUF_ID_UART,
  BUF_ID_LAST = BUF_ID_UART,
  BUF_ID_TOTAL = BUF_ID_LAST - BUF_ID_FIRST + 1
};

// This structure describes a buffer
typedef struct 
{
  u8 logsize;
  volatile u16 wptr, rptr, count;
  t_buf_data *buf;
} buf_desc;

// Buffer sizes (there are power of 2 to speed up modulo operations)
enum
{
  BUF_SIZE_NONE = 0,
  BUF_SIZE_16 = 4,
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

// Buffer API
int buf_set( unsigned resid, unsigned resnum, u8 logsize );
int buf_is_enabled( unsigned resid, unsigned resnum );
unsigned buf_get_size( unsigned resid, unsigned resnum );
unsigned buf_get_count( unsigned resid, unsigned resnum );
int buf_get_char( unsigned resid, unsigned resnum );
void buf_rx_cb( unsigned resid, unsigned resnum, t_buf_data data );

#endif
