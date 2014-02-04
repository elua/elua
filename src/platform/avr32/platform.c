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
#include "elua_adc.h"
#include "platform_conf.h"
#include "common.h"
#include "buf.h"
#ifdef BUILD_MMCFS
#include "diskio.h"
#endif

#ifdef  BUILD_UIP
#include "ethernet.h"
#include "uip_arp.h"
#include "elua_uip.h"
#include "uip-conf.h"
#endif

// Platform-specific includes
#include <avr32/io.h>
#include "compiler.h"
#include "flashc.h"
#include "pm.h"
#include "board.h"
#include "usart.h"
#include "gpio.h"
#include "tc.h"
#include "intc.h"
#include "spi.h"
#include "adc.h"
#include "pwm.h"
#include "i2c.h"

#ifdef  BUILD_USB_CDC
#if !defined( VTMR_NUM_TIMERS ) || VTMR_NUM_TIMERS == 0
# error "On AVR32, USB_CDC needs virtual timer support. Define VTMR_NUM_TIMERS > 0."
#endif
#include "usb-cdc.h"
#endif

#ifdef BUILD_UIP

// UIP sys tick data
// NOTE: when using virtual timers, SYSTICKHZ and VTMR_FREQ_HZ should have the
// same value, as they're served by the same timer (the systick)
#if !defined( VTMR_NUM_TIMERS ) || VTMR_NUM_TIMERS == 0
# error "On AVR32, UIP needs virtual timer support. Define VTMR_NUM_TIMERS > 0."
#endif

#define SYSTICKHZ VTMR_FREQ_HZ
#define SYSTICKMS (1000 / SYSTICKHZ)

static int eth_timer_fired;

#endif // BUILD_UIP

// ****************************************************************************
// AVR32 system timer implementation

// Since the timer hardware (TC) on the AVR32 is pretty basic (16-bit timers,
// limited prescaling options) we'll be using the PWM subsystem for the system
// timer. The PWM hardware has much better prescaling options and it uses 20-bit
// timers which are perfect for our needs. As a bonus, each PWM channel can be
// clocked from two independent linear prescalers (CLKA and CLKB). The AVR32
// PWM driver (pwm.c) uses only CLKA and disables CLKB, so by using CLKB we
// won't change the regular PWM behaviour. The only downside is that we'll steal
// a PWM channel for the system timer.

#define SYSTIMER_PWM_CH       6

__attribute__((__interrupt__)) static void systimer_int_handler()
{
  volatile u32 dummy = AVR32_PWM.isr; // clear interrupt

  ( void )dummy;
  cmn_systimer_periodic();
}

static void platform_systimer_init()
{
  avr32_pwm_mr_t mr = AVR32_PWM.MR;

  // Set CLKB prescaler for 1MHz clock (which is exactly our system clock frequency)
  mr.preb = 0; // main source clock is MCK (PBA)
  mr.divb = REQ_PBA_FREQ / 1000000; // set CLKB to 1MHz
  AVR32_PWM.MR = mr;

  // Now setup our PWM channel
  // Clock from CLKB, left aligned (the other parameters are not important)
  AVR32_PWM.channel[ SYSTIMER_PWM_CH ].cmr = AVR32_PWM_CMR_CPRE_CLKB;
  // The period register is 20-bit wide (1048576). We set it so we get interrupts
  // every second (which results in a very reasonable system load)
  AVR32_PWM.channel[ SYSTIMER_PWM_CH ].cprd = 1000000;
  // The duty cycle is arbitrary set to 50%
  AVR32_PWM.channel[ SYSTIMER_PWM_CH ].cdty = 500000;

  // Setup PWM interrupt
  INTC_register_interrupt( &systimer_int_handler, AVR32_PWM_IRQ, AVR32_INTC_INT0 );
  AVR32_PWM.ier = 1 << SYSTIMER_PWM_CH;

  // Enable the channel
  AVR32_PWM.ena = 1 << SYSTIMER_PWM_CH;
}

// ****************************************************************************
// Platform initialization
#ifdef BUILD_UIP
u32 platform_ethernet_setup( void );
#endif

extern int pm_configure_clocks( pm_freq_param_t *param );

static u32 platform_timer_set_clock( unsigned id, u32 clock );

#ifdef BUILD_ADC
__attribute__((__interrupt__)) static void adc_int_handler();
#endif

const u32 uart_base_addr[ ] = {
  AVR32_USART0_ADDRESS,
  AVR32_USART1_ADDRESS,
#if NUM_UART > 2
  AVR32_USART2_ADDRESS,
#ifdef AVR32_USART3_ADDRESS
  AVR32_USART3_ADDRESS,
#endif
#endif
};

