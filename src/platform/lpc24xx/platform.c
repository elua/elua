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

static void platform_setup_timers();

// Power management definitions
enum
{
  PCUART2 = 1ULL << 24,
  PCUART3 = 1ULL << 25,
  PCTIM2 = 1ULL << 22,
  PCTIM3 = 1ULL << 23
};

// CPU initialization
static void platform_cpu_setup()
{
  // Enable clock for UART2 and UART3
  PCONP |= PCUART2 | PCUART3;

  // Enable clock for Timer 2 and Timer 3
  PCONP |= PCTIM2 | PCTIM3;

  // Setup GPIO0 and GPIO1 in fast mode
  SCS |= 1;
}

int platform_init()
{
  // Complete CPU initialization
  platform_cpu_setup();

  // Setup timers
  platform_setup_timers();

  // Initialize UART
  platform_uart_setup( CON_UART_ID, CON_UART_SPEED, 8, PLATFORM_UART_PARITY_NONE, PLATFORM_UART_STOPBITS_1 );

  // Common platform initialization code
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
// UART1 doesn't have a fractional baud rate generator
static const u32 uart_fdr[ NUM_UART ] = { ( u32 )&U0FDR, ( u32 )0,      ( u32 )&U2FDR, ( u32 )&U3FDR };

// Quick-and-dirty FP
#define INTTOFP(x)      ( (x) << 8 )
#define FLTTOFP(x)      ( ( u32 )( x * 256 ) )

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  u32 temp, frest, fr, dlest, minfr, minbauddiff, baudest;
  unsigned d, m, bd, bm;

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
  // Based on the datasheet, but modified slightly (also uses 24.8 fixed point)
  temp = ( Fpclk >> 4 ) / baud;
  if( ( ( Fpclk >> 4 ) % baud != 0 ) && UxFDR )
  {
    // Non-integer result, must estimate fr
    // Start from 1.025 and stop before 2, in 0.025 increments
    minfr = 0xFFFFFFFF;
    minbauddiff = 0xFFFFFFFF;
    for( fr = FLTTOFP( 1.025 ); fr < INTTOFP( 2 ); fr += FLTTOFP( 0.025 ) )
    {
      dlest = ( Fpclk << 4 ) / ( baud * fr ); // this is an integer
      frest = ( Fpclk << 4 ) / ( baud * dlest ); // this is a 24.8 FP number
      baudest =  ( Fpclk << 4 ) / ( frest * dlest ); // this is an integer
      if( ( ABSDIFF( baudest, baud ) < minbauddiff ) && ( frest > FLTTOFP( 1.0 ) ) && ( frest < FLTTOFP( 2 ) ) )
      {
        temp = dlest;
        minfr = frest;
        minbauddiff = ABSDIFF( baudest, baud );
      }
    }
  }
  else
    minfr = INTTOFP( 1 );
  // Divisor is in 'temp', now find best fraction that approximates 'minfr'
  fr = minfr;
  if( fr != INTTOFP( 1 ) )
  {
    minfr = 0xFFFFFFFF;
    bd = 0;
    bm = 1;
    for( d = 0; d <= 14; d ++ )
      for( m = d + 1; m <= 15; m ++ )
      {
        frest = INTTOFP( d ) / m + INTTOFP( 1 );  
        if( ABSDIFF( frest, fr ) < ABSDIFF( minfr, fr ) )
        {
          minfr = frest;
          bd = d;
          bm = m;
        }
      }
  }
  else
  {
    bd = 0;
    bm = 1;
  }  
  // Set baud and divisors
  *UxLCR |= UART_DLAB_ENABLE;
  if( UxFDR )
   *UxFDR = ( bm << 4 ) | bd;
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
  return ( Fpclk << 4 ) / ( temp * minfr );
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
// Timer section

static const u32 tmr_tcr[ NUM_TIMER ] = { ( u32 )&T0TCR, ( u32 )&T1TCR, ( u32 )&T2TCR, ( u32 )&T3TCR };
static const u32 tmr_ttc[ NUM_TIMER ] = { ( u32 )&T0TC, ( u32 )&T1TC, ( u32 )&T2TC, ( u32 )&T3TC };
static const u32 tmr_tpr[ NUM_TIMER ] = { ( u32 )&T0PR, ( u32 )&T1PR, ( u32 )&T2PR, ( u32 )&T3PR };
static const u32 tmr_tpc[ NUM_TIMER ] = { ( u32 )&T0PC, ( u32 )&T1PC, ( u32 )&T2PC, ( u32 )&T3PC };

// Timer register definitions
enum
{
  TMR_ENABLE = 1,
  TMR_RESET = 2
};

// Helper function: get timer clock
static u32 platform_timer_get_clock( unsigned id )
{
  PREG TxPR = ( PREG )tmr_tpr[ id ];

  return Fpclk / ( *TxPR + 1 );
}

// Helper function: set timer clock
static u32 platform_timer_set_clock( unsigned id, unsigned clock )
{
  u32 div = Fpclk / clock, prevtc;
  PREG TxPR = ( PREG )tmr_tpr[ id ];  
  PREG TxPC = ( PREG )tmr_tpc[ id ];
  PREG TxTC = ( PREG )tmr_ttc[ id ]; 
  
  prevtc = *TxTC;
  *TxTC = 0;
  *TxPC = 0; 
  *TxPR = div - 1;
  *TxTC = prevtc;
  return Fpclk / div;
}

// Helper function: setup timers
static void platform_setup_timers()
{
  unsigned i;
  PREG TxTC;

  // Set base frequency to 1MHz, as we can't use a better resolution anyway
  for( i = 0; i < NUM_TIMER; i ++ )
  {
    TxTC = ( PREG )tmr_ttc[ i ];
    *TxTC = 0;
    platform_timer_set_clock( 0, 1000000 );
  }
}

void platform_s_timer_delay( unsigned id, u32 delay_us )
{
  PREG TxCR = ( PREG )tmr_tcr[ id ];
  PREG TxTC = ( PREG )tmr_ttc[ id ];
  u32 last;

  last = ( ( u64 )delay_us * platform_timer_get_clock( id ) ) / 1000000;
  *TxTC = TMR_ENABLE | TMR_RESET;
  *TxTC = TMR_ENABLE;
  while( *TxCR < last );
}
      
u32 platform_s_timer_op( unsigned id, int op, u32 data )
{
  u32 res = 0;
  PREG TxCR = ( PREG )tmr_tcr[ id ];
  PREG TxTC = ( PREG )tmr_ttc[ id ];

  switch( op )
  {
    case PLATFORM_TIMER_OP_START:
      *TxTC = TMR_ENABLE | TMR_RESET;
      *TxTC = TMR_ENABLE;
      break;
      
    case PLATFORM_TIMER_OP_READ:
      res = *TxCR;
      break;

    case PLATFORM_TIMER_OP_GET_MAX_DELAY:
      res = platform_timer_get_diff_us( id, 0, 0xFFFFFFFF );
      break;
      
    case PLATFORM_TIMER_OP_GET_MIN_DELAY:
      res = platform_timer_get_diff_us( id, 0, 1 );
      break;      
      
    case PLATFORM_TIMER_OP_SET_CLOCK:
      res = platform_timer_set_clock( id, data );
      break;
      
    case PLATFORM_TIMER_OP_GET_CLOCK:
      res = platform_timer_get_clock( id );
      break;
  }
  return res;
}

