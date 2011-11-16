/* This source file is part of the ATMEL AVR32-UC3-SoftwareFramework-1.6.0 Release */

/*This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief MACB driver for EVK1100 board.
 *
 * This file defines a useful set of functions for the MACB interface on
 * AVR32 devices.
 *
 * - Compiler:           IAR EWAVR32 and GNU GCC for AVR32
 * - Supported devices:  All AVR32 devices with a MACB module can be used.
 * - AppNote:
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support and FAQ: http://support.atmel.no/
 *
 *****************************************************************************/

/* Copyright (c) 2009 Atmel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an Atmel
 * AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 *
 */
/*
 * ported on elua world by nuccio raciti <raciti.nuccio@gmail.com>, enjoy.
 */
#include <stdio.h>
#include <string.h>
#include <avr32/io.h>

#include "compiler.h"

#include "platform_conf.h"

#ifdef BUILD_UIP

#include "ethernet.h"

/* Size of each receive buffer - DO NOT CHANGE. */
#define RX_BUFFER_SIZE    128


/* The buffer addresses written into the descriptors must be aligned so the
last two bits are zero.  These bits have special meaning for the MACB
peripheral and cannot be used as part of the address. */
#define ADDRESS_MASK      ( ( unsigned long ) 0xFFFFFFFC )

/* Bit used within the address stored in the descriptor to mark the last
descriptor in the array. */
#define RX_WRAP_BIT       ( ( unsigned long ) 0x02 )

/* A short delay is used to wait for a buffer to become available, should
one not be immediately available when trying to transmit a frame. */
#define BUFFER_WAIT_DELAY   ( 2 )

#include "ethernet.h"
#include "intc.h"
#include "elua_uip.h"

#ifndef FREERTOS_USED
#define portENTER_CRITICAL           Disable_global_interrupt
#define portEXIT_CRITICAL            Enable_global_interrupt
#define portENTER_SWITCHING_ISR()
#define portEXIT_SWITCHING_ISR()
#endif

#define TIMEOUT 500


/* Buffer written to by the MACB DMA.  Must be aligned as described by the
comment above the ADDRESS_MASK definition. */

static volatile char pcRxBuffer[ ETHERNET_CONF_NB_RX_BUFFERS * RX_BUFFER_SIZE ] __attribute__ ((aligned (4)));
/* Buffer read by the MACB DMA.  Must be aligned as described by the comment
above the ADDRESS_MASK definition. */

static volatile char pcTxBuffer[ ETHERNET_CONF_NB_TX_BUFFERS * ETHERNET_CONF_TX_BUFFER_SIZE ] __attribute__ ((aligned (4)));

/* Descriptors used to communicate between the program and the MACB peripheral.
These descriptors hold the locations and state of the Rx and Tx buffers.
Alignment value chosen from RBQP and TBQP registers description in datasheet. */

static volatile AVR32_TxTdDescriptor xTxDescriptors[ ETHERNET_CONF_NB_TX_BUFFERS ] __attribute__ ((aligned (8)));
static volatile AVR32_RxTdDescriptor xRxDescriptors[ ETHERNET_CONF_NB_RX_BUFFERS ] __attribute__ ((aligned (8)));

/* The IP and Ethernet addresses are read from the header files. */
unsigned char cMACAddress[ 6 ] = { ETHERNET_CONF_ETHADDR0,ETHERNET_CONF_ETHADDR1,ETHERNET_CONF_ETHADDR2,ETHERNET_CONF_ETHADDR3,ETHERNET_CONF_ETHADDR4,ETHERNET_CONF_ETHADDR5 };

/*-----------------------------------------------------------*/

/* See the header file for descriptions of public functions. */

/*
 * Prototype for the MACB interrupt function - called by the asm wrapper.
 */

__attribute__((__interrupt__)) void vMACB_ISR(void);


/*
 * Initialise both the Tx and Rx descriptors used by the MACB.
 */
static void prvSetupDescriptors(volatile avr32_macb_t *macb);

//
// Restore ownership of all Rx buffers to the MACB.
//
static void vResetMacbRxFrames( void );

/*
 * Write our MAC address into the MACB.
 */
static void prvSetupMACAddress(volatile avr32_macb_t *macb);