int platform_init()
{
  pm_freq_param_t pm_freq_param =
  {
    REQ_CPU_FREQ,
    REQ_PBA_FREQ,
    FOSC0,
    OSC0_STARTUP,
  };
  tc_waveform_opt_t tmropt =
  {
    .waveform.bswtrg = TC_EVT_EFFECT_NOOP, // Software trigger effect on TIOB.
    .waveform.beevt  = TC_EVT_EFFECT_NOOP, // External event effect on TIOB.
    .waveform.bcpc   = TC_EVT_EFFECT_NOOP, // RC compare effect on TIOB.
    .waveform.bcpb   = TC_EVT_EFFECT_NOOP, // RB compare effect on TIOB.

    .waveform.aswtrg = TC_EVT_EFFECT_NOOP, // Software trigger effect on TIOA.
    .waveform.aeevt  = TC_EVT_EFFECT_NOOP, // External event effect on TIOA.
    .waveform.acpc   = TC_EVT_EFFECT_NOOP, // RC compare effect on TIOA: toggle.
    .waveform.acpa   = TC_EVT_EFFECT_NOOP, // RA compare effect on TIOA: toggle (other possibilities are none, set and clear).

    .waveform.wavsel = TC_WAVEFORM_SEL_UP_MODE, // Waveform selection: Up mode
    .waveform.enetrg = FALSE,              // External event trigger enable.
    .waveform.eevt   = 0,                  // External event selection.
    .waveform.eevtedg= TC_SEL_NO_EDGE,     // External event edge selection.
    .waveform.cpcdis = FALSE,              // Counter disable when RC compare.
    .waveform.cpcstop= FALSE,              // Counter clock stopped with RC compare.

    .waveform.burst  = FALSE,              // Burst signal selection.
    .waveform.clki   = FALSE,              // Clock inversion.
    .waveform.tcclks = TC_CLOCK_SOURCE_TC1 // Internal source clock 1 (32768Hz)
  };
  volatile avr32_tc_t *tc = &AVR32_TC;
  unsigned i;

  Disable_global_interrupt();
  INTC_init_interrupts();

  // Setup clocks
  if( PM_FREQ_STATUS_FAIL == pm_configure_clocks( &pm_freq_param ) )
    return PLATFORM_ERR;
#ifdef FOSC32
  // Select the 32-kHz oscillator crystal
  pm_enable_osc32_crystal (&AVR32_PM );
  // Enable the 32-kHz clock
  pm_enable_clk32_no_wait( &AVR32_PM, AVR32_PM_OSCCTRL32_STARTUP_0_RCOSC );
#endif

#ifdef BUILD_USB_CDC
  pm_configure_usb_clock();
#endif

  // Initialize external memory if any.
#ifdef AVR32_SDRAMC
# ifndef BOOTLOADER_EMBLOD
  sdramc_init( REQ_CPU_FREQ );
# endif
#endif

  // Setup timers
  for( i = 0; i < 3; i ++ )
  {
    tc_init_waveform( tc, i, &tmropt );
#ifndef FOSC32
    // At reset, timers run from the 32768Hz crystal. If there is no such clock
    // then run them all at the lowest frequency available (PBA_FREQ / 128)
    platform_timer_set_clock( i, REQ_PBA_FREQ / 128 );
#endif
  }

#ifdef BUILD_ADC
  (&AVR32_ADC)->ier = AVR32_ADC_DRDY_MASK;
  INTC_register_interrupt( &adc_int_handler, AVR32_ADC_IRQ, AVR32_INTC_INT0);

  for( i = 0; i < NUM_ADC; i++ )
    adc_init_ch_state( i );
#endif

#if NUM_PWM > 0
  pwm_init();
#endif

#ifdef BUILD_UIP
  platform_ethernet_setup();
#endif

#ifdef ELUA_BOARD_MIZAR32
  // If BUF_ENABLE_UART is enabled on Mizar32 (which it is by default) but the
  // serial board is not plugged in, we get an infinite number of interrupts
  // due to the RX pin picking up electrical noise and crashing the board.
  // We avoid this by enabling the internal pull-up resistor on that pin
  // before the UART interrupt is enabled.
  // UART0 RX pin is on GPIO port A pin 0, hence port 0, pin mask (1 << 0)
  platform_pio_op( 0, ( pio_type )1 << 0 , PLATFORM_IO_PIN_PULLUP );
#endif

  // Setup system timer
  // NOTE: this MUST come AFTER pwm_init!
  cmn_systimer_set_base_freq( 1000000 );
  cmn_systimer_set_interrupt_freq( 1 );
  platform_systimer_init();

  // Setup virtual timers if needed
#if VTMR_NUM_TIMERS > 0
  platform_cpu_set_interrupt( INT_TMR_MATCH, VTMR_CH, PLATFORM_CPU_ENABLE );
  platform_timer_set_match_int( VTMR_CH, 1000000 / VTMR_FREQ_HZ, PLATFORM_TIMER_INT_CYCLIC );
#endif // #if VTMR_NUM_TIMERS > 0

#ifdef BUILD_USB_CDC
  usb_init();
#endif

  cmn_platform_init();

  // All done
  return PLATFORM_OK;
}

// ****************************************************************************
// PIO functions

pio_type platform_pio_op( unsigned port, pio_type pinmask, int op )
{
  pio_type retval = 1;

  // Pointer to the register set for this GPIO port
  volatile avr32_gpio_port_t *gpio_regs = &AVR32_GPIO.port[ port ];

  switch( op )
  {
    case PLATFORM_IO_PORT_SET_VALUE:
      gpio_regs->ovr = pinmask;
      break;

    case PLATFORM_IO_PIN_SET:
      gpio_regs->ovrs = pinmask;
      break;

    case PLATFORM_IO_PIN_CLEAR:
      gpio_regs->ovrc = pinmask;
      break;

    case PLATFORM_IO_PORT_DIR_INPUT:
      pinmask = 0xFFFFFFFF;
    case PLATFORM_IO_PIN_DIR_INPUT:
      gpio_regs->oderc = pinmask;  // Disable output drivers
      gpio_regs->gpers = pinmask;  // Make GPIO control this pin
      break;

    case PLATFORM_IO_PORT_DIR_OUTPUT:
      pinmask = 0xFFFFFFFF;
    case PLATFORM_IO_PIN_DIR_OUTPUT:
      gpio_regs->oders = pinmask;  // Enable output drivers
      gpio_regs->gpers = pinmask;  // Make GPIO control this pin
      break;

    case PLATFORM_IO_PORT_GET_VALUE:
      retval = gpio_regs->pvr;
      break;

    case PLATFORM_IO_PIN_GET:
      retval = gpio_regs->pvr & pinmask ? 1 : 0;
      break;

    case PLATFORM_IO_PIN_PULLUP:
      gpio_regs->puers = pinmask;
      break;

    case PLATFORM_IO_PIN_NOPULL:
      gpio_regs->puerc = pinmask;
      break;

    default:
      retval = 0;
  }
  return retval;
}

