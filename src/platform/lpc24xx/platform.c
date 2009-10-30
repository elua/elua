// Platform-dependent functions

#include "platform.h"
#include "type.h"
#include "devman.h"
#include "genstd.h"
#include "stacks.h"
#include <reent.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "utils.h"
#include "common.h"
#include "platform_conf.h"

// Platform includes
#include "LPC23xx.h"                        /* LPC23xx/24xx definitions */
#include "target.h"
#include "irq.h"
#include "uart.h"

// ****************************************************************************
// Platform initialization

int platform_init()
{
  // [TODO] the rest of initialization must go here

  // Setup GPIO1 and GPIO1 in fast mode
  SCS |= 1;

  // Initialize UART
  platform_uart_setup( CON_UART_ID, CON_UART_SPEED, 8, PLATFORM_UART_PARITY_NONE, PLATFORM_UART_STOPBITS_1 );

  // Common platform initialiation code
  cmn_platform_init();

  return PLATFORM_OK;
} 

// ****************************************************************************
// PIO section

static const u32 pio_fiodir[ NUM_PIO ] = { ( u32 )&FIO0DIR, ( u32 )&FIO1DIR, ( u32 )&FIO2DIR, ( u32 )&FIO3DIR, ( u32 )&FIO4DIR };
static const u32 pio_fiopin[ NUM_PIO ] = { ( u32 )&FIO0PIN, ( u32 )&FIO1PIN, ( u32 )&FIO2PIN, ( u32 )&FIO3PIN, ( u32 )&FIO4PIN };
static const u32 pio_fioset[ NUM_PIO ] = { ( u32 )&FIO0SET, ( u32 )&FIO1SET, ( u32 )&FIO2SET, ( u32 )&FIO3SET, ( u32 )&FIO4SET };
static const u32 pio_fioclr[ NUM_PIO ] = { ( u32 )&FIO0CLR, ( u32 )&FIO1CLR, ( u32 )&FIO2CLR, ( u32 )&FIO3CLR, ( u32 )&FIO4CLR };

// The platform I/O functions
pio_type platform_pio_op( unsigned port, pio_type pinmask, int op )
{
  pio_type retval = 1;
  PREG FIOxDIR = ( PREG )pio_fiodir[ port ];
  PREG FIOxPIN = ( PREG )pio_fiopin[ port ];
  PREG FIOxSET = ( PREG )pio_fioset[ port ];
  PREG FIOxCLR = ( PREG )pio_fioclr[ port ];
   
  switch( op )
  {
    case PLATFORM_IO_PORT_SET_VALUE:   
      *FIOxPIN = pinmask;
      break;
      
    case PLATFORM_IO_PIN_SET:
      *FIOxSET = pinmask;
      break;
      
    case PLATFORM_IO_PIN_CLEAR:
      *FIOxCLR = pinmask;
      break;
      
    case PLATFORM_IO_PORT_DIR_OUTPUT:
      *FIOxDIR = 0xFFFFFFFF;
      break;    

    case PLATFORM_IO_PIN_DIR_OUTPUT:
      *FIOxDIR |= pinmask;
      break;
      
    case PLATFORM_IO_PORT_DIR_INPUT:
      *FIOxDIR = 0;
      break;

    case PLATFORM_IO_PIN_DIR_INPUT:
      *FIOxDIR &= ~pinmask;
      break;    
            
    case PLATFORM_IO_PORT_GET_VALUE:
      retval = *FIOxPIN;
      break;
      
    case PLATFORM_IO_PIN_GET:
      retval =( *FIOxPIN & pinmask ) ? 1 : 0;
      break;
      
    default:
      retval = 0;
      break;
  }
  return retval;
}


// ****************************************************************************
// UART section

// UART0: Rx = P0.3, Tx = P0.2
// The other UARTs have assignable Rx/Tx pins and thus have to be configured
// by the user
static const u32 uart_lcr[ NUM_UART ] = { ( u32 )&U0LCR, ( u32 )&U1LCR, ( u32 )&U2LCR, ( u32 )&U3LCR };
static const u32 uart_dlm[ NUM_UART ] = { ( u32 )&U0DLM, ( u32 )&U1DLM, ( u32 )&U2DLM, ( u32 )&U3DLM };
static const u32 uart_dll[ NUM_UART ] = { ( u32 )&U0DLL, ( u32 )&U1DLL, ( u32 )&U2DLL, ( u32 )&U3DLL };
static const u32 uart_fcr[ NUM_UART ] = { ( u32 )&U0FCR, ( u32 )&U1FCR, ( u32 )&U2FCR, ( u32 )&U3FCR };
static const u32 uart_thr[ NUM_UART ] = { ( u32 )&U0THR, ( u32 )&U1THR, ( u32 )&U2THR, ( u32 )&U3THR };
static const u32 uart_lsr[ NUM_UART ] = { ( u32 )&U0LSR, ( u32 )&U1LSR, ( u32 )&U2LSR, ( u32 )&U3LSR };
static const u32 uart_rbr[ NUM_UART ] = { ( u32 )&U0RBR, ( u32 )&U1RBR, ( u32 )&U2RBR, ( u32 )&U3RBR };
static const u32 uart_fdr[ NUM_UART ] = { ( u32 )&U0FDR, ( u32 )0,      ( u32 )&U2FDR, ( u32 )&U3FDR };