/*
 * Configure the MACB for interrupts.
 */
static void prvSetupMACBInterrupt(volatile avr32_macb_t *macb);

/*
 * Some initialisation functions.
 */
static Bool prvProbePHY(volatile avr32_macb_t *macb);
static unsigned long ulReadMDIO(volatile avr32_macb_t *macb, unsigned short usAddress);
static void vWriteMDIO(volatile avr32_macb_t *macb, unsigned short usAddress, unsigned short usValue);


/* Holds the index to the next buffer from which data will be read. */
volatile unsigned long ulNextRxBuffer = 0;


long lMACBSend(volatile avr32_macb_t *macb, const void *pvFrom, unsigned long ulLength, long lEndOfFrame)
{
  const unsigned char *pcFrom = pvFrom;
  static unsigned long uxTxBufferIndex = 0;
  void *pcBuffer;
  unsigned long ulLastBuffer, ulDataBuffered = 0, ulDataRemainingToSend, ulLengthToSend;

  /* If the length of data to be transmitted is greater than each individual
  transmit buffer then the data will be split into more than one buffer.
  Loop until the entire length has been buffered. */
  while( ulDataBuffered < ulLength )
  {
    // Is a buffer available ?
    while( !( xTxDescriptors[ uxTxBufferIndex ].U_Status.status & AVR32_TRANSMIT_OK ) )
    {
      // There is no room to write the Tx data to the Tx buffer.
      // Wait a short while, then try again.
      __asm__ __volatile__ ("nop");
    }

    portENTER_CRITICAL();
    {
      // Get the address of the buffer from the descriptor,
      // then copy the data into the buffer.
      pcBuffer = ( void * ) xTxDescriptors[ uxTxBufferIndex ].addr;

      // How much can we write to the buffer ?
      ulDataRemainingToSend = ulLength - ulDataBuffered;
      if( ulDataRemainingToSend <= ETHERNET_CONF_TX_BUFFER_SIZE )
      {
        // We can write all the remaining bytes.
        ulLengthToSend = ulDataRemainingToSend;
      }
      else
      {
        // We can't write more than ETH_TX_BUFFER_SIZE in one go.
        ulLengthToSend = ETHERNET_CONF_TX_BUFFER_SIZE;
      }
      // Copy the data into the buffer.
      memcpy( pcBuffer, &( pcFrom[ ulDataBuffered ] ), ulLengthToSend );
      ulDataBuffered += ulLengthToSend;
      // Is this the last data for the frame ?
      if( lEndOfFrame && ( ulDataBuffered >= ulLength ) )
      {
        // No more data remains for this frame so we can start the transmission.
        ulLastBuffer = AVR32_LAST_BUFFER;
      }
      else
      {
        // More data to come for this frame.
        ulLastBuffer = 0;
      }
      // Fill out the necessary in the descriptor to get the data sent,
      // then move to the next descriptor, wrapping if necessary.
      if( uxTxBufferIndex >= ( ETHERNET_CONF_NB_TX_BUFFERS - 1 ) )
      {
        xTxDescriptors[ uxTxBufferIndex ].U_Status.status =   ( ulLengthToSend & ( unsigned long ) AVR32_LENGTH_FRAME )
                                    | ulLastBuffer
                                    | AVR32_TRANSMIT_WRAP;
        uxTxBufferIndex = 0;
      } else
      {
        xTxDescriptors[ uxTxBufferIndex ].U_Status.status =   ( ulLengthToSend & ( unsigned long ) AVR32_LENGTH_FRAME )
                                    | ulLastBuffer;
        uxTxBufferIndex++;
      }
      /* If this is the last buffer to be sent for this frame we can
         start the transmission. */
      if( ulLastBuffer )
        macb->ncr |=  AVR32_MACB_TSTART_MASK;

    }
    portEXIT_CRITICAL();
  }

  return PASS;
}