// ****************************************************************************
// UART functions

static const gpio_map_t uart_pins =
{
  // UART 0
  { AVR32_USART0_RXD_0_0_PIN, AVR32_USART0_RXD_0_0_FUNCTION },
  { AVR32_USART0_TXD_0_0_PIN, AVR32_USART0_TXD_0_0_FUNCTION },

  // UART 1
  { AVR32_USART1_RXD_0_0_PIN, AVR32_USART1_RXD_0_0_FUNCTION },
  { AVR32_USART1_TXD_0_0_PIN, AVR32_USART1_TXD_0_0_FUNCTION },

#if NUM_UART > 2

  // UART 2
  { AVR32_USART2_RXD_0_0_PIN, AVR32_USART2_RXD_0_0_FUNCTION },
  { AVR32_USART2_TXD_0_0_PIN, AVR32_USART2_TXD_0_0_FUNCTION },

#ifdef AVR32_USART3_ADDRESS
  // UART 3
  { AVR32_USART3_RXD_0_0_PIN, AVR32_USART3_RXD_0_0_FUNCTION },
  { AVR32_USART3_TXD_0_0_PIN, AVR32_USART3_TXD_0_0_FUNCTION },
#endif

#endif
};

#ifdef BUILD_USB_CDC
static void avr32_usb_cdc_send( u8 data );
static int avr32_usb_cdc_recv( s32 timeout );
#endif

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  volatile avr32_usart_t *pusart = ( volatile avr32_usart_t* )uart_base_addr[ id ];
  usart_options_t opts;

  opts.channelmode = USART_NORMAL_CHMODE;
  opts.charlength = databits;
  opts.baudrate = baud;

  if( id == CDC_UART_ID )
    return 0;  

  // Set stopbits
#if PLATFORM_UART_STOPBITS_1 == USART_1_STOPBIT && \
    PLATFORM_UART_STOPBITS_1_5 == USART_1_5_STOPBIT && \
    PLATFORM_UART_STOPBITS_2 == USART_2_STOPBIT
  // The AVR32 header values and the eLua values are the same (0, 1, 2)
  if (stopbits > PLATFORM_UART_STOPBITS_2) return 0;
  opts.stopbits = stopbits;
#else
  switch (stopbits) {
  case PLATFORM_UART_STOPBITS_1:
    opts.stopbits = USART_1_STOPBIT;
    break;
  case PLATFORM_UART_STOPBITS_1_5:
    opts.stopbits = USART_1_5_STOPBITS;
    break;
  case PLATFORM_UART_STOPBITS_2:
    opts.stopbits = USART_2_STOPBITS;
    break;
  default:
    return 0;
  }
#endif

  // Set parity
  switch (parity) {
  case PLATFORM_UART_PARITY_EVEN:
    opts.paritytype = USART_EVEN_PARITY;
    break;
  case PLATFORM_UART_PARITY_ODD:
    opts.paritytype = USART_ODD_PARITY;
    break;
  case PLATFORM_UART_PARITY_SPACE:
    opts.paritytype = USART_SPACE_PARITY;
    break;
  case PLATFORM_UART_PARITY_MARK:
    opts.paritytype = USART_MARK_PARITY;
    break;
  case PLATFORM_UART_PARITY_NONE:
    opts.paritytype = USART_NO_PARITY;
    break;
  default:
    return 0;
  }

  // Set actual interface
  gpio_enable_module(uart_pins + id * 2, 2 );
  if ( usart_init_rs232( pusart, &opts, REQ_PBA_FREQ ) != USART_SUCCESS )
    return 0;

  // Return actual baud here
  return usart_get_async_baudrate(pusart, REQ_PBA_FREQ);
}

void platform_s_uart_send( unsigned id, u8 data )
{
  volatile avr32_usart_t *pusart = ( volatile avr32_usart_t* )uart_base_addr[ id ];

#ifdef BUILD_USB_CDC
  if( id == CDC_UART_ID )
    avr32_usb_cdc_send( data );
  else
#endif
  {
    while( !usart_tx_ready( pusart ) );
    pusart->thr = ( data << AVR32_USART_THR_TXCHR_OFFSET ) & AVR32_USART_THR_TXCHR_MASK;
  }
}

int platform_s_uart_recv( unsigned id, timer_data_type timeout )
{
  volatile avr32_usart_t *pusart = ( volatile avr32_usart_t* )uart_base_addr[ id ];
  int temp;

#ifdef BUILD_USB_CDC
  if( id == CDC_UART_ID )
    return avr32_usb_cdc_recv( timeout );
#endif

  if( timeout == 0 )
  {
    if( usart_read_char( pusart, &temp ) != USART_SUCCESS )
      return -1;
    else
      return temp;
  }
  else
    return usart_getchar( pusart );
}

typedef struct
{
  u8 pin;
  u8 function;
} gpio_pin_data;