// Quick-and-dirty FP
#define INTTOFP(x)      ( (x) << 8 )
#define FLTTOFP(x)      ( ( u32 )( x * 256 ) )

// Table of FR versus DivAddVal/MulVal (taken from the datasheet)
typedef struct
{
  u32 fr;
  u8 divadd;
  u8 mul;
} uart_fractional_data;

static const uart_fractional_data fr_to_vals[] = 
{
  {FLTTOFP(1.000), 0, 1}, {FLTTOFP(1.067), 1, 15}, {FLTTOFP(1.071), 1, 14}, {FLTTOFP(1.077), 1, 13}, {FLTTOFP(1.083), 1, 12}, 
  {FLTTOFP(1.091), 1, 11}, {FLTTOFP(1.100), 1, 10}, {FLTTOFP(1.111), 1, 9}, {FLTTOFP(1.125), 1, 8}, {FLTTOFP(1.133), 2, 15}, 
  {FLTTOFP(1.143), 1, 7}, {FLTTOFP(1.154), 2, 13}, {FLTTOFP(1.167), 1, 6}, {FLTTOFP(1.182), 2, 11}, {FLTTOFP(1.200), 1, 5},
  {FLTTOFP(1.214), 3, 14}, {FLTTOFP(1.222), 2, 9}, {FLTTOFP(1.231), 3, 13}, {FLTTOFP(1.250), 1, 4}, {FLTTOFP(1.267), 4, 15},
  {FLTTOFP(1.273), 3, 11}, {FLTTOFP(1.286), 2, 7}, {FLTTOFP(1.300), 3, 10}, {FLTTOFP(1.308), 4, 13}, {FLTTOFP(1.333), 1, 3},
  {FLTTOFP(1.357), 5, 14}, {FLTTOFP(1.364), 4, 11}, {FLTTOFP(1.375), 3, 8}, {FLTTOFP(1.385), 5, 13}, {FLTTOFP(1.400), 2, 5},
  {FLTTOFP(1.417), 5, 12}, {FLTTOFP(1.429), 3, 7}, {FLTTOFP(1.444), 4, 9}, {FLTTOFP(1.455), 5, 11}, {FLTTOFP(1.462), 6, 13},
  {FLTTOFP(1.467), 7, 15}, {FLTTOFP(1.500), 1, 2}, {FLTTOFP(1.533), 8, 15}, {FLTTOFP(1.538), 7, 13}, {FLTTOFP(1.545), 6, 11},
  {FLTTOFP(1.556), 5, 9}, {FLTTOFP(1.571), 4, 7}, {FLTTOFP(1.583), 7, 12}, {FLTTOFP(1.600), 3, 5}, {FLTTOFP(1.615), 8, 13},
  {FLTTOFP(1.625), 5, 8}, {FLTTOFP(1.636), 7, 11}, {FLTTOFP(1.643), 9, 14}, {FLTTOFP(1.667), 2, 3}, {FLTTOFP(1.692), 9, 13},
  {FLTTOFP(1.700), 7, 10}, {FLTTOFP(1.714), 5, 7}, {FLTTOFP(1.727), 8, 11}, {FLTTOFP(1.733), 11, 15}, {FLTTOFP(1.750), 3, 4},
  {FLTTOFP(1.769), 10, 13}, {FLTTOFP(1.778), 7, 9}, {FLTTOFP(1.786), 11, 14}, {FLTTOFP(1.800), 4, 5}, {FLTTOFP(1.818), 9, 11},
  {FLTTOFP(1.833), 5, 6}, {FLTTOFP(1.846), 11, 13}, {FLTTOFP(1.857), 6, 7}, {FLTTOFP(1.867), 13, 15}, {FLTTOFP(1.875), 7, 8},
  {FLTTOFP(1.889), 8, 9}, {FLTTOFP(1.900), 9, 10}, {FLTTOFP(1.909), 10, 11}, {FLTTOFP(1.917), 11, 12}, {FLTTOFP(1.923), 12, 13},
  {FLTTOFP(1.929), 13, 14},{FLTTOFP(1.933), 14, 15}
};

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  u32 temp, frest, fr, dlest, minfr;
  unsigned i, idx;

  PREG UxLCR = ( PREG )uart_lcr[ id ];
  PREG UxDLM = ( PREG )uart_dlm[ id ];
  PREG UxDLL = ( PREG )uart_dll[ id ];
  PREG UxFCR = ( PREG )uart_fcr[ id ];  
  PREG UxFDR = ( PREG )uart_fdr[ id ];

  // Set data bits, parity, stop bit
  temp = 0;
  switch( databits )
  {
    case 5:
      temp |= UART_DATABITS_5;
      break;

    case 6:
      temp |= UART_DATABITS_6;
      break;

    case 7:
      temp |= UART_DATABITS_7;
      break;

    case 8:
      temp |= UART_DATABITS_8;
      break;
  }
  if( stopbits == PLATFORM_UART_STOPBITS_2 )
    temp |= UART_STOPBITS_2;
  else
    temp |= UART_STOPBITS_1;
  if( parity != PLATFORM_UART_PARITY_NONE )
  {
    temp |= UART_PARITY_ENABLE;
    if( parity == PLATFORM_UART_PARITY_ODD )
      temp |= UART_PARITY_ODD;
    else
      temp |= UART_PARITY_EVEN;
  }
  *UxLCR = temp;

  // Divisor and fractional baud computation
  // Based on the datasheet, but modified slightly (also used 24.8 fixed point)
  temp = ( Fpclk >> 4 ) / baud;
  if( ( ( Fpclk >> 4 ) % baud != 0 ) && UxFDR )
  {
    // Non-integer result, must estimate fr
    // Start from 1.1 and stop before 2, in 0.05 increments
    minfr = 0xFFFFFFFF;
    for( fr = FLTTOFP( 1.1 ); fr < INTTOFP( 2 ); fr += FLTTOFP( 0.05 ) )
    {
      dlest = ( Fpclk << 4 ) / ( baud * fr ); // this is an integer
      frest = ( Fpclk << 4 ) / ( baud * dlest ); // this is a 24.8 FP number
      if( ( frest < minfr ) && ( frest > FLTTOFP( 1.1 ) ) && ( frest < FLTTOFP( 1.9 ) ) )
      {
        temp = dlest;
        minfr = frest;
      }
    }
  }
  else
    minfr = INTTOFP( 1 );
  // Divisor is in 'temp', now find DivAddVal and MulVal from frest
  fr = minfr;
  idx = 0;
  if( fr != INTTOFP( 1 ) )
  {
    minfr = ABSDIFF( fr, fr_to_vals[ 0 ].fr );
    for( i = 1; i < sizeof( fr_to_vals ) / sizeof( uart_fractional_data ); i ++ )
      if( ABSDIFF( fr_to_vals[ i ].fr, fr ) < minfr )
      {
        idx = i;
        minfr = ABSDIFF( fr_to_vals[ i ].fr, fr );
      }
  }
  // Set baud and divisors
  *UxLCR |= UART_DLAB_ENABLE;
  if( UxFDR )
   *UxFDR = ( fr_to_vals[ idx ].mul << 4 ) | fr_to_vals[ idx ].divadd;
  *UxDLM = temp >> 8;
  *UxDLL = temp & 0xFF;
  *UxLCR &= ~UART_DLAB_ENABLE;

  // Enable and reset Tx and Rx FIFOs
  *UxFCR = UART_FIFO_ENABLE | UART_RXFIFO_RESET | UART_TXFIFO_RESET;

  // Setup PIOs for UART0. For the other ports, the user needs to specify what pin(s)
  // are allocated for UART Rx/Tx.
  if( id == 0 )
    PINSEL0 = ( PINSEL0 & 0xFFFFFF0F ) | 0x00000050;

  // Return the actual baud
  return ( Fpclk << 4 ) / ( 16 * temp * fr_to_vals[ idx ].fr );
}

