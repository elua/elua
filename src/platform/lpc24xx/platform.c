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
#include "elua_adc.h"
#include "platform_conf.h"
#include "buf.h"
#include "elua_int.h"
#include "arm_constants.h"

// Platform includes
#include "LPC23xx.h"                        /* LPC23xx/24xx definitions */
#include "target.h"
#include "irq.h"
#include "uart.h"

extern void enable_ints();
extern void disable_ints();

// ****************************************************************************
// Platform initialization

static void platform_setup_timers();
static void platform_setup_pwm();
static void platform_setup_adcs();

// Power management definitions
enum
{
  PCUART2 = 1ULL << 24,
  PCUART3 = 1ULL << 25,
  PCTIM2 = 1ULL << 22,
  PCTIM3 = 1ULL << 23,
  PCADC = 1ULL << 12
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

#define P2C(Period)     (((Period<EMC_PERIOD)?0:(unsigned int)((float)Period/EMC_PERIOD)))
#define SDRAM_BASE_ADDR *(volatile unsigned int*)0xA0000000 //DYCS0
#define SDRAM_CS0_BASE (0xA0000000)

// External memory initialization
static void platform_setup_extmem()
{
#ifdef ELUA_BOARD_ELUAPUC
  volatile unsigned int i;
  volatile DWORD wtemp;

  EMC_CTRL = 0x00000001;                /*Disable Address mirror*/
  PCONP   |= 0x00000800;                /* Turn On EMC PCLK */
  PINSEL4  = 0x50000000;
  PINSEL5  = 0x05050555;
  PINSEL6  = 0x55555555;
  PINSEL8  = 0x55555555;
  PINSEL9  = 0x50555555;      
    
  EMC_DYN_RP = P2C(SDRAM_TRP);
  EMC_DYN_RAS = P2C(SDRAM_TRAS);
  EMC_DYN_SREX = P2C(SDRAM_TXSR); 
  EMC_DYN_APR = SDRAM_TAPR; 
  EMC_DYN_DAL = SDRAM_TDAL ;
  EMC_DYN_WR = SDRAM_TWR;
  EMC_DYN_RC = P2C(SDRAM_TRC); 
  EMC_DYN_RFC = P2C(SDRAM_TRFC); 
  EMC_DYN_XSR = P2C(SDRAM_TXSR); 
  EMC_DYN_RRD = P2C(SDRAM_TRRD); 
  EMC_DYN_MRD = SDRAM_TMRD; 
  
  EMC_DYN_RD_CFG=1;//Configures the dynamic memory read strategy(Command delayed strategy)
  
  /* Default setting, RAS latency 3 CCLKs, CAS latenty 3 CCLKs. */
  EMC_DYN_RASCAS0 = 0x00000303; // RAS delay = 3, CAS delay = 3  
  
   
  EMC_DYN_CFG0 = 0x00000280;   //16 bit external bus, 64 MB (4Mx16), 4 banks, row length = 12, column length = 8 
  for( i = 0; i < 40000; i ++ );
                                                                                    
  // JEDEC General SDRAM Initialization Sequence
  // DELAY to allow power and clocks to stabilize ~100 us
  // NOP
  EMC_DYN_CTRL = 0x0183;   
  //Issue SDRAM NOP (no operation) command ; CLKOUT runs continuously;All clock enables are driven HIGH continuously
  
  for( i = 0; i < 80000; i ++ );

  EMC_DYN_CTRL = 0x00000103; //Issue SDRAM PALL (precharge all) command.
  EMC_DYN_RFSH = 1;  //Indicates 1X16 CCLKs between SDRAM refresh cycles.
  for(i = 0; i < 0x40; i ++);

  //EMC_DYN_RFSH = P2C(SDRAM_REFRESH) >> 4; // //Indicates ?? CCLKs between SDRAM refresh cycles.
  EMC_DYN_RFSH = 70;
  
  EMC_DYN_CTRL = 0x00000083;
  wtemp = *(volatile DWORD *)(SDRAM_CS0_BASE | (0x33 << 11)); /* 8 burst, 3 CAS latency */ // modified from AN

  EMC_DYN_CTRL = 0x0000;  //Issue SDRAM norm command ; CLKOUT stop;All clock enables low

  EMC_DYN_CFG0|=0x80000; //Buffer enabled for accesses to DCS0 chip
  for(i = 200*10; i;i--);  
#endif
}

int platform_init()
{
  // Complete CPU initialization
  platform_setup_cpu();

  // External memory
  platform_setup_extmem();   

  // Setup peripherals
  platform_setup_timers();
  platform_setup_pwm();
  
#ifdef BUILD_ADC
  // Setup ADCs
  platform_setup_adcs();
#endif

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
static const u32 uart_fdr[ NUM_UART ] = { ( u32 )&U0FDR, ( u32 )&U1FDR, ( u32 )&U2FDR, ( u32 )&U3FDR };

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  u32 temp, uclk, mul_frac_div, div_add_frac_div;
  u32 diviser = 0;
  u32 mul_frac_div_opt = 0;
  u32 div_add_opt = 0;
  u32 div_opt = 0;
  u32 calc_baud = 0;
  u32 rel_err = 0;
  u32 rel_err_opt = 100000;

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

  // Divisor computation
  //temp = ( Fpclk_UART >> 4 ) / baud;
  uclk = Fpclk_UART >> 4;

  for( mul_frac_div = 1; mul_frac_div <= 15; mul_frac_div++ )
  {
    for( div_add_frac_div = 1; div_add_frac_div <= 15; div_add_frac_div++ )
    {
      temp = ( mul_frac_div * uclk ) / ( ( mul_frac_div + div_add_frac_div ) );

      diviser = temp / baud;

      if ( ( temp % baud ) > ( baud / 2 ) )
        diviser++;

      if ( diviser > 2 && diviser < 65536 )
      {
        calc_baud = temp / diviser;
       
        if (calc_baud <= baud)
          rel_err = baud - calc_baud;

        if ((rel_err < rel_err_opt))
        {
          mul_frac_div_opt = mul_frac_div ;
          div_add_opt = div_add_frac_div;
          div_opt = diviser;
          rel_err_opt = rel_err;
          if (rel_err == 0)
            break;
        }
      }
    }
  }
  // Set baud and divisors
  *UxLCR |= UART_DLAB_ENABLE;
  *UxDLM = div_opt >> 8;
  *UxDLL = div_opt & 0xFF;
  *UxLCR &= ~UART_DLAB_ENABLE;
  *UxFDR = ( ( mul_frac_div_opt << 4 ) & 0xF0 ) | ( div_add_opt & 0x0F );

  // Enable and reset Tx and Rx FIFOs
  *UxFCR = UART_FIFO_ENABLE | UART_RXFIFO_RESET | UART_TXFIFO_RESET;

  // Setup PIOs for UART0. For the other ports, the user needs to specify what pin(s)
  // are allocated for UART Rx/Tx.
  if( id == 0 )
    PINSEL0 = ( PINSEL0 & 0xFFFFFF0F ) | 0x00000050;

  // Return the actual baud
  return ( Fpclk_UART >> 4 ) / temp;
}

void platform_s_uart_send( unsigned id, u8 data )
{
  PREG UxTHR = ( PREG )uart_thr[ id ];
  PREG UxLSR = ( PREG )uart_lsr[ id ];
  
  while( ( *UxLSR & LSR_THRE ) == 0 );
  *UxTHR = data;
}

int platform_s_uart_recv( unsigned id, timer_data_type timeout )
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
  else
    while( ( *UxLSR & LSR_RDR ) == 0 );
  return *UxRBR;
}

