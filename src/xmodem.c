/*-
 * Copyright (c) 2006 M. Warner Losh.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This software is derived from software provide by Kwikbyte who specifically
 * disclaimed copyright on the code.  This version of xmodem has been nearly
 * completely rewritten, but the CRC is from the original.
 *
 * $FreeBSD: src/sys/boot/arm/at91/libat91/xmodem.c,v 1.1 2006/04/19 17:16:49 imp Exp $
 */

// Modified by BogdanM for eLua

#include <string.h>
#include <stdlib.h>
#include "xmodem.h"
#include "platform.h"

#include "platform_conf.h"
#ifdef BUILD_XMODEM

#define PACKET_SIZE    128
static p_xm_send_func xmodem_out_func;
static p_xm_recv_func xmodem_in_func;

// Line control codes
#define SOH  0x01
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18
#define EOT  0x04

// Arguments to xmodem_flush
#define XMODEM_FLUSH_ONLY       0
#define XMODEM_FLUSH_AND_CAN    1

// Delay in "flush packet" mode
#define XMODEM_PACKET_DELAY     10000UL

void xmodem_init( p_xm_send_func send_func, p_xm_recv_func recv_func )
{
  xmodem_out_func = send_func;
  xmodem_in_func = recv_func;
}

// Utility function: flush the receive buffer
static void xmodem_flush( int how )
{
  while( xmodem_in_func( XMODEM_PACKET_DELAY ) != -1 );
  if( how == XMODEM_FLUSH_AND_CAN )
  {
    xmodem_out_func( CAN );
    xmodem_out_func( CAN );
    xmodem_out_func( CAN );
  }
}

// This private function receives a x-modem record to the pointer and
// returns 1 on success and 0 on error
static int xmodem_get_record( unsigned char blocknum, unsigned char *pbuf )
{
  unsigned chk, j, size;
  int ch;
  
  // Read packet
  for( j = 0; j < PACKET_SIZE + 4; j ++ )
  {
    if( ( ch = xmodem_in_func( XMODEM_TIMEOUT ) ) == -1 )
      goto err;
    pbuf[ j ] = ( unsigned char )ch;
  }

  // Check block number
  if( *pbuf ++ != blocknum )
    goto err;
  if( *pbuf ++ != ( unsigned char )~blocknum )
    goto err;
  // Check CRC
  for( size = chk = 0; size < PACKET_SIZE; size++, pbuf ++ ) 
  {
    chk = chk ^ *pbuf << 8;
    for( j = 0; j < 8; j ++ ) 
    {
      if( chk & 0x8000 )
        chk = chk << 1 ^ 0x1021;
      else
        chk = chk << 1;
    }
  }
  chk &= 0xFFFF;
  if( *pbuf ++ != ( ( chk >> 8 ) & 0xFF ) )
    goto err;
  if( *pbuf ++ != ( chk & 0xFF ) )
    goto err;
  return 1;
  
err:
  xmodem_out_func( NAK );
  return 0;
}

// This global function receives a x-modem transmission consisting of
// (potentially) several blocks.  Returns the number of bytes received or
// an error code an error
long xmodem_receive( char **dest )
{
  int starting = 1, ch;
  unsigned char packnum = 1, buf[ PACKET_SIZE + 4 ];
  unsigned retries = XMODEM_RETRY_LIMIT;
  u32 limit = XMODEM_INITIAL_BUFFER_SIZE, size = 0;
  void *p;
  
  while( retries-- ) 
  {
    if( starting )
      xmodem_out_func( 'C' );
    if( ( ( ch = xmodem_in_func( XMODEM_TIMEOUT ) ) == -1 ) || ( ch != SOH && ch != EOT && ch != CAN ) )
      continue;
    if( ch == EOT ) 
    {
      // End of transmission
      xmodem_out_func( ACK );
      xmodem_flush( XMODEM_FLUSH_ONLY );
      return size;
    }
    else if( ch == CAN )
    {
      // The remote part ended the transmission
      xmodem_out_func( ACK );
      xmodem_flush( XMODEM_FLUSH_ONLY );
      return XMODEM_ERROR_REMOTECANCEL;      
    }
    starting = 0;
    
    // Get XMODEM packet
    if( !xmodem_get_record( packnum, buf ) )
      continue; // allow for retransmission
    xmodem_flush( XMODEM_FLUSH_ONLY );      
    retries = XMODEM_RETRY_LIMIT;
    packnum ++;
      
    // Got a valid packet
    if( size + PACKET_SIZE > limit )
    {
      limit += XMODEM_INCREMENT_AMMOUNT;
      if( ( p = realloc( *dest, limit ) ) == NULL )
      {
        // Not enough memory, force cancel and return
        xmodem_flush( XMODEM_FLUSH_AND_CAN );
        return XMODEM_ERROR_OUTOFMEM;
      }
      *dest = ( char* )p;
    }    
    // Acknowledge and consume packet
    xmodem_out_func( ACK );
    memcpy( *dest + size, buf + 2, PACKET_SIZE );
    size += PACKET_SIZE;
  }
  
  // Exceeded retry count
  xmodem_flush( XMODEM_FLUSH_AND_CAN );
  return XMODEM_ERROR_RETRYEXCEED;
}

#else // #ifdef BUILD_XMODEM

// Dummy init function

void xmodem_init( p_xm_send_func send_func, p_xm_recv_func recv_func )
{
  send_func = send_func;
  recv_func = recv_func;
}

#endif // #ifdef BUILD_XMODEM