// This is a complete hack and it will stay like this until eLua will be able
// to specify what pins to use for a peripheral at runtime
static const gpio_pin_data uart_flow_control_pins[] =
{
#ifdef AVR32_USART0_RTS_0_0_PIN
  // UART 0
  { AVR32_USART0_RTS_0_0_PIN, AVR32_USART0_RTS_0_0_FUNCTION },
  { AVR32_USART0_CTS_0_0_PIN, AVR32_USART0_CTS_0_0_FUNCTION },
#else
// UART 0
  { AVR32_USART0_RTS_0_PIN, AVR32_USART0_RTS_0_FUNCTION },
  { AVR32_USART0_CTS_0_PIN, AVR32_USART0_CTS_0_FUNCTION },
#endif

  // UART 1
  { AVR32_USART1_RTS_0_0_PIN, AVR32_USART1_RTS_0_0_FUNCTION },
  { AVR32_USART1_CTS_0_0_PIN, AVR32_USART1_CTS_0_0_FUNCTION },

#if NUM_UART > 2

#ifdef AVR32_USART2_RTS_0_0_PIN
  // UART 2
  { AVR32_USART2_RTS_0_0_PIN, AVR32_USART2_RTS_0_0_FUNCTION },
  { AVR32_USART2_CTS_0_0_PIN, AVR32_USART2_CTS_0_0_FUNCTION },
#else
  // UART 2
  { AVR32_USART2_RTS_0_PIN, AVR32_USART2_RTS_0_FUNCTION },
  { AVR32_USART2_CTS_0_PIN, AVR32_USART2_CTS_0_FUNCTION },
#endif

#ifdef AVR32_USART3_ADDRESS
  // UART 3
  { AVR32_USART3_RTS_0_0_PIN, AVR32_USART3_RTS_0_0_FUNCTION },
  { AVR32_USART3_CTS_0_0_PIN, AVR32_USART3_CTS_0_0_FUNCTION },
#endif

#endif
};


int platform_s_uart_set_flow_control( unsigned id, int type )
{
  unsigned i;
  volatile avr32_usart_t *pusart = ( volatile avr32_usart_t* )uart_base_addr[ id ];
  volatile avr32_gpio_port_t *gpio_port;
  const gpio_pin_data *ppindata = uart_flow_control_pins + id * 2;

  // AVR32 only supports combined RTS/CTS flow control
  if( type != PLATFORM_UART_FLOW_NONE && type != ( PLATFORM_UART_FLOW_RTS | PLATFORM_UART_FLOW_CTS ) )
    return PLATFORM_ERR;
  // Set UART mode register first
  pusart->mr &= ~AVR32_USART_MR_MODE_MASK;
  pusart->mr |= ( type == PLATFORM_UART_FLOW_NONE ? AVR32_USART_MR_MODE_NORMAL : AVR32_USART_MR_MODE_HARDWARE ) << AVR32_USART_MR_MODE_OFFSET;
  // Then set GPIO pins
  for( i = 0; i < 2; i ++, ppindata ++ )
    if( type != PLATFORM_UART_FLOW_NONE ) // enable pin for UART functionality
      gpio_enable_module_pin( ppindata->pin, ppindata->function );
    else // release pin to GPIO module
    {
      gpio_port = &AVR32_GPIO.port[ ppindata->pin >> 5 ];
      gpio_port->gpers = 1 << ( ppindata->pin & 0x1F );
    }
  return PLATFORM_OK;
}

// ****************************************************************************
// Timer functions

static const u16 clkdivs[] = { 0xFFFF, 2, 8, 32, 128 };
u8 avr32_timer_int_periodic_flag[ TC_NUMBER_OF_CHANNELS ];

// Helper: get timer clock
static u32 platform_timer_get_clock( unsigned id )
{
  volatile avr32_tc_t *tc = &AVR32_TC;
  unsigned int clksel = tc->channel[ id ].CMR.waveform.tcclks;

#ifdef FOSC32
  return clksel == 0 ? FOSC32 : REQ_PBA_FREQ / clkdivs[ clksel ];
#else
  return REQ_PBA_FREQ / clkdivs[ clksel ];
#endif
}

// Helper: set timer clock
static u32 platform_timer_set_clock( unsigned id, u32 clock )
{
  unsigned i, mini;
  volatile avr32_tc_t *tc = &AVR32_TC;
  volatile unsigned long *pclksel = &tc->channel[ id ].cmr;

#ifdef FOSC32
  for( i = mini = 0; i < 5; i ++ )
    if( ABSDIFF( clock, i == 0 ? FOSC32 : REQ_PBA_FREQ / clkdivs[ i ] ) <
        ABSDIFF( clock, mini == 0 ? FOSC32 : REQ_PBA_FREQ / clkdivs[ mini ] ) )
      mini = i;
  *pclksel = ( *pclksel & ~0x07 ) | mini;
  return mini == 0 ? FOSC32 : REQ_PBA_FREQ / clkdivs[ mini ];
#else
  // There is no 32768Hz clock so choose from the divisors of PBA.
  for( i = mini = 1; i < 5; i ++ )
    if( ABSDIFF( clock, REQ_PBA_FREQ / clkdivs[ i ] ) <
        ABSDIFF( clock, REQ_PBA_FREQ / clkdivs[ mini ] ) )
      mini = i;
  *pclksel = ( *pclksel & ~0x07 ) | mini;
  return REQ_PBA_FREQ / clkdivs[ mini ];
#endif
}

void platform_s_timer_delay( unsigned id, timer_data_type delay_us )
{
  volatile avr32_tc_t *tc = &AVR32_TC;
  u32 freq;
  timer_data_type final;
  volatile int i;
  volatile const avr32_tc_sr_t *sr = &tc->channel[ id ].SR;

  freq = platform_timer_get_clock( id );
  final = ( ( u64 )delay_us * freq ) / 1000000;
  if( final > 0xFFFF )
    final = 0xFFFF;
  tc_start( tc, id );
  i = sr->covfs;
  for( i = 0; i < 200; i ++ );
  while( ( tc_read_tc( tc, id ) < final ) && !sr->covfs );
}