int platform_s_uart_set_flow_control( unsigned id, int type )
{
  return PLATFORM_ERR;
}

// ****************************************************************************
// Timer section

static const u32 tmr_tcr[] = { ( u32 )&T0TCR, ( u32 )&T1TCR, ( u32 )&T2TCR, ( u32 )&T3TCR };
static const u32 tmr_tc[] = { ( u32 )&T0TC, ( u32 )&T1TC, ( u32 )&T2TC, ( u32 )&T3TC };
static const u32 tmr_pr[] = { ( u32 )&T0PR, ( u32 )&T1PR, ( u32 )&T2PR, ( u32 )&T3PR };
static const u32 tmr_pc[] = { ( u32 )&T0PC, ( u32 )&T1PC, ( u32 )&T2PC, ( u32 )&T3PC };
static const u32 tmr_mr1[] = { ( u32 )&T0MR1, ( u32 )&T1MR1, ( u32 )&T2MR1, ( u32 )&T3MR1 };
static const u32 tmr_mcr[] = { ( u32 )&T0MCR, ( u32 )&T1MCR, ( u32 )&T2MCR, ( u32 )&T3MCR };
static const u32 tmr_emr[] = { ( u32 )&T0EMR, ( u32 )&T1EMR, ( u32 )&T2EMR, ( u32 )&T3EMR };

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

