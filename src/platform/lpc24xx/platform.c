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
#include "elua_int.h"

// Platform includes
#include "LPC23xx.h"                        /* LPC23xx/24xx definitions */
#include "target.h"
#include "irq.h"
#include "uart.h"

// *****************************************************************************
// These interrupt handlers are the link to elua_int.c

static PREG const posedge_status[] = { ( PREG )&IO0_INT_STAT_R, ( PREG )&IO2_INT_STAT_R };
static PREG const negedge_status[] = { ( PREG )&IO0_INT_STAT_F, ( PREG )&IO2_INT_STAT_F };
static PREG const intclr_regs[] = { ( PREG )&IO0_INT_CLR, ( PREG )&IO2_INT_CLR };

static void __attribute__((optimize(0))) __attribute__((interrupt ("IRQ"))) eint3_int_handler()
{
  elua_int_id id = ELUA_INT_INVALID_INTERRUPT;
  pio_code resnum = 0;
  int pidx, pin;
  
  EXTINT |= 1 << 3; // clear interrupt
  // Look for interrupt source
  // In can only be GPIO0/GPIO2, as the EXT interrupts are not (yet) used
  pidx = ( IO_INT_STAT & 1 ) ? 0 : 1;
  if( *posedge_status[ pidx ] )
  {
    id = INT_GPIO_POSEDGE;
    pin = intlog2( *posedge_status[ pidx ] );
  }
  else
  {
    id = INT_GPIO_NEGEDGE;
    pin = intlog2( *negedge_status[ pidx ] );
  }
  resnum = PLATFORM_IO_ENCODE( pidx * 2, pin, PLATFORM_IO_ENC_PIN );   
  *intclr_regs[ pidx ] = 1 << pin;
  
  // Queue interrupt
  elua_int_add( id, resnum );
  VICVectAddr = 0; // ACK interrupt    
}

// ****************************************************************************
// Platform initialization

static void platform_setup_timers();
static void platform_setup_pwm();

// Power management definitions
enum
{
  PCUART2 = 1ULL << 24,
  PCUART3 = 1ULL << 25,
  PCTIM2 = 1ULL << 22,
  PCTIM3 = 1ULL << 23
};

// CPU initialization
static void platform_setup_cpu()
{
  // Enable clock for UART2 and UART3
  PCONP |= PCUART2 | PCUART3;

  // Set clock for all the UARTs to the system clock (helps in baud generation)
  PCLKSEL0 = ( PCLKSEL0 & 0xFFFFFC3F ) | 0x00000140;
  PCLKSEL1 = ( PCLKSEL1 & 0xFFF0FFFF ) | 0x00050000;

  // Enable clock for Timer 2 and Timer 3
  PCONP |= PCTIM2 | PCTIM3;

  // Setup GPIO0 and GPIO1 in fast mode
  SCS |= 1;
}

// Setup all required interrupt handlers
static void platform_setup_interrupts()
{
  install_irq( EINT3_INT, eint3_int_handler, HIGHEST_PRIORITY - 1 );   
}