timer_data_type platform_s_timer_op( unsigned id, int op, timer_data_type data )
{
  u32 res = 0;
  volatile int i;
  volatile avr32_tc_t *tc = &AVR32_TC;

  switch( op )
  {
    case PLATFORM_TIMER_OP_START:
      res = 0;
      tc_start( tc, id );
      for( i = 0; i < 200; i ++ );
      break;

    case PLATFORM_TIMER_OP_READ:
      res = tc_read_tc( tc, id );
      break;

    case PLATFORM_TIMER_OP_SET_CLOCK:
      res = platform_timer_set_clock( id, data );
      break;

    case PLATFORM_TIMER_OP_GET_CLOCK:
      res = platform_timer_get_clock( id );
      break;

    case PLATFORM_TIMER_OP_GET_MAX_CNT:
      res = 0xFFFF;
      break;
  }
  return res;
}

int platform_s_timer_set_match_int( unsigned id, timer_data_type period_us, int type )
{
  volatile avr32_tc_t *tc = &AVR32_TC;
  u64 final;

  if( period_us == 0 )
  {
    tc->channel[ id ].CMR.waveform.wavsel = TC_WAVEFORM_SEL_UP_MODE;
    return PLATFORM_TIMER_INT_OK;
  }
  final = ( u64 )platform_timer_get_clock( id ) * period_us / 1000000;
  if( final == 0 )
    return PLATFORM_TIMER_INT_TOO_SHORT;
  if( final > 0xFFFF )
    return PLATFORM_TIMER_INT_TOO_LONG;
  tc_stop( tc, id );
  tc->channel[ id ].CMR.waveform.wavsel = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER;
  tc->channel[ id ].rc = ( u32 )final;
  avr32_timer_int_periodic_flag[ id ] = type;
  tc_start( tc, id );
  return PLATFORM_TIMER_INT_OK;
}

u64 platform_timer_sys_raw_read()
{
  return AVR32_PWM.channel[ SYSTIMER_PWM_CH ].ccnt;
}

void platform_timer_sys_disable_int()
{
  AVR32_PWM.idr = 1 << SYSTIMER_PWM_CH;
}

void platform_timer_sys_enable_int()
{
  AVR32_PWM.ier = 1 << SYSTIMER_PWM_CH;
}

timer_data_type platform_timer_read_sys()
{
  return cmn_systimer_get();
}

// ****************************************************************************
// SPI functions

/* Note about AVR32 SPI
 *
 * Each controller can handle up to 4 different settings.
 * Here, for convenience, we don't use the builtin chip select lines,
 * it's up to the user to drive the corresponding GPIO lines.
 *
*/
static const gpio_map_t spi_pins =
{
  // SPI0
  { BOARD_SPI0_SCK_PIN, BOARD_SPI0_SCK_PIN_FUNCTION },
  { BOARD_SPI0_MISO_PIN, BOARD_SPI0_MISO_PIN_FUNCTION },
  { BOARD_SPI0_MOSI_PIN, BOARD_SPI0_MOSI_PIN_FUNCTION },
  { BOARD_SPI0_CS_PIN, BOARD_SPI0_CS_PIN_FUNCTION },

  // SPI1
#if NUM_SPI > 4
  { BOARD_SPI1_SCK_PIN, BOARD_SPI1_SCK_PIN_FUNCTION },
  { BOARD_SPI1_MISO_PIN, BOARD_SPI1_MISO_PIN_FUNCTION },
  { BOARD_SPI1_MOSI_PIN, BOARD_SPI1_MOSI_PIN_FUNCTION },
  { BOARD_SPI1_CS_PIN, BOARD_SPI1_CS_PIN_FUNCTION },
#endif
};

static const
u32 spireg[] =
{
  AVR32_SPI0_ADDRESS,
#ifdef AVR32_SPI1_ADDRESS
  AVR32_SPI1_ADDRESS,
#endif
};

// Enabling of the SPI clocks is deferred until platform_spi_setup() is called
// for the first time so that, if you don't use the SPI ports and don't
// have MMCFS enabled, power consumption is reduced.

// Initialise the specified SPI controller (== id / 4) as a master
static void spi_init_master( unsigned controller )
{
  static const spi_master_options_t spiopt = {
    .modfdis = TRUE,
    .pcs_decode = FALSE,
    .delay = 0,
  };
  static bool spi_is_master[ (NUM_SPI + 3) / 4];  // initialized as 0

  if ( ! spi_is_master[controller] ) {
    spi_initMaster(
#if NUM_SPI <= 4
                   &AVR32_SPI0,
#else
		   &AVR32_SPI0 + ( controller * ( &AVR32_SPI1 - &AVR32_SPI0 ) ),
#endif
                   &spiopt, REQ_PBA_FREQ);
    spi_is_master[controller]++;
  }
}

u32 platform_spi_setup( unsigned id, int mode, u32 clock, unsigned cpol, unsigned cpha, unsigned databits )
{
#if NUM_SPI > 0
  spi_options_t opt;

  spi_init_master(id >> 2);

  opt.baudrate = clock;
  opt.bits = min(databits, 16);
  opt.spck_delay = 0;
  opt.trans_delay = 0;
  opt.mode = (cpol << 1) | cpha;

  // Set actual interface
  gpio_enable_module(spi_pins + (id >> 2) * 4, 4);
  return spi_setupChipReg((volatile avr32_spi_t *) spireg[id >> 2], id % 4,
                          &opt, REQ_PBA_FREQ);
#else
  return 0;
#endif
}