unsigned long ulMACBInputLength(void)
{
  register unsigned long ulIndex , ulLength = 0;
  unsigned int uiTemp;
  volatile unsigned long ulEventStatus;

  // Check if the MACB encountered a problem.
  ulEventStatus = AVR32_MACB.rsr;
  if( ulEventStatus & AVR32_MACB_RSR_BNA_MASK )
  {     // MACB couldn't get ownership of a buffer. This could typically
        // happen if the total numbers of Rx buffers is tailored too small
        // for a noisy network with big frames.
        // We might as well restore ownership of all buffers to the MACB to
        // restart from a clean state.
    vResetMacbRxFrames();
    return( ulLength );
  }

  // Skip any fragments.  We are looking for the first buffer that contains
  // data and has the SOF (start of frame) bit set.
  while( ( xRxDescriptors[ ulNextRxBuffer ].addr & AVR32_OWNERSHIP_BIT )
        && !( xRxDescriptors[ ulNextRxBuffer ].U_Status.status & AVR32_SOF ) )
  {
    // Ignoring this buffer.  Mark it as free again.
    uiTemp = xRxDescriptors[ ulNextRxBuffer ].addr;
    xRxDescriptors[ ulNextRxBuffer ].addr = uiTemp & ~( AVR32_OWNERSHIP_BIT );
    ulNextRxBuffer++;
    if( ulNextRxBuffer >= ETHERNET_CONF_NB_RX_BUFFERS )
    {
      ulNextRxBuffer = 0;
    }
  }

  // We are going to walk through the descriptors that make up this frame,
  // but don't want to alter ulNextRxBuffer as this would prevent vMACBRead()
  // from finding the data.  Therefore use a copy of ulNextRxBuffer instead.
  ulIndex = ulNextRxBuffer;

  // Walk through the descriptors until we find the last buffer for this frame.
  // The last buffer will give us the length of the entire frame.
  while ( xRxDescriptors[ ulIndex ].addr & AVR32_OWNERSHIP_BIT )
  {
    ulLength = xRxDescriptors[ ulIndex ].U_Status.status & AVR32_LENGTH_FRAME;
    if (ulLength) break; //return ulLength

    // Increment to the next buffer, wrapping if necessary.
    if( ++ulIndex >= ETHERNET_CONF_NB_RX_BUFFERS ) ulIndex = 0;

    // Is the descriptor valid?
    if (!(xRxDescriptors[ ulIndex ].addr & AVR32_OWNERSHIP_BIT)) break; //return 0

    // Is it a SOF? If so, the head packet is bad and should be discarded
    if (xRxDescriptors[ ulIndex ].U_Status.status & AVR32_SOF)
    {
      // Mark the buffers of the CURRENT, FAULTY packet available.
      unsigned int i = ulNextRxBuffer;
      do{
        // Ignore the faulty frame. Mark its buffers as owned by the MACB.
        uiTemp = xRxDescriptors[ i ].addr;
        xRxDescriptors[ i ].addr = uiTemp & ~(AVR32_OWNERSHIP_BIT);
        if (++i>=ETHERNET_CONF_NB_RX_BUFFERS) i=0;
      }while (i!=ulIndex);
      ulNextRxBuffer=ulIndex;
      // We have the start of a new packet, look at that one instead.
    }
  }
  return ulLength;
}
/*-----------------------------------------------------------*/

