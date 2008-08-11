/*****************************************************************************
 *   uart.c:  UART API file for NXP LPC288x Family Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.07.12  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#include "LPC288x.h"                        /* LPC288x definitions */
#include "type.h"
#include "target.h"
#include "irq.h"
#include "uart.h"
#include <reent.h>
#include <errno.h>
#include <string.h>

static char uart_rx_buf[ UART_BUFSIZE ];
volatile static int uart_rx_in, uart_rx_out;
int uart_tx_count;

// Pre-computed values for different baud rates, using Philip's 'baud rate calculator' XLS file
typedef struct 
{
  int udlm, udll, divaddval, mulval;
} BAUDDATA;

static const BAUDDATA baudinfo[] = 
{
  { 12, 53,   0, 2 },
  { 4,  226,  1, 4 },
  { 2,  113,  1, 4 },
  { 0,  217,  4, 5 },
  { 0,  217,  1, 5 },
  { 0,  71,   3, 8 },
  { 0,  47,   5, 13 },
  { 0,  24,   5, 14 },
};
// Local state information
static struct uart_data uartdata;

static void uart_isr(void) 
{
  u8 IIRValue, LSRValue;
  u8 dummy;
  
  IIRValue = UART_IIR;
    
  IIRValue >>= 1;       /* skip pending bit in IIR */
  IIRValue &= 0x07;       /* check bit 1~3, interrupt identification */
  
  if ( IIRValue == IIR_RLS )    /* Receive Line Status */
  {
    LSRValue = UART_LSR;
    /* Receive Line Status */
    if ( LSRValue & (LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) )
    {
      /* There are errors or break interrupt */
      /* Read LSR will clear the interrupt */
      dummy = UART_RBR;
      return;
    }
    if( LSRValue & LSR_RDR )   /* Receive Data Ready */      
    {
      dummy = UART_RBR;
      if( dummy != '\r' )
      {
        uart_rx_buf[ uart_rx_in ] = dummy;
        UART_INC_OP( uart_rx_in );
      }
    }
  }
  
  else if ( IIRValue == IIR_RDA )   /* Receive Data Available */
  {
    dummy = UART_RBR;
    if( dummy != '\r' )
    {
      uart_rx_buf[ uart_rx_in ] = dummy;
      UART_INC_OP( uart_rx_in );
    }
  }
  
  else if ( IIRValue == IIR_CTI )   /* Character timeout indicator */
  {
    dummy = UART_RBR;
  }  

}

// Utility function: write a char to the UART
static void uart_putchar( char c )
{
  if( uart_tx_count == TXBUFSIZE )
  {
    while( ( UART_LSR & LSR_THRE ) == 0 );
    uart_tx_count = 0;  
  }
  UART_THR = c;
  uart_tx_count ++;
}

// Utility function: return the number of bytes in the UART RX buffer
static int uart_bytes_received()
{
  return ( uart_rx_in + UART_BUFSIZE - uart_rx_out ) & UART_BUF_MASK;
}

// Utility function: read a char from the UART. This is a BLOCKING function.
static char uart_getchar()
{
  char c;
  
  while( uart_bytes_received() == 0 );
  c = uart_rx_buf[ uart_rx_out ];
  UART_INC_OP( uart_rx_out );
  return c;
}

// 'read' function for UART
static long uart_read( struct _reent *r, int fd, char *ptr, int len )
{
  int i, c;
  
  // Check file number
  if( ( fd < DM_STDIN_NUM ) || ( fd > DM_STDERR_NUM ) )
  {
    r->_errno = EBADF;
    return -1;
  }  
  if( fd != DM_STDIN_NUM )
  {
    r->_errno = EINVAL;
    return -1;
  }      
  
  for( i = 0; i < len; i ++ )
  {
    c = uart_getchar();
    ptr[ i ] = c;
    if( c == '\n' )
      return i + 1;    
  }
  return len;
}