static void int_handler_tmr()
{
  T3IR = 1; // clear interrupt
  cmn_virtual_timer_cb(); // handle virtual timers if they're present
  cmn_systimer_periodic(); // handle the system timer
  VICVectAddr = 0; // ACK interrupt
}

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

  // Setup system timer
  cmn_systimer_set_base_freq( 1000000 );
  cmn_systimer_set_interrupt_freq( VTMR_FREQ_HZ );

  // Setup virtual timers / system timer here
  // Timer 3 is allocated for virtual timers and the system timer, nothing else
  T3TCR = TMR_RESET;
  T3MR0 = 1000000ULL / VTMR_FREQ_HZ - 1;
  T3IR = 0xFF;
  // Set interrupt handle and eanble timer interrupt (and global interrupts)
  T3MCR = 0x03; // interrupt on match with MR0 and clear on match
  install_irq( TIMER3_INT, int_handler_tmr, HIGHEST_PRIORITY ); 
  platform_cpu_set_global_interrupts( PLATFORM_CPU_ENABLE );
  // Start timer
  T3TCR = TMR_ENABLE;
}

void platform_s_timer_delay( unsigned id, timer_data_type delay_us )
{
  PREG TxTCR = ( PREG )tmr_tcr[ id ];
  PREG TxTC = ( PREG )tmr_tc[ id ];
  u32 last;

  last = ( ( u64 )delay_us * platform_timer_get_clock( id ) ) / 1000000;
  *TxTCR = TMR_ENABLE | TMR_RESET;
  *TxTCR = TMR_ENABLE;
  while( *TxTC < last );
}
      
timer_data_type platform_s_timer_op( unsigned id, int op, timer_data_type data )
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

    case PLATFORM_TIMER_OP_SET_CLOCK:
      res = platform_timer_set_clock( id, data );
      break;
      
    case PLATFORM_TIMER_OP_GET_CLOCK:
      res = platform_timer_get_clock( id );
      break;

    case PLATFORM_TIMER_OP_GET_MAX_CNT:
      res = 0xFFFFFFFF;
      break;
  }
  return res;
}

int platform_s_timer_set_match_int( unsigned id, timer_data_type period_us, int type )
{
  return PLATFORM_TIMER_INT_INVALID_ID;
}

u64 platform_timer_sys_raw_read()
{
  return T3TC;
}

void platform_timer_sys_disable_int()
{
  T3MCR = 0x02; // clear on match, no interrupt
}

void platform_timer_sys_enable_int()
{
  T3MCR = 0x03; // interrupt on match with MR0 and clear on match
}

timer_data_type platform_timer_read_sys()
{
  return cmn_systimer_get();
}

// *****************************************************************************
// ADC specific functions and variables

#ifdef BUILD_ADC

static const u32 adc_trig[] = { 6, 7, 0, 0 };

static const u32 adc_dr[] = { ( u32 )&AD0DR0, ( u32 )&AD0DR1, ( u32 )&AD0DR2, ( u32 )&AD0DR3,
                              ( u32 )&AD0DR4, ( u32 )&AD0DR5, ( u32 )&AD0DR6, ( u32 )&AD0DR7 };

static const u8 pclk_div[] = { 4, 1, 2, 8};

int platform_adc_check_timer_id( unsigned id, unsigned timer_id )
{
  return ( ( timer_id == 1 ) );
}

void platform_adc_stop( unsigned id )
{  
  elua_adc_ch_state *s = adc_get_ch_state( id );
  elua_adc_dev_state *d = adc_get_dev_state( 0 );
    
  s->op_pending = 0;
  INACTIVATE_CHANNEL( d, id );
  
  // If there are no more active channels, stop the sequencer
  if( d->ch_active == 0 && d->running == 1 )
  {
    d->running = 0;
    AD0CR &= 0xF8FFFF00; // stop ADC, disable channels
  }
}