void vMACBRead(void *pvTo, unsigned long ulSectionLength, unsigned long ulTotalFrameLength)
{
  unsigned char *pcTo = pvTo;
  static unsigned long ulSectionBytesReadSoFar = 0, ulBufferPosition = 0, ulFrameBytesReadSoFar = 0;
  static const unsigned char *pcSource;
  register unsigned long ulBytesRemainingInBuffer, ulRemainingSectionBytes;
  unsigned int uiTemp;

  // Read ulSectionLength bytes from the Rx buffers.
  // This is not necessarily any correspondence between the length of our Rx buffers,
  // and the length of the data we are returning or the length of the data being requested.
  // Therefore, between calls  we have to remember not only which buffer we are currently
  // processing, but our position within that buffer.
  // This would be greatly simplified if PBUF_POOL_BUFSIZE could be guaranteed to be greater
  // than the size of each Rx buffer, and that memory fragmentation did not occur.

  // This function should only be called after a call to ulMACBInputLength().
  // This will ensure ulNextRxBuffer is set to the correct buffer. */

  // vMACBRead is called with pcTo set to NULL to indicate that we are about
  // to read a new frame.  Any fragments remaining in the frame we were
  // processing during the last call should be dropped.
  if( pcTo == NULL )
  {
    // How many bytes are indicated as being in this buffer?
    // If none then the buffer is completely full and the frame is contained within more
    // than one buffer.
    // Reset our state variables ready for the next read from this buffer.
    pcSource = ( unsigned char * )( xRxDescriptors[ ulNextRxBuffer ].addr & ADDRESS_MASK );
    ulFrameBytesReadSoFar = ( unsigned long ) 0;
    ulBufferPosition = ( unsigned long ) 0;
  }
  else
  {
    // Loop until we have obtained the required amount of data.
    ulSectionBytesReadSoFar = 0;
    while( ulSectionBytesReadSoFar < ulSectionLength )
    {
      // We may have already read some data from this buffer.
      // How much data remains in the buffer?
      ulBytesRemainingInBuffer = ( RX_BUFFER_SIZE - ulBufferPosition );

      // How many more bytes do we need to read before we have the
      // required amount of data?
      ulRemainingSectionBytes = ulSectionLength - ulSectionBytesReadSoFar;

      // Do we want more data than remains in the buffer?
      if( ulRemainingSectionBytes > ulBytesRemainingInBuffer )
      {
        // We want more data than remains in the buffer so we can
        // write the remains of the buffer to the destination, then move
        // onto the next buffer to get the rest.
        memcpy( &( pcTo[ ulSectionBytesReadSoFar ] ), &( pcSource[ ulBufferPosition ] ), ulBytesRemainingInBuffer );
        ulSectionBytesReadSoFar += ulBytesRemainingInBuffer;
        ulFrameBytesReadSoFar += ulBytesRemainingInBuffer;

        // Mark the buffer as free again.
        uiTemp = xRxDescriptors[ ulNextRxBuffer ].addr;
        xRxDescriptors[ ulNextRxBuffer ].addr = uiTemp & ~( AVR32_OWNERSHIP_BIT );
        // Move onto the next buffer.
        ulNextRxBuffer++;

        if( ulNextRxBuffer >= ETHERNET_CONF_NB_RX_BUFFERS )
        {
          ulNextRxBuffer = ( unsigned long ) 0;
        }

        // Reset the variables for the new buffer.
        pcSource = ( unsigned char * )( xRxDescriptors[ ulNextRxBuffer ].addr & ADDRESS_MASK );
        ulBufferPosition = ( unsigned long ) 0;
      }
      else
      {
        // We have enough data in this buffer to send back.
        // Read out enough data and remember how far we read up to.
        memcpy( &( pcTo[ ulSectionBytesReadSoFar ] ), &( pcSource[ ulBufferPosition ] ), ulRemainingSectionBytes );

        // There may be more data in this buffer yet.
        // Increment our position in this buffer past the data we have just read.
        ulBufferPosition += ulRemainingSectionBytes;
        ulSectionBytesReadSoFar += ulRemainingSectionBytes;
        ulFrameBytesReadSoFar += ulRemainingSectionBytes;

        // Have we now finished with this buffer?
        if( ( ulBufferPosition >= RX_BUFFER_SIZE ) || ( ulFrameBytesReadSoFar >= ulTotalFrameLength ) )
        {
          // Mark the buffer as free again.
          uiTemp = xRxDescriptors[ ulNextRxBuffer ].addr;
          xRxDescriptors[ ulNextRxBuffer ].addr = uiTemp & ~( AVR32_OWNERSHIP_BIT );
          // Move onto the next buffer.
          ulNextRxBuffer++;

          if( ulNextRxBuffer >= ETHERNET_CONF_NB_RX_BUFFERS )
          {
            ulNextRxBuffer = 0;
          }

          pcSource = ( unsigned char * )( xRxDescriptors[ ulNextRxBuffer ].addr & ADDRESS_MASK );
          ulBufferPosition = 0;
        }
      }
    }
  }
}

