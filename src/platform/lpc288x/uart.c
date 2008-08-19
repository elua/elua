// UART driver for LPC2888

#include "lpc288x.h"                        /* LPC288x definitions */
#include "type.h"
#include "target.h"
#include "uart.h"
#include <reent.h>
#include <errno.h>
#include <string.h>
#include "utils.h"

// Structure with UART baud/parameters 
// Computed used Philip's "baud rate calculator" XLS file

typedef struct 
{
  unsigned baud;
  unsigned udlm, udll, divaddval, mulval;
} UART_BAUDDATA;

static const UART_BAUDDATA uart_baudinfo[] = 
{
  {   1200, 12, 53,   0, 2  },
  {   2400, 4,  226,  1, 4  },
  {   4800, 2,  113,  1, 4  },
  {   9600, 0,  217,  4, 5  },
  {  14400, 0,  217,  1, 5  },
  {  38400, 0,  71,   3, 8  },
  {  57600, 0,  47,   5, 13 },
  { 115200, 0,  24,   5, 14 }
};

// Write a character to the UART
void uart_write( u8 data )
{
  while( ( UART_LSR & LSR_THRE ) == 0 );
  UART_THR = data;
}

// Read a character from the UART (blocking)
u8 uart_read()
{
  while( ( UART_LSR & LSR_RDR ) == 0 );
  return UART_RBR & 0xFF;
}

// Read a character from the UART (non-blocking)
int uart_read_nb()
{
  if( ( UART_LSR & LSR_RDR ) == 0 )
    return -1;
  else
    return UART_RBR & 0xFF;
}

// Initialize UART
u32 uart_init( u32 baud, int databits, int parity, int stopbits )
{
  unsigned i, minpos;
  const UART_BAUDDATA* pdata;
  u32 temp;
  
  // Setup I/O ports
  // RXD is P6.0, TXD is P6.1
  MODE0S_6 = 3;
  MODE1C_6 = 3; 
  
  // Find correct baud
  for( i = minpos = 0; i < sizeof( uart_baudinfo ) / sizeof( UART_BAUDDATA ); i ++ )
    if( ABSDIFF( baud, uart_baudinfo[ i ].baud ) < ABSDIFF( baud, uart_baudinfo[ minpos ].baud ) )
      minpos = i;
  pdata = uart_baudinfo + minpos;
      
  // Disable all interrupts and initialize FIFOs
  UART_IER = 0;
  UART_FCR = 0x0F;
  
  // Setup format and enable access to divisor latches
  temp = databits - 5;
  if( stopbits == PLATFORM_UART_STOPBITS_2 || stopbits == PLATFORM_UART_STOPBITS_1_5 )
    temp |= 1 << 2;
  if( parity != PLATFORM_UART_PARITY_NONE )
  {
    temp |= 1 << 3;
    if( parity == PLATFORM_UART_PARITY_EVEN )
      temp |= 1 << 4;
  }
  UART_LCR = temp | 0x80;
  
  // Write divisor latches and fractional data
  UART_DLM = pdata->udlm;
  UART_DLL = pdata->udll;
  UART_FDR = ( pdata->mulval << 4 ) + pdata->divaddval;
  
  // Re-enable UART and its interrupts
  UART_LCR = temp;
      
  return pdata->baud;
}