spi_data_type platform_spi_send_recv( unsigned id, spi_data_type data )
{
  volatile avr32_spi_t * spi = (volatile avr32_spi_t *) spireg[id >> 2];

  /* Since none of the builtin chip select lines are externally wired,
   * spi_selectChip() just ensure that the correct spi settings are
   * used for the transfer.
   */
  spi_selectChip(spi, id % 4);
  return spi_single_transfer(spi, (u16) data);
}

void platform_spi_select( unsigned id, int is_select )
{
  volatile avr32_spi_t * spi = (volatile avr32_spi_t *) spireg[id >> 2];

  if(is_select == PLATFORM_SPI_SELECT_ON)
    spi_selectChip(spi, id % 4);
  else
    spi_unselectChip(spi, id % 4);

}
// ****************************************************************************
// CPU functions

int platform_cpu_set_global_interrupts( int status )
{
  int previous = Is_global_interrupt_enabled();

  if( status == PLATFORM_CPU_ENABLE )
    Enable_global_interrupt();
  else
    Disable_global_interrupt();
  return previous;
}

int platform_cpu_get_global_interrupts()
{
  return Is_global_interrupt_enabled();
}

// ****************************************************************************
// ADC functions

#ifdef BUILD_ADC

static const gpio_map_t adc_pins =
{
  {AVR32_ADC_AD_0_PIN, AVR32_ADC_AD_0_FUNCTION},
  {AVR32_ADC_AD_1_PIN, AVR32_ADC_AD_1_FUNCTION},
  {AVR32_ADC_AD_2_PIN, AVR32_ADC_AD_2_FUNCTION},
  {AVR32_ADC_AD_3_PIN, AVR32_ADC_AD_3_FUNCTION},
  {AVR32_ADC_AD_4_PIN, AVR32_ADC_AD_4_FUNCTION},
  {AVR32_ADC_AD_5_PIN, AVR32_ADC_AD_5_FUNCTION},
  {AVR32_ADC_AD_6_PIN, AVR32_ADC_AD_6_FUNCTION},
  {AVR32_ADC_AD_7_PIN, AVR32_ADC_AD_7_FUNCTION}
};

volatile avr32_adc_t *adc = &AVR32_ADC;

int platform_adc_check_timer_id( unsigned id, unsigned timer_id )
{
  return 0; // no timers supported initially
}

void platform_adc_stop( unsigned id )
{
  elua_adc_ch_state *s = adc_get_ch_state( id );
  elua_adc_dev_state *d = adc_get_dev_state( 0 );

  s->op_pending = 0;
  INACTIVATE_CHANNEL( d, id );

  adc_disable( adc, s->id );

  // If there are no more active channels, stop the sequencer
  if( d->ch_active == 0 )
    d->running = 0;
}

int platform_adc_update_sequence( )
{
  elua_adc_dev_state *d = adc_get_dev_state( 0 );

  adc->cr = AVR32_ADC_SWRST_MASK;
  adc->ier = AVR32_ADC_DRDY_MASK;

  adc_configure( adc );

  d->seq_ctr = 0;
  while( d->seq_ctr < d->seq_len )
  {
    adc_enable( adc, d->ch_state[ d->seq_ctr ]->id );
    gpio_enable_module( adc_pins + d->ch_state[ d->seq_ctr ]->id, 1 );
    d->seq_ctr++;
  }
  d->seq_ctr = 0;

  return PLATFORM_OK;
}

__attribute__((__interrupt__)) static void adc_int_handler()
{
  elua_adc_dev_state *d = adc_get_dev_state( 0 );
  elua_adc_ch_state *s;

  d->seq_ctr = 0;
  while( d->seq_ctr < d->seq_len )
  {
    s = d->ch_state[ d->seq_ctr ];

    if( adc_check_eoc( adc, s->id ) )
    {
      d->sample_buf[ s->id ] = ( u16 )adc_get_value(adc, s->id );
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

      // If we have the number of requested samples, stop sampling
      if ( adc_samples_available( s->id ) >= s->reqsamples && s->freerunning == 0 )
        platform_adc_stop( s->id );
    }

    d->seq_ctr++;
  }
  d->seq_ctr = 0;

  adc_update_dev_sequence( 0 );

  if ( d->clocked == 0 && d->running == 1 )
    adc_start( adc );
}


u32 platform_adc_set_clock( unsigned id, u32 frequency )
{
  return 0;
}


int platform_adc_start_sequence( )
{
  elua_adc_dev_state *d = adc_get_dev_state( 0 );

  // Only force update and initiate if we weren't already running
  // changes will get picked up during next interrupt cycle
  if ( d->running != 1 )
  {
    adc_update_dev_sequence( 0 );

    d->seq_ctr = 0;
    d->running = 1;

    if( d->clocked == 0 )
      adc_start(adc);
  }

  return PLATFORM_OK;
}

#endif

// ****************************************************************************
// PWM functions

// Sanity check
#if NUM_PWM > AVR32_PWM_CHANNEL_LENGTH
# error "NUM_PWM > AVR32_PWM_CHANNEL_LENGTH"
#endif

#if NUM_PWM > 0