/*-----------------------------------------------------------*/
void vMACBFlushCurrentPacket(unsigned long ulTotalFrameLength)
{
   unsigned int   uiTemp;
   long int lTotalFrameLen = (long int)ulTotalFrameLength;

   while( lTotalFrameLen > 0 )
   {
      // Ignoring this buffer.  Mark it as free again.
      uiTemp = xRxDescriptors[ ulNextRxBuffer ].addr;
      xRxDescriptors[ ulNextRxBuffer ].addr = uiTemp & ~( AVR32_OWNERSHIP_BIT );

      // Move on to the next buffer.
      ulNextRxBuffer++;
      ulNextRxBuffer = ulNextRxBuffer%ETHERNET_CONF_NB_RX_BUFFERS;

      lTotalFrameLen -= RX_BUFFER_SIZE;
   }
}


/*-----------------------------------------------------------*/
void vMACBSetMACAddress(const unsigned char *MACAddress)
{
  memcpy(cMACAddress, MACAddress, sizeof(cMACAddress));
}

Bool xMACBInit(volatile avr32_macb_t *macb)
{
  Bool global_interrupt_enabled = Is_global_interrupt_enabled();
  volatile unsigned long status;

  //vDisableMACBOperations(macb);

  // set up registers
  macb->ncr = 0;
  macb->tsr = ~0UL;
  macb->rsr = ~0UL;

  if (global_interrupt_enabled) Disable_global_interrupt();
  macb->idr = ~0UL;
  status = macb->isr;
  if (global_interrupt_enabled) Enable_global_interrupt();

  // RMII used, set 0 to the USRIO Register
  macb->usrio &= ~AVR32_MACB_RMII_MASK;

  // Load our MAC address into the MACB.
  prvSetupMACAddress(macb);

  // Setup the buffers and descriptors.
  prvSetupDescriptors(macb);

#if ETHERNET_CONF_SYSTEM_CLOCK <= 20000000
  macb->ncfgr |= (AVR32_MACB_NCFGR_CLK_DIV8 << AVR32_MACB_NCFGR_CLK_OFFSET);
#elif ETHERNET_CONF_SYSTEM_CLOCK <= 40000000
  macb->ncfgr |= (AVR32_MACB_NCFGR_CLK_DIV16 << AVR32_MACB_NCFGR_CLK_OFFSET);
#elif ETHERNET_CONF_SYSTEM_CLOCK <= 80000000
  macb->ncfgr |= AVR32_MACB_NCFGR_CLK_DIV32 << AVR32_MACB_NCFGR_CLK_OFFSET;
#elif ETHERNET_CONF_SYSTEM_CLOCK <= 160000000
  macb->ncfgr |= AVR32_MACB_NCFGR_CLK_DIV64 << AVR32_MACB_NCFGR_CLK_OFFSET;
#else
# error System clock too fast
#endif

  // Are we connected?
  if( prvProbePHY(macb) == TRUE )
  {
    // Enable the interrupt!
    portENTER_CRITICAL();
    {
      prvSetupMACBInterrupt(macb);
    }
    portEXIT_CRITICAL();
    // Enable Rx and Tx, plus the stats register.
    macb->ncr = AVR32_MACB_NCR_TE_MASK | AVR32_MACB_NCR_RE_MASK;
    return (TRUE);
  }
  return (FALSE);
}

void vDisableMACBOperations (volatile avr32_macb_t *macb)
{
  Bool global_interrupt_enabled = Is_global_interrupt_enabled();

  // write the MACB control register : disable Tx & Rx
  macb->ncr &= ~((1 << AVR32_MACB_RE_OFFSET) | (1 << AVR32_MACB_TE_OFFSET));

  // We no more want to interrupt on Rx and Tx events.
  if (global_interrupt_enabled) Disable_global_interrupt();
  macb->idr = AVR32_MACB_IER_RCOMP_MASK | AVR32_MACB_IER_TCOMP_MASK;
  macb->isr;
  if (global_interrupt_enabled) Enable_global_interrupt();
}


