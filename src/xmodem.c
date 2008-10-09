/*! \file xmodem.c \brief XModem Transmit/Receive Implementation with CRC and 1K support. */
//*****************************************************************************
//
// File Name  : 'xmodem.c'
// Title    : XModem Transmit/Receive Implementation with CRC and 1K support
// Author     : Pascal Stang - Copyright (C) 2006
// Created    : 4/22/2006
// Revised    : 7/22/2006
// Version    : 0.1
// Target MCU   : AVR processors
// Editor Tabs  : 4
//
// This code is distributed under the GNU Public License
//    which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************
// Modified by BogdanM for the eLua project
//*****************************************************************************

#include <string.h>
#include "xmodem.h"
#include "platform.h"

#include "platform_conf.h"
#ifdef BUILD_XMODEM

#define XMODEM_BUFFER_SIZE    128

// pointers to stream I/O functions
static p_xm_send_func xmodem_out_func;
static p_xm_recv_func xmodem_in_func;

typedef u16 uint16_t;
typedef u8 uint8_t;

static uint16_t crc_xmodem_update(uint16_t crc, uint8_t data)
{
  int i;

  crc = crc ^ ((uint16_t)data << 8);
  for (i=0; i<8; i++)
  {
    if(crc & 0x8000)
      crc = (crc << 1) ^ 0x1021;
    else
      crc <<= 1;
  }

  return crc;
}

static int xmodem_crc_check( int crcflag, const unsigned char *buffer, int size )
{
  // crcflag=0 - do regular checksum
  // crcflag=1 - do CRC checksum

  if(crcflag)
  {
    unsigned short crc=0;
    unsigned short pktcrc = (buffer[size]<<8)+buffer[size+1];
    // do CRC checksum
    while(size--)
      crc = crc_xmodem_update(crc, *buffer++);
    // check checksum against packet
    if(crc == pktcrc)
      return 1;
  }
  else
  {
    int i;
    unsigned char cksum = 0;
    // do regular checksum
    for(i=0; i<size; ++i)
    {
      cksum += buffer[i];
    }
    // check checksum against packet
    if(cksum == buffer[size])
      return 1;
  }

  return 0;
}

static void xmodem_flush(void)
{
  while( xmodem_in_func( XMODEM_TIMEOUT_DELAY ) >= 0 );
}

void xmodem_init( p_xm_send_func send_func, p_xm_recv_func recv_func )
{
  xmodem_out_func = send_func;
  xmodem_in_func = recv_func;
}

long xmodem_receive( char* dest, u32 limit )
{
  unsigned char xmbuf[XMODEM_BUFFER_SIZE+6];
  unsigned char seqnum=1;     // xmodem sequence number starts at 1
  unsigned short pktsize=128;   // default packet size is 128 bytes
  unsigned char response='C';   // solicit a connection with CRC
  char retry=XMODEM_RETRY_LIMIT;
  unsigned char crcflag=0;
  unsigned long totalbytes=0;
  int i,c;

  while(retry > 0)
  {
    // solicit a connection/packet
    xmodem_out_func(response);
    // wait for start of packet
    if( (c = xmodem_in_func(XMODEM_TIMEOUT_DELAY)) >= 0)
    {
      switch(c)
      {
      case SOH:
        pktsize = 128;
        break;
      case EOT:
        xmodem_flush();
        xmodem_out_func(ACK);
        // completed transmission normally
        return totalbytes;
      case CAN:
        if((c = xmodem_in_func(XMODEM_TIMEOUT_DELAY)) == CAN)
        {
          xmodem_flush();
          xmodem_out_func(ACK);
          // transaction cancelled by remote node
          return XMODEM_ERROR_REMOTECANCEL;
        }
      default:
        break;
      }
    }
    else
    {
      // timed out, try again
      // no need to flush because receive buffer is already empty
      retry--;
      //response = NAK;
      continue;
    }

    // check if CRC mode was accepted
    if(response == 'C') crcflag = 1;
    // got SOH/STX, add it to processing buffer
    xmbuf[0] = c;
    // try to get rest of packet
    for(i=0; i<(pktsize+crcflag+4-1); i++)
    {
      if((c = xmodem_in_func(XMODEM_TIMEOUT_DELAY)) >= 0)
      {
        xmbuf[1+i] = c;
      }
      else
      {
        // timed out, try again
        retry--;
        xmodem_flush();
        response = NAK;
        break;
      }
    }
    // packet was too small, retry
    if(i<(pktsize+crcflag+4-1))
      continue;

    // got whole packet
    // check validity of packet
    if(   (xmbuf[1] == (unsigned char)(~xmbuf[2])) &&     // sequence number was transmitted w/o error
      xmodem_crc_check(crcflag, &xmbuf[3], pktsize) ) // packet is not corrupt
    {
      // is this the packet we were waiting for?
      if(xmbuf[1] == seqnum)
      {
        // write/deliver data
        if( totalbytes + pktsize > limit )
        {
          // Cancel transmission
          xmodem_flush();
          xmodem_out_func(CAN);
          xmodem_out_func(CAN);
          xmodem_out_func(CAN);    
          return XMODEM_ERROR_OUTOFMEM;    
        }
        memcpy( dest + totalbytes, xmbuf + 3, pktsize );
        totalbytes += pktsize;
        // next sequence number
        seqnum++;
        // reset retries
        retry = XMODEM_RETRY_LIMIT;
        // reply with ACK
        response = ACK;
        continue;
      }
      else if(xmbuf[1] == (unsigned char)(seqnum-1))
      {
        // this is a retransmission of the last packet
        // ACK and move on
        response = ACK;
        continue;
      }
      else
      {
        // we are completely out of sync
        // cancel transmission
        xmodem_flush();
        xmodem_out_func(CAN);
        xmodem_out_func(CAN);
        xmodem_out_func(CAN);
        return XMODEM_ERROR_OUTOFSYNC;
      }
    }
    else
    {
      // packet was corrupt
      // NAK it and try again
      retry--;
      xmodem_flush();
      response = NAK;
      continue;
    }
  }

  // exceeded retry count
  xmodem_flush();
  xmodem_out_func(CAN);
  xmodem_out_func(CAN);
  xmodem_out_func(CAN);
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