// One PWM channel is used by the AVR32 system timer (look at the start of this
// file for more information). Currently this channel is hardcoded in platform.c
// (SYSTIMER_PWM_CH) to 6. If this is not convenient feel free to move the
// definition of SYSTIMER_PWM_CH in platform_conf.h and select another PWM channel,
// BUT remember to modify the below PWM pin mapping accordingly!
static const gpio_map_t pwm_pins =
{
#if ( BOARD == ATEVK1100 ) || ( BOARD == MIZAR32 )
  { AVR32_PWM_0_PIN, AVR32_PWM_0_FUNCTION },      // PB19 - LED4
  { AVR32_PWM_1_PIN, AVR32_PWM_1_FUNCTION },      // PB20 - LED5
  { AVR32_PWM_2_PIN, AVR32_PWM_2_FUNCTION },	  // PB21 - LED6
  { AVR32_PWM_3_PIN, AVR32_PWM_3_FUNCTION },      // PB22 - LED7
  { AVR32_PWM_4_1_PIN, AVR32_PWM_4_1_FUNCTION },  // PB27 - LED0
  { AVR32_PWM_5_1_PIN, AVR32_PWM_5_1_FUNCTION },  // PB28 - LED1
//  { AVR32_PWM_6_PIN, AVR32_PWM_6_FUNCTION },      // PB18 - LCD_C / GPIO50
#elif BOARD == ATEVK1101
  { AVR32_PWM_0_0_PIN, AVR32_PWM_0_0_FUNCTION },  // PA7  LED0
  { AVR32_PWM_1_0_PIN, AVR32_PWM_1_0_FUNCTION },  // PA8  LED1
  { AVR32_PWM_2_0_PIN, AVR32_PWM_2_0_FUNCTION },  // PA21 LED2
  { AVR32_PWM_3_0_PIN, AVR32_PWM_3_0_FUNCTION },  // PA14 ? or _1 PA25
  { AVR32_PWM_4_1_PIN, AVR32_PWM_4_1_FUNCTION },  // PA28 - audio out
  { AVR32_PWM_5_1_PIN, AVR32_PWM_5_1_FUNCTION },  // PB5: UART1-RTS & Nexus i/f EVTIn / _0 PA18=Xin0
//  { AVR32_PWM_6_0_PIN, AVR32_PWM_6_0_FUNCTION },  // PA22 - LED3 and audio out
#endif
};


/*
 * Configure a PWM channel to run at "frequency" Hz with a duty cycle of
 * "duty" (0-100).  0 means low all the time, 100 high all the time.
 * Return actual frequency set.
 */
u32 platform_pwm_setup( unsigned id, u32 frequency, unsigned duty )
{
  u32 pwmclk;        // base clock frequency for PWM counters
  u32 period;        // number of base clocks per cycle
  u32 duty_cycle;    // number of base clocks to be high (low?) for

  // Sanity checks
  if (id >= NUM_PWM || duty > 100)
    return 0;    // Returning an actual frequency of 0 should worry them!

  gpio_enable_module(pwm_pins + id, 1 );

  pwmclk = pwm_get_clock_freq();

  // Compute period and duty period in clock cycles.
  //
  // PWM output wave frequency is requested in Hz but programmed as a
  // number of cycles of the master PWM clock frequency.
  //
  // Here, we use rounding to select the numerically closest available
  // frequency and return the closest integer in Hz to that.

  period = (pwmclk + frequency/2) / frequency;
  if (period == 0) period = 1;
  if (period >= 1<<20) period = (1<<20) - 1;
  duty_cycle = (period * duty + 50) / 100;

  // The AVR32 PWM duty cycle is upside down:
  // duty_period==0 gives an all-active output, while
  // duty_period==period gives an all-inactive output.
  // So we invert the duty cycle here.
  pwm_channel_set_period_and_duty_cycle( id, period, period - duty_cycle );

  return (pwmclk + period/2) / period;
}

/*
 * Helper function:
 * Find a prescaler/divisor couple to generate the closest available
 * clock frequency.
 * Dumps the "pre" and "div" values for MR in *pre and *div.
 * If the configuration cannot be met (because freq is too high), set the
 * maximum frequency possible.
 *
 * The algorithm is too simple: the actual clock frequency is always >=
 * the one requested, not the closest possible.
 */
static void pwm_find_clock_configuration( u32 frequency,
                                          unsigned *pre, unsigned *div )
{
  // prescalers[11] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
#define prescalers( n ) ( 1 << n )
  const unsigned nprescalers = 11;

  unsigned prescaler;	// Which of the prescalers are we considering?
  unsigned divisor;

  if ( frequency > REQ_PBA_FREQ )
  {
    *pre = 0;    // Select master clock frequency
    *div = 1;    // divided by one
    return;
  }

  // Find prescaler and divisor values
  prescaler = 0;
  do
    divisor = REQ_PBA_FREQ / ( prescalers( prescaler ) * frequency );
  while ( ( divisor > 255 ) && ( ++prescaler < nprescalers ) );

  // Return result
  if ( prescaler < nprescalers )
  {
    *pre = prescaler;
    *div = divisor;
  } else {
    // It failed because the frequency is too low.
    // Set the lowest possible frequency.
    *pre = nprescalers - 1;
    *div = 255;
  }
  return;
}
#undef prescalers

u32 platform_pwm_set_clock( unsigned id, u32 freq )
{
  unsigned pre, div;

  pwm_find_clock_configuration( freq, &pre, &div );
  pwm_set_linear_divider( pre, div );

  return pwm_get_clock_freq();
}

u32 platform_pwm_get_clock( unsigned id )
{
  return pwm_get_clock_freq();
}

void platform_pwm_start( unsigned id )
{
  pwm_channel_start( id );
}

void platform_pwm_stop( unsigned id )
{
  pwm_channel_stop( id );
}

#endif // #if NUM_PWM > 0

// ****************************************************************************
// I2C support

u32 platform_i2c_setup( unsigned id, u32 speed )
{
  return i2c_setup(speed);
}