void vClearMACBTxBuffer(void)
{
  static unsigned long uxNextBufferToClear = 0;

  // Called on Tx interrupt events to set the AVR32_TRANSMIT_OK bit in each
  // Tx buffer within the frame just transmitted.  This marks all the buffers
  // as available again.

  // The first buffer in the frame should have the bit set automatically. */
  if( xTxDescriptors[ uxNextBufferToClear ].U_Status.status & AVR32_TRANSMIT_OK )
  {
    // Loop through the other buffers in the frame.
    while( !( xTxDescriptors[ uxNextBufferToClear ].U_Status.status & AVR32_LAST_BUFFER ) )
    {
      uxNextBufferToClear++;

      if( uxNextBufferToClear >= ETHERNET_CONF_NB_TX_BUFFERS )
      {
        uxNextBufferToClear = 0;
      }

      xTxDescriptors[ uxNextBufferToClear ].U_Status.status |= AVR32_TRANSMIT_OK;
    }

    // Start with the next buffer the next time a Tx interrupt is called.
    uxNextBufferToClear++;

    // Do we need to wrap back to the first buffer?
    if( uxNextBufferToClear >= ETHERNET_CONF_NB_TX_BUFFERS )
    {
      uxNextBufferToClear = 0;
    }
  }
}

static void prvSetupDescriptors(volatile avr32_macb_t *macb)
{
  unsigned long xIndex;
  unsigned long ulAddress;

  // Initialise xRxDescriptors descriptor.
  for( xIndex = 0; xIndex < ETHERNET_CONF_NB_RX_BUFFERS; ++xIndex )
  {
    // Calculate the address of the nth buffer within the array.
    ulAddress = ( unsigned long )( pcRxBuffer + ( xIndex * RX_BUFFER_SIZE ) );

    // Write the buffer address into the descriptor.
    // The DMA will place the data at this address when this descriptor is being used.
    // No need to mask off the bottom bits of the address (these have special meaning
    // for the MACB) because pcRxBuffer is 4Bytes-aligned.
    xRxDescriptors[ xIndex ].addr = ulAddress;
  }

  // The last buffer has the wrap bit set so the MACB knows to wrap back
  // to the first buffer.
  xRxDescriptors[ ETHERNET_CONF_NB_RX_BUFFERS - 1 ].addr |= RX_WRAP_BIT;

  // Initialise xTxDescriptors.
  for( xIndex = 0; xIndex < ETHERNET_CONF_NB_TX_BUFFERS; ++xIndex )
  {
    // Calculate the address of the nth buffer within the array.
    ulAddress = ( unsigned long )( pcTxBuffer + ( xIndex * ETHERNET_CONF_TX_BUFFER_SIZE ) );

    // Write the buffer address into the descriptor.
    // The DMA will read data from here when the descriptor is being used.
    xTxDescriptors[ xIndex ].addr = ulAddress;
    xTxDescriptors[ xIndex ].U_Status.status = AVR32_TRANSMIT_OK;
  }

  // The last buffer has the wrap bit set so the MACB knows to wrap back
  // to the first buffer.
  xTxDescriptors[ ETHERNET_CONF_NB_TX_BUFFERS - 1 ].U_Status.status = AVR32_TRANSMIT_WRAP | AVR32_TRANSMIT_OK;

  // Tell the MACB where to find the descriptors.
  macb->rbqp =   ( unsigned long )xRxDescriptors;
  macb->tbqp =   ( unsigned long )xTxDescriptors;

  // Do not copy the FCS field of received frames to memory.
  macb->ncfgr |= ( AVR32_MACB_NCFGR_DRFCS_MASK );

}

//!
//! \brief Restore ownership of all Rx buffers to the MACB.
//!
static void vResetMacbRxFrames( void )
{
   register unsigned long  ulIndex;
   unsigned int            uiTemp;


   // Disable MACB frame reception.
   AVR32_MACB.ncr &= ~(AVR32_MACB_NCR_RE_MASK);

   // Restore ownership of all Rx buffers to the MACB.
   for( ulIndex = 0; ulIndex < ETHERNET_CONF_NB_RX_BUFFERS; ++ulIndex )
   {
      // Mark the buffer as owned by the MACB.
      uiTemp = xRxDescriptors[ ulIndex ].addr;
      xRxDescriptors[ ulIndex ].addr = uiTemp & ~( AVR32_OWNERSHIP_BIT );
   }

   // Reset the Buffer-not-available bit and the overrun bit.
   AVR32_MACB.rsr = AVR32_MACB_RSR_BNA_MASK | AVR32_MACB_RSR_OVR_MASK;  // Clear
   AVR32_MACB.rsr; // We read to force the previous operation.

   // Reset the MACB starting point.
   AVR32_MACB.rbqp = ( unsigned long )xRxDescriptors;

   // Reset the index to the next buffer from which data will be read.
   ulNextRxBuffer = 0;

   // Enable MACB frame reception.
   AVR32_MACB.ncr |= AVR32_MACB_NCR_RE_MASK;
}