// 'write' function for UART
static long uart_write( struct _reent *r, int fd, const char *ptr, int len )
{
  int i;
    
  // Check file number
  if( ( fd < DM_STDIN_NUM ) || ( fd > DM_STDERR_NUM ) )
  {
    r->_errno = EBADF;
    return -1;
  }    
  if( ( fd != DM_STDOUT_NUM ) && ( fd != DM_STDERR_NUM ) )
  {
    r->_errno = EINVAL;
    return -1;
  }  
  for( i = 0; i < len; i ++ )
    uart_putchar( ptr[ i ] );
  return len;
}

// 'ioctl' function for UART
static int uart_ioctl( struct _reent *r, int file, unsigned long request, void *ptr )
{
  struct uart_data *pdata;
  int temp, oldint;
  const BAUDDATA* pbaud;
  
  // Check file number
  if( ( file < DM_STDIN_NUM ) || ( file > DM_STDERR_NUM ) )
  {
    r->_errno = EBADF;
    return -1;
  }
  
  // Check request
  if( ( request < IOCTL_UART_FIRST ) || ( request > IOCTL_UART_LAST ) )
  {
    r->_errno = EINVAL;
    return -1;
  }
  
  // Serve request
  switch( request )
  {
    case IOCTL_UART_SET:
      // Set UART data
      pdata = ( struct uart_data* )ptr;
      if( pdata->baud < 0 || pdata->baud > UART_BAUD_115200 )
      {
        r->_errno = EINVAL;
        return -1;
      }
      memcpy( &uartdata, pdata, sizeof( struct uart_data ) );
      
      // Disable all interrupts and initialize FIFOs
      temp = UART_IIR;
      oldint = UART_IER;
      UART_IER = 0;
      UART_FCR = 0x0F;
      
      // Setup format and enable access to divisor latches
      temp = pdata->databits == UART_8_DATABITS ? 3 : 2;
      temp |= ( pdata->parity == UART_ODD_PARITY ) ? 0x08 : ( ( pdata->parity == UART_EVEN_PARITY ) ? 0x18 : 0 );
      UART_LCR = temp | 0x80;
      
      // Write divisor latches and fractional data
      pbaud = baudinfo + pdata->baud;
      UART_DLM = pbaud->udlm;
      UART_DLL = pbaud->udll;
      UART_FDR = ( pbaud->mulval << 4 ) + pbaud->divaddval;
      
      // Re-enable UART and its interrupts
      UART_LCR = temp;
      UART_IER = oldint;
      break;
      
    case IOCTL_UART_GET:
      memcpy( ptr, &uartdata, sizeof( struct uart_data ) );
      break;      
      
    case IOCTL_UART_FLUSH_IN:
      while( uart_rx_in != uart_rx_out )
        uart_getchar();
      break;
      
    case IOCTL_UART_FLUSH_OUT:
      while( ( UART_LSR & LSR_THRE ) == 0 );
      uart_tx_count = 0;
      break;
  }
  return 0;
}

// Our UART device descriptor structure
static DM_DEVICE uart_device = 
{
  UART_DEV_NAME,
  NULL,                 // we don't have 'open' on UART
  NULL,                 // we don't hace 'close' on UART
  uart_write,
  uart_read,
  uart_ioctl
};

// Initialize the UART device with the data found in the 'pdata' structure
// Returns a pointer to the UART's 'device' structure if OK, NULL for error
DM_DEVICE* uart_init( struct uart_data* pdata )
{  
  // Call the IOCTL function directly to set UART parameters
  if( uart_ioctl( _REENT, DM_STDIN_NUM, IOCTL_UART_SET, pdata ) != 0 )
    return NULL;
   
  // Install UART interrupt handler
  if ( install_IRQ( 12, 1, uart_isr ) == FALSE )
    return NULL;
  INT_REQ12 = ( 1<<28 ) | ( 1<<27 ) | ( 1<<26 ) | ( 1<<16 ) | 0x1; 
  UART_IER = IER_RBR | IER_RLS;
  return &uart_device;
}

// Return UART's descriptor
DM_DEVICE* platform_get_uart_desc()
{
  return &uart_device;
}