static void adc_int_handler() 
{
  elua_adc_dev_state *d = adc_get_dev_state( 0 );
  elua_adc_ch_state *s = d->ch_state[ d->seq_ctr ];
  u32 tmp, dreg_t;

  tmp = AD0STAT; // Clear interrupt flag
  //AD0INTEN = 0; // Disable generating interrupts

  dreg_t =  *( PREG )adc_dr[ s->id ];
  if ( dreg_t & ( 1UL << 31 ) )
  { 
    d->sample_buf[ s->id ] = ( u16 )( ( dreg_t >> 6 ) & 0x3FF );
    AD0CR &= 0xF8FFFF00;        // stop ADC, disable channels
    s->value_fresh = 1;
            
    if ( s->logsmoothlen > 0 && s->smooth_ready == 0)
      adc_smooth_data( s->id );
#if defined( BUF_ENABLE_ADC )
    else if ( s->reqsamples > 1 )
    {
      buf_write( BUF_ID_ADC, s->id, ( t_buf_data* )s->value_ptr );
      s->value_fresh = 0;
    }
#endif
    
    if ( adc_samples_available( s->id ) >= s->reqsamples && s->freerunning == 0 )
    {
      platform_adc_stop( s->id );      
    }
  }

  // Set up for next channel acquisition if we're still running
  if( d->running == 1 )
  {
    // Prep next channel in sequence, if applicable
    if( d->seq_ctr < ( d->seq_len - 1 ) )
      d->seq_ctr++;
    else if( d->seq_ctr == ( d->seq_len - 1 ) )
    { 
      adc_update_dev_sequence( 0 );
      d->seq_ctr = 0; // reset sequence counter if on last sequence entry
    }

    AD0CR |= ( 1ULL << d->ch_state[ d->seq_ctr ]->id );
    //AD0INTEN |= ( 1ULL << d->ch_state[ d->seq_ctr ]->id );
         
    if( d->clocked == 1  && d->seq_ctr == 0 ) // always use clock for first in clocked sequence
    {
      AD0CR |= ( adc_trig[ d->timer_id ] << 24 );
    }

    // Start next conversion if unclocked or if clocked and sequence index > 0
    if( ( d->clocked == 1 && d->seq_ctr > 0 ) || d->clocked == 0 )
    {
      AD0CR |= ( 1ULL << 24 ); // Start conversion now
    }
  }
  VICVectAddr = 0; // ACK interrupt
}

static void platform_setup_adcs()
{
  unsigned id;
  
  for( id = 0; id < NUM_ADC; id ++ )
    adc_init_ch_state( id );

  PCONP |= PCADC;

  AD0CR = ( ( Fpclk / 4500000 - 1 ) << 8 ) |  /* CLKDIV = Fpclk / 1000000 - 1 */   
          ( 0 << 16 ) |     /* BURST = 0, no BURST, software controlled */   
          ( 0 << 17 ) |     /* CLKS = 0, 11 clocks/10 bits */   
          ( 1 << 21 ) |     /* PDN = 1, normal operation */   
          ( 0 << 22 ) |     /* TEST1:0 = 00 */   
          ( 0 << 24 ) |     /* START = 0 A/D conversion stops */   
          ( 0 << 27 );      /* EDGE = 0 (CAP/MAT singal falling,trigger A/D conversion) */ 
  
  // Default enables ADC interrupt only on global, switch to per-channel
  //AD0INTEN &= ~( 1ULL << 8 );
   
  install_irq( ADC0_INT, adc_int_handler, HIGHEST_PRIORITY );
}


// NOTE: On this platform, there is only one ADC, clock settings apply to the whole device
u32 platform_adc_set_clock( unsigned id, u32 frequency )
{
  elua_adc_dev_state *d = adc_get_dev_state( 0 );

  if ( frequency > 0 )
  {
    d->clocked = 1;
    
    // Max Sampling Rate on LPC2468 is 200 kS/s
    if ( frequency > 200000 )
      frequency = 200000;
        
    // Set clock to 1 MHz
    platform_timer_set_clock( d->timer_id, 1000000ULL );
    
    // Set match to period in uS
    *( PREG )tmr_mr1[ d->timer_id ] = ( u32 )( ( 1000000ULL / ( frequency * 2 ) ) - 1 );
    
    // Reset on match
    *( PREG )tmr_mcr[ d->timer_id ] |= ( 1ULL << 4 );
    
    // Don't stop on match
    *( PREG )tmr_mcr[ d->timer_id ] &= ~( 1ULL << 5 );
    
    // Set match channel to 1
    *( PREG )tmr_emr[ d->timer_id ] |= ( 1ULL << 1 );
    
    // Toggle output on match
    *( PREG )tmr_emr[ d->timer_id ] |= ( 3ULL << 6 );
        
    frequency = 1000000ULL / (*( PREG )tmr_mr1[ d->timer_id ] + 1);
  }
  else
    d->clocked = 0;
    
  return frequency;
}

