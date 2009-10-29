/*****************************************************************************
 *   uart.h:  Header file for NXP LPC23xx Family Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.09.01  ver 1.00    Prelimnary version, first Release
 *
 *   Modified by BogdanM for eLua
******************************************************************************/

#ifndef __UART_H 
#define __UART_H

#include "type.h"

#define IER_RBR		0x01
#define IER_THRE	0x02
#define IER_RLS		0x04

#define IIR_PEND	0x01
#define IIR_RLS		0x03
#define IIR_RDA		0x02
#define IIR_CTI		0x06
#define IIR_THRE	0x01

#define LSR_RDR		0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80

// UART setup constants
enum
{
  UART_DATABITS_5 = 0,
  UART_DATABITS_6 = 1,
  UART_DATABITS_7 = 2,
  UART_DATABITS_8 = 3,
  UART_STOPBITS_1 = 0,
  UART_STOPBITS_2 = 4,
  UART_PARITY_ENABLE = 8,
  UART_PARITY_ODD = 0,
  UART_PARITY_EVEN = 1 << 4,
  UART_DLAB_ENABLE = 1 << 7,
  UART_FIFO_ENABLE = 1,
  UART_RXFIFO_RESET = 2,
  UART_TXFIFO_RESET = 4
};

#endif /* end __UART_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/

