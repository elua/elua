// UART driver for LPC2888

#ifndef __UART_H__
#define __UART_H__

#include "platform.h"
#include "type.h"

u32 uart_init( u32 baud, int databits, int parity, int stopbits );
void uart_write( u8 data );
u8 uart_read();
int uart_read_nb();

#endif