void platform_i2c_send_start( unsigned id )
{
  i2c_start_cond();
}

void platform_i2c_send_stop( unsigned id )
{
  i2c_stop_cond();
}

int platform_i2c_send_address( unsigned id, u16 address, int direction )
{
  // Convert enum codes to R/w bit value.
  // If TX == 0 and RX == 1, this test will be removed by the compiler
  if ( ! ( PLATFORM_I2C_DIRECTION_TRANSMITTER == 0 &&
           PLATFORM_I2C_DIRECTION_RECEIVER == 1 ) ) {
    direction = ( direction == PLATFORM_I2C_DIRECTION_TRANSMITTER ) ? 0 : 1;
  }

  // Low-level returns nack (0=acked); we return ack (1=acked).
  return ! i2c_write_byte( (address << 1) | direction );
}

int platform_i2c_send_byte( unsigned id, u8 data )
{
  // Low-level returns nack (0=acked); we return ack (1=acked).
  return ! i2c_write_byte( data );
}

int platform_i2c_recv_byte( unsigned id, int ack )
{
  return i2c_read_byte( !ack );
}


// ****************************************************************************
// Network support

#ifdef BUILD_UIP
static const gpio_map_t MACB_GPIO_MAP =
{
  { AVR32_MACB_MDC_0_PIN,    AVR32_MACB_MDC_0_FUNCTION    },
  { AVR32_MACB_MDIO_0_PIN,   AVR32_MACB_MDIO_0_FUNCTION   },
  { AVR32_MACB_RXD_0_PIN,    AVR32_MACB_RXD_0_FUNCTION    },
  { AVR32_MACB_TXD_0_PIN,    AVR32_MACB_TXD_0_FUNCTION    },
  { AVR32_MACB_RXD_1_PIN,    AVR32_MACB_RXD_1_FUNCTION    },
  { AVR32_MACB_TXD_1_PIN,    AVR32_MACB_TXD_1_FUNCTION    },
  { AVR32_MACB_TX_EN_0_PIN,  AVR32_MACB_TX_EN_0_FUNCTION  },
  { AVR32_MACB_RX_ER_0_PIN,  AVR32_MACB_RX_ER_0_FUNCTION  },
  { AVR32_MACB_RX_DV_0_PIN,  AVR32_MACB_RX_DV_0_FUNCTION  },
  { AVR32_MACB_TX_CLK_0_PIN, AVR32_MACB_TX_CLK_0_FUNCTION },
};

u32 platform_ethernet_setup()
{
  static struct uip_eth_addr sTempAddr = {
    .addr[0] = ETHERNET_CONF_ETHADDR0,
    .addr[1] = ETHERNET_CONF_ETHADDR1,
    .addr[2] = ETHERNET_CONF_ETHADDR2,
    .addr[3] = ETHERNET_CONF_ETHADDR3,
    .addr[4] = ETHERNET_CONF_ETHADDR4,
    .addr[5] = ETHERNET_CONF_ETHADDR5,
  };

  // Assign GPIO to MACB
  gpio_enable_module( MACB_GPIO_MAP, sizeof(MACB_GPIO_MAP ) / sizeof( MACB_GPIO_MAP[0] ) );

  // initialize MACB & Phy Layers
  if ( xMACBInit( &AVR32_MACB ) == FALSE ) {
    return PLATFORM_ERR;
  }

  // Initialize the eLua uIP layer
  elua_uip_init( &sTempAddr );
  return PLATFORM_OK;
}

void platform_eth_send_packet( const void* src, u32 size )
{
  lMACBSend( &AVR32_MACB,src, size, TRUE );
}

u32 platform_eth_get_packet_nb( void* buf, u32 maxlen )
{
  u32 len;

  /* Obtain the size of the packet. */
  len = ulMACBInputLength();

  if( len > maxlen ) {
    return 0;
  }

  if( len ) {
    /* Let the driver know we are going to read a new packet. */
    vMACBRead( NULL, 0, len );
    vMACBRead( buf, len, len );
  }

  return len;
}

void platform_eth_force_interrupt()
{
  elua_uip_mainloop();
}

u32 platform_eth_get_elapsed_time()
{
  if( eth_timer_fired )
  {
    eth_timer_fired = 0;
    return SYSTICKMS;
  }
  else
    return 0;
}

void platform_eth_timer_handler()
{
  // Indicate that a SysTick interrupt has occurred.
  eth_timer_fired = 1;

  // Generate a fake Ethernet interrupt.  This will perform the actual work
  // of incrementing the timers and taking the appropriate actions.
  platform_eth_force_interrupt();
}

#else // #ifdef BUILD_UIP

void platform_eth_timer_handler()
{
}

#endif // #ifdef BUILD_UIP

#ifdef BUILD_USB_CDC

static void avr32_usb_cdc_send( u8 data )
{
  if (!Is_device_enumerated())
    return;
  while(!UsbCdcTxReady());      // "USART"-USB free ?
  UsbCdcSendChar(data);
}

static int avr32_usb_cdc_recv( s32 timeout )
{
  int data;
  int read;

  if (!Is_device_enumerated())
    return -1;

  // Try to read one byte from buffer, if none available return -1 or
  // retry forever if timeout != 0 ( = PLATFORM_TIMER_INF_TIMEOUT)
  do {
    read = UsbCdcReadChar(&data);
  } while( read == 0 && timeout != 0 );

  if( read == 0 )
    return -1;
  else
    return data;
}

void platform_cdc_timer_handler()
{
  usb_device_task();
  UsbCdcFlush ();
}
#else
void platform_cdc_timer_handler()
{
}
#endif // #ifdef BUILD_USB_CDC