static void prvSetupMACAddress(volatile avr32_macb_t *macb)
{
  // Must be written SA1L then SA1H.
  macb->sa1b =  ( ( unsigned long ) cMACAddress[ 3 ] << 24 ) |
                ( ( unsigned long ) cMACAddress[ 2 ] << 16 ) |
                ( ( unsigned long ) cMACAddress[ 1 ] << 8  ) |
                                    cMACAddress[ 0 ];

  macb->sa1t =  ( ( unsigned long ) cMACAddress[ 5 ] << 8 ) |
                                    cMACAddress[ 4 ];
}

static void prvSetupMACBInterrupt(volatile avr32_macb_t *macb)
{

    // Setup the interrupt for MACB.
    // Register the interrupt handler to the interrupt controller at interrupt level 2
    INTC_register_interrupt((__int_handler)&vMACB_ISR, AVR32_MACB_IRQ, AVR32_INTC_INT2);

    // We want to interrupt on Rx and Tx events
    macb->ier = AVR32_MACB_IER_RCOMP_MASK | AVR32_MACB_IER_TCOMP_MASK ;
}

/*! Read a register on MDIO bus (access to the PHY)
 *         This function is looping until PHY gets ready
 *
 * \param macb         Input. instance of the MACB to use
 * \param usAddress    Input. register to set.
 *
 * \return unsigned long data that has been read
 */
static unsigned long ulReadMDIO(volatile avr32_macb_t *macb, unsigned short usAddress)
{
  unsigned long value, status;
  u16 timeout = 0;

  // initiate transaction : enable management port
  macb->ncr |= AVR32_MACB_NCR_MPE_MASK;
  // Write the PHY configuration frame to the MAN register
  macb->man = (AVR32_MACB_SOF_MASK & (0x01<<AVR32_MACB_SOF_OFFSET))  // SOF
            | (2 << AVR32_MACB_CODE_OFFSET)                          // Code
            | (2 << AVR32_MACB_RW_OFFSET)                            // Read operation
            | ((ETHERNET_CONF_PHY_ADDR & 0x1f) << AVR32_MACB_PHYA_OFFSET)  // Phy Add
            | (usAddress << AVR32_MACB_REGA_OFFSET);                 // Reg Add
  // wait for PHY to be ready
  do {
    status = macb->nsr;
  } while (!(status & AVR32_MACB_NSR_IDLE_MASK) && (timeout++ < TIMEOUT));
  // read the register value in maintenance register
  value = macb->man & 0x0000ffff;
  // disable management port
  macb->ncr &= ~AVR32_MACB_NCR_MPE_MASK;
  // return the read value
  return (value);
}

/*! Write a given value to a register on MDIO bus (access to the PHY)
 *         This function is looping until PHY gets ready
 *
 * \param *macb        Input. instance of the MACB to use
 * \param usAddress    Input. register to set.
 * \param usValue      Input. value to write.
 *
 */
static void vWriteMDIO(volatile avr32_macb_t *macb, unsigned short usAddress, unsigned short usValue)
{
  unsigned long status;
  u16 timeout = 0;

  // initiate transaction : enable management port
  macb->ncr |= AVR32_MACB_NCR_MPE_MASK;
  // Write the PHY configuration frame to the MAN register
  macb->man = (( AVR32_MACB_SOF_MASK & (0x01<<AVR32_MACB_SOF_OFFSET)) // SOF
             | (2 << AVR32_MACB_CODE_OFFSET)                          // Code
             | (1 << AVR32_MACB_RW_OFFSET)                            // Write operation
             | ((ETHERNET_CONF_PHY_ADDR & 0x1f) << AVR32_MACB_PHYA_OFFSET)  // Phy Add
             | (usAddress << AVR32_MACB_REGA_OFFSET))                 // Reg Add
             | (usValue & 0xffff);                                    // Data
  // wait for PHY to be ready
  do {
    status = macb->nsr;
  } while (!(status & AVR32_MACB_NSR_IDLE_MASK) && (timeout++ < TIMEOUT));
  // disable management port
  macb->ncr &= ~AVR32_MACB_NCR_MPE_MASK;
}