void platform_uart_send( unsigned id, u8 data )
{
  PREG UxTHR = ( PREG )uart_thr[ id ];
  PREG UxLSR = ( PREG )uart_lsr[ id ];
  
  while( ( *UxLSR & LSR_THRE ) == 0 );
  *UxTHR = data;
}

int platform_s_uart_recv( unsigned id, s32 timeout )
{
  PREG UxLSR = ( PREG )uart_lsr[ id ];
  PREG UxRBR = ( PREG )uart_rbr[ id ];

  if( timeout == 0 )
  {
    // Return data only if already available
    if( *UxLSR & LSR_RDR )
      return *UxRBR;
    else
      return -1;
  }
  while( ( *UxLSR & LSR_RDR ) == 0 );
  return *UxRBR;
}

// ****************************************************************************
// Timer

void platform_s_timer_delay( unsigned id, u32 delay_us )
{
}
      
u32 platform_s_timer_op( unsigned id, int op, u32 data )
{
  u32 res = 0;

  switch( op )
  {
    case PLATFORM_TIMER_OP_START:
      break;
      
    case PLATFORM_TIMER_OP_READ:
      break;
      
    case PLATFORM_TIMER_OP_GET_MAX_DELAY:
      break;
      
    case PLATFORM_TIMER_OP_GET_MIN_DELAY:
      break;      
      
    case PLATFORM_TIMER_OP_SET_CLOCK:
      break;
      
    case PLATFORM_TIMER_OP_GET_CLOCK:
      break;
  }
  return res;
}