int platform_init()
{
  // Complete CPU initialization
  platform_setup_cpu();

  // Setup peripherals
  platform_setup_timers();
  platform_setup_pwm();
  
  // Setup interrupt handlers
  platform_setup_interrupts();

  // Initialize console UART
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
static const u32 pio_pinmode[ NUM_PIO * 2 ] = { ( u32 )&PINMODE0, ( u32 )&PINMODE1, ( u32 )&PINMODE2, ( u32 )&PINMODE3, ( u32 )&PINMODE4,
                                                ( u32 )&PINMODE5, ( u32 )&PINMODE6, ( u32 )&PINMODE7, ( u32 )&PINMODE8, ( u32 )&PINMODE9 };

// The platform I/O functions
pio_type platform_pio_op( unsigned port, pio_type pinmask, int op )
{
  pio_type retval = 1;
  PREG FIOxDIR = ( PREG )pio_fiodir[ port ];
  PREG FIOxPIN = ( PREG )pio_fiopin[ port ];
  PREG FIOxSET = ( PREG )pio_fioset[ port ];
  PREG FIOxCLR = ( PREG )pio_fioclr[ port ];
  PREG PINxMODE0 = ( PREG )pio_pinmode[ port * 2 ];
  PREG PINxMODE1 = ( PREG )pio_pinmode[ port * 2 + 1 ];
  u32 mask;
   
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

    case PLATFORM_IO_PIN_PULLUP:
      if( port == 0 && ( pinmask & 0xF8000000 ) )
        printf( "Unable to set pullups on specified pin(s).\n" );
      else
      {
        for( mask = 1; mask < 16; mask ++ )
          if( pinmask & ( 1 << mask ) )
            *PINxMODE0 = *PINxMODE0 & ~( 3 << ( mask * 2 ) );
        for( mask = 16; mask < 32; mask ++ ) 
          if( pinmask & ( 1 << mask ) )
            *PINxMODE1 = *PINxMODE1 & ~( 3 << ( mask * 2 ) );
      }
      break;

    case PLATFORM_IO_PIN_PULLDOWN:
      if( port == 0 && ( pinmask & 0xF8000000 ) )
        printf( "Unable to set pulldowns on specified pin(s).\n" );
      else
      {
         for( mask = 1; mask < 16; mask ++ )
          if( pinmask & ( 1 << mask ) )
            *PINxMODE0 = ( *PINxMODE0 & ~( 3 << ( mask * 2 ) ) ) | ( 3 << ( mask * 2 ) );
        for( mask = 16; mask < 32; mask ++ ) 
          if( pinmask & ( 1 << mask ) )
            *PINxMODE1 = ( *PINxMODE1 & ~( 3 << ( mask * 2 ) ) ) | ( 3 << ( mask * 2 ) );
      }
      break;

    case PLATFORM_IO_PIN_NOPULL:
      if( port == 0 && ( pinmask & 0xF8000000 ) )
        printf( "Unable to reset pullups/pulldowns on specified pin(s).\n" );
      else
      {
        for( mask = 1; mask < 16; mask ++ )
          if( pinmask & ( 1 << mask ) )
            *PINxMODE0 = ( *PINxMODE0 & ~( 3 << ( mask * 2 ) ) ) | ( 2 << ( mask * 2 ) );
        for( mask = 16; mask < 32; mask ++ ) 
          if( pinmask & ( 1 << mask ) )
            *PINxMODE1 = ( *PINxMODE1 & ~( 3 << ( mask * 2 ) ) ) | ( 2 << ( mask * 2 ) );
      }
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

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  u32 temp;

  PREG UxLCR = ( PREG )uart_lcr[ id ];
  PREG UxDLM = ( PREG )uart_dlm[ id ];
  PREG UxDLL = ( PREG )uart_dll[ id ];
  PREG UxFCR = ( PREG )uart_fcr[ id ];  

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

  // Divisor computation
  temp = ( Fpclk_UART >> 4 ) / baud;
  // Set baud and divisors
  *UxLCR |= UART_DLAB_ENABLE;
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
  return ( Fpclk_UART >> 4 ) / temp;
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

static const u32 tmr_tcr[] = { ( u32 )&T0TCR, ( u32 )&T1TCR, ( u32 )&T2TCR, ( u32 )&T3TCR };
static const u32 tmr_tc[] = { ( u32 )&T0TC, ( u32 )&T1TC, ( u32 )&T2TC, ( u32 )&T3TC };
static const u32 tmr_pr[] = { ( u32 )&T0PR, ( u32 )&T1PR, ( u32 )&T2PR, ( u32 )&T3PR };
static const u32 tmr_pc[] = { ( u32 )&T0PC, ( u32 )&T1PC, ( u32 )&T2PC, ( u32 )&T3PC };

// Timer register definitions
enum
{
  TMR_ENABLE = 1,
  TMR_RESET = 2
};

// Helper function: get timer clock
static u32 platform_timer_get_clock( unsigned id )
{
  PREG TxPR = ( PREG )tmr_pr[ id ];

  return Fpclk / ( *TxPR + 1 );
}

// Helper function: set timer clock
static u32 platform_timer_set_clock( unsigned id, u32 clock )
{
  u32 div = Fpclk / clock, prevtc;
  PREG TxPR = ( PREG )tmr_pr[ id ];  
  PREG TxPC = ( PREG )tmr_pc[ id ];
  PREG TxTCR = ( PREG )tmr_tcr[ id ]; 
  
  prevtc = *TxTCR;
  *TxTCR = 0;
  *TxPC = 0; 
  *TxPR = div - 1;
  *TxTCR = prevtc;
  return Fpclk / div;
}

#if VTMR_NUM_TIMERS > 0
static void __attribute__((interrupt ("IRQ"))) tmr_int_handler() 
{
  T3IR = 1; // clear interrupt
  cmn_virtual_timer_cb();
  VICVectAddr = 0; // ACK interrupt
}
#endif

// Helper function: setup timers
static void platform_setup_timers()
{
  unsigned i;
  PREG TxTCR;

  // Set base frequency to 1MHz, as we can't use a better resolution anyway
  for( i = 0; i < 4; i ++ )
  {
    TxTCR = ( PREG )tmr_tcr[ i ];
    *TxTCR = 0;
    platform_timer_set_clock( i, 1000000ULL );
  }
#if VTMR_NUM_TIMERS > 0
  // Setup virtual timers here
  // Timer 3 is allocated for virtual timers and nothing else in this case
  T3TCR = TMR_RESET;
  T3MR0 = 1000000ULL / VTMR_FREQ_HZ - 1;
  T3IR = 0xFF;
  // Set interrupt handle and eanble timer interrupt (and global interrupts)
  T3MCR = 0x03; // interrupt on match with MR0 and clear on match
  install_irq( TIMER3_INT, tmr_int_handler, HIGHEST_PRIORITY ); 
  platform_cpu_set_global_interrupts( PLATFORM_CPU_ENABLE );
  // Start timer
  T3TCR = TMR_ENABLE;
#endif
}

void platform_s_timer_delay( unsigned id, u32 delay_us )
{
  PREG TxTCR = ( PREG )tmr_tcr[ id ];
  PREG TxTC = ( PREG )tmr_tc[ id ];
  u32 last;

  last = ( ( u64 )delay_us * platform_timer_get_clock( id ) ) / 1000000;
  *TxTCR = TMR_ENABLE | TMR_RESET;
  *TxTCR = TMR_ENABLE;
  while( *TxTC < last );
}
      
u32 platform_s_timer_op( unsigned id, int op, u32 data )
{
  u32 res = 0;
  PREG TxTCR = ( PREG )tmr_tcr[ id ];
  PREG TxTC = ( PREG )tmr_tc[ id ];

  switch( op )
  {
    case PLATFORM_TIMER_OP_START:
      *TxTCR = TMR_ENABLE | TMR_RESET;
      *TxTCR = TMR_ENABLE;
      break;
      
    case PLATFORM_TIMER_OP_READ:
      res = *TxTC;
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

// ****************************************************************************
// CPU functions

#define INTERRUPT_ENABLED_MASK          0x80

extern void enable_ints();
extern void disable_ints();
extern u32 get_int_status();

static PREG const posedge_regs[] = { ( PREG )&IO0_INT_EN_R, NULL, ( PREG )&IO2_INT_EN_R };
static PREG const negedge_regs[] = { ( PREG )&IO0_INT_EN_F, NULL, ( PREG )&IO0_INT_EN_F };

int platform_cpu_set_global_interrupts( int status )
{
  u32 crt_status = get_int_status();

  if( status == PLATFORM_CPU_ENABLE )
    enable_ints();
  else
    disable_ints();
  return ( crt_status & INTERRUPT_ENABLED_MASK ) != 0;
}

int platform_cpu_get_global_interrupts()
{
  return ( get_int_status() & INTERRUPT_ENABLED_MASK ) != 0;
}

// Helper: return the status of a specific interrupt (enabled/disabled)
static int platform_cpuh_get_int_status( elua_int_id id, elua_int_resnum resnum )
{
  int port, pin;
  
  if( id == INT_GPIO_POSEDGE || id == INT_GPIO_NEGEDGE )
  {
    port = PLATFORM_IO_GET_PORT( resnum ); 
    pin = PLATFORM_IO_GET_PIN( resnum ); 
    if( id == INT_GPIO_POSEDGE )
      return *posedge_regs[ port ] & ( 1 << pin );
    else
      return *negedge_regs[ port ] & ( 1 << pin );        
  } 
  return 0;
}

int platform_cpu_set_interrupt( elua_int_id id, elua_int_resnum resnum, int status )
{
  int crt_status = platform_cpuh_get_int_status( id, resnum );
  int port, pin;
  
  if( id == INT_GPIO_POSEDGE || id == INT_GPIO_NEGEDGE )
  {
    port = PLATFORM_IO_GET_PORT( resnum ); 
    pin = PLATFORM_IO_GET_PIN( resnum ); 
    if( id == INT_GPIO_POSEDGE )
    {
      if( status == PLATFORM_CPU_ENABLE )
        *posedge_regs[ port ] |= 1 << pin;
      else
        *posedge_regs[ port ] &= ~( 1 << pin );       
    }
    else
    {
      if( status == PLATFORM_CPU_ENABLE )
        *negedge_regs[ port ] |= 1 << pin;
      else
        *negedge_regs[ port ] &= ~( 1 << pin );         
    }    
  }
  return crt_status;
}

int platform_cpu_get_interrupt( elua_int_id id, elua_int_resnum resnum )
{
  return platform_cpuh_get_int_status( id, resnum );
}

// ****************************************************************************
// PWM functions

static const u32 pwm_tcr[] = { ( u32 )&PWM0TCR, ( u32 )&PWM1TCR };
static const u32 pwm_pr[] = { ( u32 )&PWM0PR, ( u32 )&PWM1PR };
static const u32 pwm_pc[] = { ( u32 )&PWM0PC, ( u32 )&PWM1PC };
static const u32 pwm_pcr[] = { ( u32 )&PWM0PCR, ( u32 )&PWM1PCR };
static const u32 pwm_mcr[] = { ( u32 )&PWM0MCR, ( u32 )&PWM1MCR };
static const u32 pwm_ler[] = { ( u32 )&PWM0LER, ( u32 )&PWM1LER };
static const u32 pwm_channels[ 2 ][ 6 ] = 
{
  { ( u32 )&PWM0MR1, ( u32 )&PWM0MR2, ( u32 )&PWM0MR3, ( u32 )&PWM0MR4, ( u32 )&PWM0MR5, ( u32 )&PWM0MR6 },
  { ( u32 )&PWM1MR1, ( u32 )&PWM1MR2, ( u32 )&PWM1MR3, ( u32 )&PWM1MR4, ( u32 )&PWM1MR5, ( u32 )&PWM1MR6 }, 
};

// Timer register definitions
enum
{
  PWM_ENABLE = 1,
  PWM_RESET = 2,
  PWM_MODE = 8,
  PWM_ENABLE_1 = 1 << 9,
  PWM_ENABLE_2 = 1 << 10,
  PWM_ENABLE_3 = 1 << 11,
  PWM_ENABLE_4 = 1 << 12,
  PWM_ENABLE_5 = 1 << 13,
  PWM_ENABLE_6 = 1 << 14,
};

// Helper function: get timer clock
static u32 platform_pwm_get_clock( unsigned id )
{
  unsigned pwmid = id / 6;
  PREG PWMxPR = ( PREG )pwm_pr[ pwmid ];

  return Fpclk / ( *PWMxPR + 1 );
}

// Helper function: set timer clock
static u32 platform_pwm_set_clock( unsigned id, u32 clock )
{
  u32 div = Fpclk / clock, prevtc;
  unsigned pwmid = id / 6;
  PREG PWMxPR = ( PREG )pwm_pr[ pwmid ];  
  PREG PWMxPC = ( PREG )pwm_pc[ pwmid ];
  PREG PWMxTCR = ( PREG )pwm_tcr[ pwmid ]; 
  
  prevtc = *PWMxTCR;
  *PWMxTCR = 0;
  *PWMxPC = 0; 
  *PWMxPR = div - 1;
  *PWMxTCR = prevtc;
  return Fpclk / div;
}

// Setup all PWM channels
static void platform_setup_pwm()
{
  unsigned i;
  PREG temp;

  for( i = 0; i < 2; i ++ )
  {
    // Keep clock in reset, set PWM code
    temp = ( PREG )pwm_tcr[ i ];
    *temp = PWM_RESET;
    // Set match mode (reset on MR0 match)
    temp = ( PREG )pwm_mcr[ i ];
    *temp = 0x02; 
    // Set base frequency to 1MHz
    platform_pwm_set_clock( i * 6, 1000000 );
  }
}

u32 platform_pwm_setup( unsigned id, u32 frequency, unsigned duty )
{
  unsigned pwmid = id / 6, chid = id % 6;
  PREG PWMxMR0 = pwmid == 0 ? ( PREG )&PWM0MR0 : ( PREG )&PWM1MR0;
  PREG PWMxMRc = ( PREG )pwm_channels[ pwmid ][ chid ];
  PREG PWMxLER = ( PREG )pwm_ler[ pwmid ];
  u32 divisor;

  divisor = platform_pwm_get_clock( id ) / frequency - 1;
  *PWMxMR0 = divisor;
  *PWMxMRc = ( divisor * duty ) / 100;
  *PWMxLER = 1 | ( 1 << ( chid + 1 ) );

  return platform_pwm_get_clock( id ) / divisor;
}

u32 platform_pwm_op( unsigned id, int op, u32 data )
{
  u32 res = 0;
  unsigned pwmid = id / 6;
  PREG PWMxTCR = ( PREG )pwm_tcr[ pwmid ];
  PREG PWMxPCR = ( PREG )pwm_pcr[ pwmid ];

  switch( op )
  {
    case PLATFORM_PWM_OP_START:
      *PWMxPCR = PWM_ENABLE_1 | PWM_ENABLE_2 | PWM_ENABLE_3 | PWM_ENABLE_4 | PWM_ENABLE_5 | PWM_ENABLE_6;
      *PWMxTCR = PWM_ENABLE | PWM_MODE;
      break;

    case PLATFORM_PWM_OP_STOP:
      *PWMxPCR = 0;   
      *PWMxTCR = PWM_RESET;
      break;

    case PLATFORM_PWM_OP_SET_CLOCK:
      res = platform_pwm_set_clock( id, data );
      break;

    case PLATFORM_PWM_OP_GET_CLOCK:
      res = platform_pwm_get_clock( id );
      break;
  }

  return res;
}