static Bool prvProbePHY(volatile avr32_macb_t *macb)
{
  volatile unsigned long mii_status, phy_ctrl;
  volatile unsigned long config;
  unsigned long upper, lower, mode, advertise, lpa;
  volatile unsigned long physID;

  // Read Phy Identifier register 1 & 2
  lower = ulReadMDIO(macb, PHY_PHYSID2);
  upper = ulReadMDIO(macb, PHY_PHYSID1);
  // get Phy ID, ignore Revision
  physID = ((upper << 16) & 0xFFFF0000) | (lower & 0xFFF0);
  // check if it match config
  if (physID == ETHERNET_CONF_PHY_ID)
  {
    // read RBR
    mode = ulReadMDIO(macb, PHY_RBR);
    // set RMII mode if not done
    if ((mode & RBR_RMII) != RBR_RMII)
    {
      // force RMII flag if strap options are wrong
      mode |= RBR_RMII;
      vWriteMDIO(macb, PHY_RBR, mode);
    }

    advertise = ADVERTISE_CSMA | ADVERTISE_ALL;
    // write advertise register
    vWriteMDIO(macb, PHY_ADVERTISE, advertise);
    // read Control register
    config = ulReadMDIO(macb, PHY_BMCR);
    // read Phy Control register
    phy_ctrl = ulReadMDIO(macb, PHY_PHYCR);
    // enable Auto MDIX
    phy_ctrl |= PHYCR_MDIX_EN;
    // reset auto-negociation capability
    config |= (BMCR_ANRESTART | BMCR_ANENABLE);
    // update Phy ctrl register
    vWriteMDIO(macb, PHY_PHYCR, phy_ctrl);

    // update ctrl register
    vWriteMDIO(macb, PHY_BMCR, config);

    // loop while link status isn't OK
    do {
      mii_status = ulReadMDIO(macb, PHY_BMSR);
    } while (!(mii_status & BMSR_LSTATUS));

    // read the LPA configuration of the PHY
    lpa = ulReadMDIO(macb, PHY_LPA);

    // read the MACB config register
    config = AVR32_MACB.ncfgr;

    // if 100MB needed
    if ((lpa & advertise) & (LPA_100HALF | LPA_100FULL))
      config |= AVR32_MACB_SPD_MASK;
    else
      config &= ~(AVR32_MACB_SPD_MASK);

    // if FULL DUPLEX needed
    if ((lpa & advertise) & (LPA_10FULL | LPA_100FULL))
      config |= AVR32_MACB_FD_MASK;
    else
      config &= ~(AVR32_MACB_FD_MASK);

    // write the MACB config register
    macb->ncfgr = config;

    return TRUE;
  }
  return FALSE;
}

/*
 * The MACB ISR.  Handles both Tx and Rx complete interrupts.
 */
__attribute__((__interrupt__)) void vMACB_ISR(void)
{
  // Variable definitions can be made now.
  volatile unsigned long ulIntStatus, ulEventStatus;

  // Find the cause of the interrupt.
  ulIntStatus = AVR32_MACB.isr;
  ulEventStatus = AVR32_MACB.rsr;

  if( ( ulIntStatus & AVR32_MACB_IDR_RCOMP_MASK ) || ( ulEventStatus & AVR32_MACB_REC_MASK ) )
  {
    // A frame has been received, signal the IP task so it can process
    // the Rx descriptors.
    elua_uip_mainloop();
    AVR32_MACB.rsr =  AVR32_MACB_REC_MASK;  // Clear
    AVR32_MACB.rsr; // Read to force the previous write
  }

  if( ulIntStatus & AVR32_MACB_TCOMP_MASK )
  {
    // A frame has been transmitted.  Mark all the buffers used by the
    // frame just transmitted as free again.
    vClearMACBTxBuffer();
    AVR32_MACB.tsr =  AVR32_MACB_TSR_COMP_MASK; // Clear
    AVR32_MACB.tsr; // Read to force the previous write
  }
}
#endif