static const u8 adc_ports[] = {  0, 0,   0,  0,  1,  1,  0,  0 };
static const u8 adc_pins[] =  { 23, 24, 25, 26, 30, 31, 12, 13 };
static const u8 adc_funcs[] = {  1,  1,  1,  1,  3,  3,  3,  3 };

static const u32 pinsel_reg[] = { ( u32 )&PINSEL0, ( u32 )&PINSEL1, ( u32 )&PINSEL2,
                                  ( u32 )&PINSEL3, ( u32 )&PINSEL4, ( u32 )&PINSEL5,
                                  ( u32 )&PINSEL6, ( u32 )&PINSEL7, ( u32 )&PINSEL8,
                                  ( u32 )&PINSEL9, ( u32 )&PINSEL10 };

// Prepare Hardware Channel
int platform_adc_update_sequence( )
{ 
  elua_adc_dev_state *d = adc_get_dev_state( 0 ); 
  u8 seq_tmp;
  unsigned id;
  u32 pinnum, pinreg_idx;

  for( seq_tmp = 0; seq_tmp < d->seq_len; seq_tmp++ )
  {
    id = d->ch_state[ seq_tmp ]->id;
    pinnum = adc_pins[ id ];
    pinreg_idx = 2 * adc_ports[ id ];

    if ( pinnum >= 16 )
    {
        pinnum -= 16;
        pinreg_idx++;
    }
    
    *( PREG )pinsel_reg[ pinreg_idx ] &= ~( 0x03UL << pinnum * 2 );
    *( PREG )pinsel_reg[ pinreg_idx ] |= ( ( u32 )adc_funcs[ id ] << pinnum * 2 );
  }
  
  return PLATFORM_OK;
}


int platform_adc_start_sequence()
{ 
  elua_adc_dev_state *d = adc_get_dev_state( 0 );
  
  if( d->running != 1 )
  {
    adc_update_dev_sequence( 0 );
    
    // Start sampling on first channel
    d->seq_ctr = 0;

    // Enable channel & interrupt on channel conversion
    AD0CR |= ( 1ULL << d->ch_state[ d->seq_ctr ]->id );
    //AD0INTEN |= ( 1ULL << d->ch_state[ d->seq_ctr ]->id );

    d->running = 1;

    if( d->clocked == 1 )
    {
      AD0CR |= ( adc_trig[ d->timer_id ] << 24 );
      platform_s_timer_op( d->timer_id,  PLATFORM_TIMER_OP_START, 0);
    }
    else
      AD0CR |= ( 1ULL << 24 );
  }
  
  return PLATFORM_OK;
}

#endif // ifdef BUILD_ADC


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

// Get timer clock
u32 platform_pwm_get_clock( unsigned id )
{
  unsigned pwmid = id / 6;
  PREG PWMxPR = ( PREG )pwm_pr[ pwmid ];

  return Fpclk / ( *PWMxPR + 1 );
}

// Set timer clock
u32 platform_pwm_set_clock( unsigned id, u32 clock )
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

void platform_pwm_start( unsigned id )
{
  unsigned pwmid = id / 6;
  PREG PWMxTCR = ( PREG )pwm_tcr[ pwmid ];
  PREG PWMxPCR = ( PREG )pwm_pcr[ pwmid ];

  *PWMxPCR = PWM_ENABLE_1 | PWM_ENABLE_2 | PWM_ENABLE_3 | PWM_ENABLE_4 | PWM_ENABLE_5 | PWM_ENABLE_6;
  *PWMxTCR = PWM_ENABLE | PWM_MODE;
}

void platform_pwm_stop( unsigned id )
{
  unsigned pwmid = id / 6;
  PREG PWMxTCR = ( PREG )pwm_tcr[ pwmid ];
  PREG PWMxPCR = ( PREG )pwm_pcr[ pwmid ];

  *PWMxPCR = 0;   
  *PWMxTCR = PWM_RESET;
}
