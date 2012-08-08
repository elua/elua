// Platform-dependent functions

#include "platform.h"
#include "type.h"
#include "devman.h"
#include "genstd.h"
#include <reent.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "uip_arp.h"
#include "elua_uip.h"
#include "elua_adc.h"
#include "uip-conf.h"
#include "platform_conf.h"
#include "common.h"
#include "math.h"
#include "diskio.h"
#include "lua.h"
#include "lauxlib.h"
#include "lrotable.h"
#include "elua_int.h" 

// Platform specific includes

#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/can.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/ssi.h"
#include "driverlib/timer.h"
#include "driverlib/pwm.h"
#include "driverlib/adc.h"
#include "driverlib/ethernet.h"
#include "driverlib/systick.h"
#include "driverlib/flash.h"
#include "driverlib/interrupt.h"
#include "elua_net.h"
#include "dhcpc.h"
#include "buf.h"
#include "rit128x96x4.h"
#include "disp.h"
#include "utils.h"

#if defined( FORLM3S9B92 )
  #define TARGET_IS_TEMPEST_RB1

  #include "lm3s9b92.h"
#elif defined( FORLM3S9D92 )
  #define TARGET_IS_FIRESTORM_RA2

  #include "lm3s9d92.h"
#elif defined( FORLM3S8962 )
  #include "lm3s8962.h"
#elif defined( FORLM3S6965 )
  #include "lm3s6965.h"
#elif defined( FORLM3S6918 )
  #include "lm3s6918.h"
#endif

#include "driverlib/rom.h"
#include "driverlib/rom_map.h"

// USB CDC Stuff
#include "driverlib/usb.h"
#include "usblib/usblib.h"
#include "usblib/usbcdc.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdcdc.h"
#include "usb_serial_structs.h"

// UIP sys tick data
// NOTE: when using virtual timers, SYSTICKHZ and VTMR_FREQ_HZ should have the
// same value, as they're served by the same timer (the systick)
#define SYSTICKHZ               5
#define SYSTICKMS               (1000 / SYSTICKHZ)

// ****************************************************************************
// Platform initialization

// forward
static void timers_init();
static void uarts_init();
static void spis_init();
static void pios_init();
static void pwms_init();
static void eth_init();
static void adcs_init();
static void cans_init();
static void usb_init();

int platform_init()
{
  // Set the clocking to run from PLL
#if defined( FORLM3S9B92 ) || defined( FORLM3S9D92 )
  MAP_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
#else
  MAP_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ);
#endif

  // Setup PIO
  pios_init();

  // Setup SSIs
  spis_init();

  // Setup UARTs
  uarts_init();

  // Setup timers
  timers_init();

  // Setup PWMs
  pwms_init();

#ifdef BUILD_ADC
  // Setup ADCs
  adcs_init();
#endif

#ifdef BUILD_CAN
  // Setup CANs
  cans_init();
#endif

#ifdef BUILD_USB_CDC
  // Setup USB
  usb_init();
#endif

  // Setup system timer
  cmn_systimer_set_base_freq( MAP_SysCtlClockGet() );
  cmn_systimer_set_interrupt_freq( SYSTICKHZ );

  // Setup ethernet (TCP/IP)
  eth_init();

  // Common platform initialization code
  cmn_platform_init();

  // Virtual timers
  // If the ethernet controller is used the timer is already initialized, so skip this sequence
#if VTMR_NUM_TIMERS > 0 && !defined( BUILD_UIP )
  // Configure SysTick for a periodic interrupt.
  MAP_SysTickPeriodSet( MAP_SysCtlClockGet() / SYSTICKHZ );
  MAP_SysTickEnable();
  MAP_SysTickIntEnable();
  MAP_IntMasterEnable();
#endif

  MAP_FlashUsecSet( SysCtlClockGet() );

  // All done
  return PLATFORM_OK;
}

// ****************************************************************************
// PIO
// Same configuration on LM3S8962, LM3S6965, LM3S6918 (8 ports)
// 9B92 has 9 ports (Port J in addition to A-H)
#if defined( FORLM3S9B92 ) || defined( FORLM3S9D92 )
  const u32 pio_base[] = { GPIO_PORTA_BASE, GPIO_PORTB_BASE, GPIO_PORTC_BASE, GPIO_PORTD_BASE,
                                  GPIO_PORTE_BASE, GPIO_PORTF_BASE, GPIO_PORTG_BASE, GPIO_PORTH_BASE, 
                                  GPIO_PORTJ_BASE };
                                  
  const u32 pio_sysctl[] = { SYSCTL_PERIPH_GPIOA, SYSCTL_PERIPH_GPIOB, SYSCTL_PERIPH_GPIOC, SYSCTL_PERIPH_GPIOD,
                                    SYSCTL_PERIPH_GPIOE, SYSCTL_PERIPH_GPIOF, SYSCTL_PERIPH_GPIOG, SYSCTL_PERIPH_GPIOH,
                                    SYSCTL_PERIPH_GPIOJ };
#else
  const u32 pio_base[] = { GPIO_PORTA_BASE, GPIO_PORTB_BASE, GPIO_PORTC_BASE, GPIO_PORTD_BASE,
                                  GPIO_PORTE_BASE, GPIO_PORTF_BASE, GPIO_PORTG_BASE, GPIO_PORTH_BASE };
  
  const u32 pio_sysctl[] = { SYSCTL_PERIPH_GPIOA, SYSCTL_PERIPH_GPIOB, SYSCTL_PERIPH_GPIOC, SYSCTL_PERIPH_GPIOD,
                                    SYSCTL_PERIPH_GPIOE, SYSCTL_PERIPH_GPIOF, SYSCTL_PERIPH_GPIOG, SYSCTL_PERIPH_GPIOH };
#endif

static void pios_init()
{
  unsigned i;

  for( i = 0; i < NUM_PIO; i ++ )
    MAP_SysCtlPeripheralEnable(pio_sysctl[ i ]);
}

pio_type platform_pio_op( unsigned port, pio_type pinmask, int op )
{
  pio_type retval = 1, base = pio_base[ port ];

  switch( op )
  {
    case PLATFORM_IO_PORT_SET_VALUE:
      MAP_GPIOPinWrite( base, 0xFF, pinmask );
      break;

    case PLATFORM_IO_PIN_SET:
      MAP_GPIOPinWrite( base, pinmask, pinmask );
      break;

    case PLATFORM_IO_PIN_CLEAR:
      MAP_GPIOPinWrite( base, pinmask, 0 );
      break;

    case PLATFORM_IO_PORT_DIR_INPUT:
      pinmask = 0xFF;
    case PLATFORM_IO_PIN_DIR_INPUT:
      MAP_GPIOPinTypeGPIOInput( base, pinmask );
      break;

    case PLATFORM_IO_PORT_DIR_OUTPUT:
      pinmask = 0xFF;
    case PLATFORM_IO_PIN_DIR_OUTPUT:
      MAP_GPIOPinTypeGPIOOutput( base, pinmask );
      break;

    case PLATFORM_IO_PORT_GET_VALUE:
      retval = MAP_GPIOPinRead( base, 0xFF );
      break;

    case PLATFORM_IO_PIN_GET:
      retval = MAP_GPIOPinRead( base, pinmask ) ? 1 : 0;
      break;

    case PLATFORM_IO_PIN_PULLUP:
    case PLATFORM_IO_PIN_PULLDOWN:
      MAP_GPIOPadConfigSet( base, pinmask, GPIO_STRENGTH_8MA, op == PLATFORM_IO_PIN_PULLUP ? GPIO_PIN_TYPE_STD_WPU : GPIO_PIN_TYPE_STD_WPD );
      break;

    case PLATFORM_IO_PIN_NOPULL:
      MAP_GPIOPadConfigSet( base, pinmask, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD );
      break;

    default:
      retval = 0;
      break;
  }
  return retval;
}


// ****************************************************************************
// CAN

#if defined( BUILD_CAN )

volatile u32 can_rx_flag = 0;
volatile u32 can_tx_flag = 0;
volatile u32 can_err_flag = 0;
char can_tx_buf[8];
tCANMsgObject can_msg_rx;

// LM3S9Bxx MCU CAN seems to run off of system clock, LM3S8962 has 8 MHz clock
#if defined( FORLM3S8962 )
#define LM3S_CAN_CLOCK  8000000
#else
#define LM3S_CAN_CLOCK  SysCtlClockGet()
#endif

void CANIntHandler(void)
{
  u32 status = CANIntStatus(CAN0_BASE, CAN_INT_STS_CAUSE);

  if(status == CAN_INT_INTID_STATUS)
  {
    status = CANStatusGet(CAN0_BASE, CAN_STS_CONTROL);
    can_err_flag = 1;
    can_tx_flag = 0;
  }
  else if( status == 1 ) // Message receive
  {
    CANIntClear(CAN0_BASE, 1);
    can_rx_flag = 1;
    can_err_flag = 0;
  }
  else if( status == 2 ) // Message send
  {
    CANIntClear(CAN0_BASE, 2);
    can_tx_flag = 0;
    can_err_flag = 0;
  }
  else
    CANIntClear(CAN0_BASE, status);
}


void cans_init( void )
{
  MAP_SysCtlPeripheralEnable( SYSCTL_PERIPH_CAN0 ); 
  MAP_CANInit( CAN0_BASE );
  CANBitRateSet(CAN0_BASE, LM3S_CAN_CLOCK, 500000);
  MAP_CANIntEnable( CAN0_BASE, CAN_INT_MASTER | CAN_INT_ERROR | CAN_INT_STATUS );
  MAP_IntEnable(INT_CAN0);
  MAP_CANEnable(CAN0_BASE);

  // Configure default catch-all message object
  can_msg_rx.ulMsgID = 0;
  can_msg_rx.ulMsgIDMask = 0;
  can_msg_rx.ulFlags = MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER;
  can_msg_rx.ulMsgLen = 8;
  MAP_CANMessageSet(CAN0_BASE, 1, &can_msg_rx, MSG_OBJ_TYPE_RX);
}


u32 platform_can_setup( unsigned id, u32 clock )
{  
  GPIOPinConfigure(GPIO_PD0_CAN0RX);
  GPIOPinConfigure(GPIO_PD1_CAN0TX);
  MAP_GPIOPinTypeCAN(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  MAP_CANDisable(CAN0_BASE);
  CANBitRateSet(CAN0_BASE, LM3S_CAN_CLOCK, clock );
  MAP_CANEnable(CAN0_BASE);
  return clock;
}

void platform_can_send( unsigned id, u32 canid, u8 idtype, u8 len, const u8 *data )
{
  tCANMsgObject msg_tx;
  const char *s = ( char * )data;
  char *d;

  // Wait for outgoing messages to clear
  while( can_tx_flag == 1 );

  msg_tx.ulFlags = MSG_OBJ_TX_INT_ENABLE;
  
  if( idtype == ELUA_CAN_ID_EXT )
    msg_tx.ulFlags |= MSG_OBJ_EXTENDED_ID;
  
  msg_tx.ulMsgIDMask = 0;
  msg_tx.ulMsgID = canid;
  msg_tx.ulMsgLen = len;
  msg_tx.pucMsgData = ( u8 * )can_tx_buf;

  d = can_tx_buf;
  DUFF_DEVICE_8( len,  *d++ = *s++ );

  can_tx_flag = 1;
  CANMessageSet(CAN0_BASE, 2, &msg_tx, MSG_OBJ_TYPE_TX);
}

int platform_can_recv( unsigned id, u32 *canid, u8 *idtype, u8 *len, u8 *data )
{
  // wait for a message
  if( can_rx_flag != 0 )
  {
    can_msg_rx.pucMsgData = data;
    CANMessageGet(CAN0_BASE, 1, &can_msg_rx, 0);
    can_rx_flag = 0;

    *canid = ( u32 )can_msg_rx.ulMsgID;
    *idtype = ( can_msg_rx.ulFlags & MSG_OBJ_EXTENDED_ID )? ELUA_CAN_ID_EXT : ELUA_CAN_ID_STD;
    *len = can_msg_rx.ulMsgLen;
    return PLATFORM_OK;
  }
  else
    return PLATFORM_UNDERFLOW;
}

#endif

// ****************************************************************************
// SPI
// Same configuration on LM3S8962, LM3S6965, LM3S6918 and LM3S9B92 (2 SPI ports)

// All possible LM3S SPIs defs
// FIXME this anticipates support for a platform with 2 SPI port
//  PIN info extracted from LM3S6950 and 5769 datasheets
static const u32 spi_base[] = { SSI0_BASE, SSI1_BASE };
static const u32 spi_sysctl[] = { SYSCTL_PERIPH_SSI0, SYSCTL_PERIPH_SSI1 };

#if defined( ELUA_BOARD_SOLDERCORE )
static const u32 spi_gpio_base[] = { GPIO_PORTA_BASE, GPIO_PORTF_BASE };
static const u8 spi_gpio_pins[] = {  GPIO_PIN_4 | GPIO_PIN_5,
                                     GPIO_PIN_4 | GPIO_PIN_5 };
//                                   SSIxRx       SSIxTx

static const u32 spi_gpio_clk_base[] = { GPIO_PORTA_BASE, GPIO_PORTH_BASE };
static const u8 spi_gpio_clk_pin[] = { GPIO_PIN_2, GPIO_PIN_4 };
#else
static const u32 spi_gpio_base[] = { GPIO_PORTA_BASE, GPIO_PORTE_BASE };
static const u8 spi_gpio_pins[] = { GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5,
                                    GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 };
//                                  SSIxClk      SSIxFss      SSIxRx       SSIxTx
static const u32 spi_gpio_clk_base[] = { GPIO_PORTA_BASE, GPIO_PORTE_BASE };
static const u8 spi_gpio_clk_pin[] = { GPIO_PIN_2, GPIO_PIN_0 };
#endif


static void spis_init()
{
  unsigned i;

#if defined( ELUA_BOARD_SOLDERCORE )
  GPIOPinConfigure( GPIO_PH4_SSI1CLK );
  GPIOPinConfigure( GPIO_PF4_SSI1RX );
  GPIOPinConfigure( GPIO_PF5_SSI1TX );
#endif

  for( i = 0; i < NUM_SPI; i ++ )
    MAP_SysCtlPeripheralEnable( spi_sysctl[ i ] );
}

u32 platform_spi_setup( unsigned id, int mode, u32 clock, unsigned cpol, unsigned cpha, unsigned databits )
{
  unsigned protocol;

  if( cpol == 0 )
    protocol = cpha ? SSI_FRF_MOTO_MODE_1 : SSI_FRF_MOTO_MODE_0;
  else
    protocol = cpha ? SSI_FRF_MOTO_MODE_3 : SSI_FRF_MOTO_MODE_2;
  mode = mode == PLATFORM_SPI_MASTER ? SSI_MODE_MASTER : SSI_MODE_SLAVE;
  MAP_SSIDisable( spi_base[ id ] );

  MAP_GPIOPinTypeSSI( spi_gpio_base[ id ], spi_gpio_pins[ id ] );
  MAP_GPIOPinTypeSSI( spi_gpio_clk_base[ id ], spi_gpio_clk_pin[ id ] );

  // FIXME: not sure this is always "right"
  GPIOPadConfigSet( spi_gpio_base[ id ], spi_gpio_pins[ id ], GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU );
  GPIOPadConfigSet( spi_gpio_clk_base[ id ], spi_gpio_clk_pin[ id ], GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU );

  MAP_SSIConfigSetExpClk( spi_base[ id ], MAP_SysCtlClockGet(), protocol, mode, clock, databits );
  MAP_SSIEnable( spi_base[ id ] );
  return clock;
}

spi_data_type platform_spi_send_recv( unsigned id, spi_data_type data )
{
  MAP_SSIDataPut( spi_base[ id ], data );
  MAP_SSIDataGet( spi_base[ id ], &data );
  return data;
}

void platform_spi_select( unsigned id, int is_select )
{
  // This platform doesn't have a hardware SS pin, so there's nothing to do here
  id = id;
  is_select = is_select;
}

// ****************************************************************************
// UART
// Different configurations for LM3S8962, LM3S6918 (2 UARTs) and LM3S6965, LM3S9B92 (3 UARTs)

// All possible LM3S uarts defs
const u32 uart_base[] = { UART0_BASE, UART1_BASE, UART2_BASE };
static const u32 uart_sysctl[] = { SYSCTL_PERIPH_UART0, SYSCTL_PERIPH_UART1, SYSCTL_PERIPH_UART2 };
static const u32 uart_gpio_base[] = { GPIO_PORTA_BASE, GPIO_PORTD_BASE, GPIO_PORTG_BASE };
static const u8 uart_gpio_pins[] = { GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_0 | GPIO_PIN_1 };

static void uarts_init()
{
  unsigned i;
  for( i = 0; i < NUM_UART; i ++ )
    MAP_SysCtlPeripheralEnable(uart_sysctl[ i ]);
}

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  u32 config;

  if( id < NUM_UART )
  {
    MAP_GPIOPinTypeUART(uart_gpio_base [ id ], uart_gpio_pins[ id ]);

    switch( databits )
    {
      case 5:
        config = UART_CONFIG_WLEN_5;
        break;
      case 6:
        config = UART_CONFIG_WLEN_6;
        break;
      case 7:
        config = UART_CONFIG_WLEN_7;
        break;
      default:
        config = UART_CONFIG_WLEN_8;
        break;
    }
    config |= ( stopbits == PLATFORM_UART_STOPBITS_1 ) ? UART_CONFIG_STOP_ONE : UART_CONFIG_STOP_TWO;
    if( parity == PLATFORM_UART_PARITY_EVEN )
      config |= UART_CONFIG_PAR_EVEN;
    else if( parity == PLATFORM_UART_PARITY_ODD )
      config |= UART_CONFIG_PAR_ODD;
    else
      config |= UART_CONFIG_PAR_NONE;

    MAP_UARTConfigSetExpClk( uart_base[ id ], MAP_SysCtlClockGet(), baud, config );
    MAP_UARTConfigGetExpClk( uart_base[ id ], MAP_SysCtlClockGet(), &baud, &config );
  }
  return baud;
}

void platform_s_uart_send( unsigned id, u8 data )
{
  MAP_UARTCharPut( uart_base[ id ], data );
}

int platform_s_uart_recv( unsigned id, timer_data_type timeout )
{
  u32 base = uart_base[ id ];

  if( timeout == 0 )
    return MAP_UARTCharGetNonBlocking( base );

  return MAP_UARTCharGet( base );
}

int platform_s_uart_set_flow_control( unsigned id, int type )
{
  return PLATFORM_ERR;
}

// ****************************************************************************
// Timers
// Same on LM3S8962, LM3S6965, LM3S6918 and LM3S9B92 (4 timers)

// All possible LM3S timers defs
const u32 timer_base[] = { TIMER0_BASE, TIMER1_BASE, TIMER2_BASE, TIMER3_BASE };
static const u32 timer_sysctl[] = { SYSCTL_PERIPH_TIMER0, SYSCTL_PERIPH_TIMER1, SYSCTL_PERIPH_TIMER2, SYSCTL_PERIPH_TIMER3 };

static void timers_init()
{
  unsigned i;

  for( i = 0; i < NUM_TIMER; i ++ )
  {
    MAP_SysCtlPeripheralEnable(timer_sysctl[ i ]);
    MAP_TimerConfigure(timer_base[ i ], TIMER_CFG_32_BIT_PER);
    MAP_TimerEnable(timer_base[ i ], TIMER_A);
  }
}

void platform_s_timer_delay( unsigned id, timer_data_type delay_us )
{
  timer_data_type final;
  u32 base = timer_base[ id ];

  final = 0xFFFFFFFF - ( ( ( u64 )delay_us * MAP_SysCtlClockGet() ) / 1000000 );
  MAP_TimerLoadSet( base, TIMER_A, 0xFFFFFFFF );
  while( MAP_TimerValueGet( base, TIMER_A ) > final );
}

timer_data_type platform_s_timer_op( unsigned id, int op,timer_data_type data )
{
  u32 res = 0;
  u32 base = timer_base[ id ];

  data = data;
  switch( op )
  {
    case PLATFORM_TIMER_OP_START:
      res = 0xFFFFFFFF;
      MAP_TimerControlTrigger(base, TIMER_A, false);
      MAP_TimerLoadSet( base, TIMER_A, 0xFFFFFFFF );
      break;

    case PLATFORM_TIMER_OP_READ:
      res = MAP_TimerValueGet( base, TIMER_A );
      break;

    case PLATFORM_TIMER_OP_SET_CLOCK:
    case PLATFORM_TIMER_OP_GET_CLOCK:
      res = MAP_SysCtlClockGet();
      break;

    case PLATFORM_TIMER_OP_GET_MAX_CNT:
      res = 0xFFFFFFFF;
      break;

  }
  return res;
}

u64 platform_timer_sys_raw_read()
{
  return MAP_SysTickPeriodGet() - 1 - MAP_SysTickValueGet();
}

void platform_timer_sys_disable_int()
{
  MAP_SysTickIntDisable();
}

void platform_timer_sys_enable_int()
{
  MAP_SysTickIntEnable();
}

timer_data_type platform_timer_read_sys()
{
  return cmn_systimer_get();
}

u8 lm3s_timer_int_periodic_flag[ NUM_TIMER ];
int platform_s_timer_set_match_int( unsigned id, timer_data_type period_us, int type )
{
  u32 base = timer_base[ id ];
  u64 final;

  if( period_us == 0 )
  {
    MAP_TimerDisable( base, TIMER_A );
    MAP_TimerIntDisable( base, TIMER_TIMA_TIMEOUT );
    MAP_TimerIntClear( base, TIMER_TIMA_TIMEOUT );
    MAP_TimerLoadSet( base, TIMER_A, 0xFFFFFFFF );
    MAP_TimerEnable( base, TIMER_A );
    return PLATFORM_TIMER_INT_OK;
  }
  final = ( ( u64 )period_us * MAP_SysCtlClockGet() ) / 1000000;
  if( final == 0 )
    return PLATFORM_TIMER_INT_TOO_SHORT;
  if( final > 0xFFFFFFFFULL )
    return PLATFORM_TIMER_INT_TOO_LONG;
  lm3s_timer_int_periodic_flag[ id ] = type;
  MAP_TimerDisable( base, TIMER_A );
  MAP_TimerIntClear( base, TIMER_TIMA_TIMEOUT );
  MAP_TimerLoadSet( base, TIMER_A, ( u32 )final - 1 );
  return PLATFORM_TIMER_INT_OK;
}

// ****************************************************************************
// PWMs
// Similar on LM3S8962 and LM3S6965
// LM3S6918 has no PWM

// SYSCTL div data and actual div factors
const static u32 pwm_div_ctl[] = { SYSCTL_PWMDIV_1, SYSCTL_PWMDIV_2, SYSCTL_PWMDIV_4, SYSCTL_PWMDIV_8, SYSCTL_PWMDIV_16, SYSCTL_PWMDIV_32, SYSCTL_PWMDIV_64 };
const static u8 pwm_div_data[] = { 1, 2, 4, 8, 16, 32, 64 };
// Port/pin information for all channels
#if defined(FORLM3S1968)
  const static u32 pwm_ports[] =  { GPIO_PORTG_BASE, GPIO_PORTD_BASE, GPIO_PORTH_BASE, GPIO_PORTH_BASE, GPIO_PORTF_BASE, GPIO_PORTF_BASE };
  const static u8 pwm_pins[] = { GPIO_PIN_2, GPIO_PIN_1, GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3 };
#elif defined(FORLM3S6965)
  const static u32 pwm_ports[] =  { GPIO_PORTF_BASE, GPIO_PORTD_BASE, GPIO_PORTB_BASE, GPIO_PORTB_BASE, GPIO_PORTE_BASE, GPIO_PORTE_BASE };
  const static u8 pwm_pins[] = { GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_0, GPIO_PIN_1 };
#elif defined( ELUA_BOARD_SOLDERCORE ) && defined( FORLM3S9D92 )
  const static u32 pwm_ports[] =  { GPIO_PORTG_BASE, GPIO_PORTD_BASE, GPIO_PORTD_BASE, GPIO_PORTD_BASE, GPIO_PORTE_BASE, GPIO_PORTE_BASE, GPIO_PORTC_BASE, GPIO_PORTC_BASE };
  const static u8 pwm_pins[] = { GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_6, GPIO_PIN_7,  GPIO_PIN_4,  GPIO_PIN_6 };
  const static u32 pwm_configs[] = { GPIO_PG0_PWM0, GPIO_PD1_PWM1, GPIO_PD2_PWM2, GPIO_PD3_PWM3, GPIO_PE6_PWM4, GPIO_PE7_PWM5, GPIO_PC4_PWM6, GPIO_PC6_PWM7 };
#elif defined( FORLM3S9B92 ) || ( defined(FORLM3S9D92) && !defined( ELUA_BOARD_SOLDERCORE ) )
  const static u32 pwm_ports[] =  { GPIO_PORTD_BASE, GPIO_PORTD_BASE, GPIO_PORTD_BASE, GPIO_PORTD_BASE, GPIO_PORTE_BASE, GPIO_PORTE_BASE, GPIO_PORTC_BASE, GPIO_PORTC_BASE };
  const static u8 pwm_pins[] = { GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_6, GPIO_PIN_7,  GPIO_PIN_4,  GPIO_PIN_6 };
  const static u32 pwm_configs[] = { GPIO_PD0_PWM0, GPIO_PD1_PWM1, GPIO_PD2_PWM2, GPIO_PD3_PWM3, GPIO_PE6_PWM4, GPIO_PE7_PWM5, GPIO_PC4_PWM6, GPIO_PC6_PWM7 };
#else
  const static u32 pwm_ports[] =  { GPIO_PORTF_BASE, GPIO_PORTG_BASE, GPIO_PORTB_BASE, GPIO_PORTB_BASE, GPIO_PORTE_BASE, GPIO_PORTE_BASE };
  const static u8 pwm_pins[] = { GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_0, GPIO_PIN_1 };
#endif

// PWM generators
#if defined( FORLM3S9B92 ) || defined(FORLM3S9D92)
  const static u16 pwm_gens[] = { PWM_GEN_0, PWM_GEN_1, PWM_GEN_2, PWM_GEN_3 };
#else
  const static u16 pwm_gens[] = { PWM_GEN_0, PWM_GEN_1, PWM_GEN_2 };
#endif

// PWM outputs
#if defined( FORLM3S9B92 ) || defined(FORLM3S9D92)
const static u16 pwm_outs[] = { PWM_OUT_0, PWM_OUT_1, PWM_OUT_2, PWM_OUT_3, PWM_OUT_4, PWM_OUT_5, PWM_OUT_6, PWM_OUT_7};
#else
const static u16 pwm_outs[] = { PWM_OUT_0, PWM_OUT_1, PWM_OUT_2, PWM_OUT_3, PWM_OUT_4, PWM_OUT_5 };
#endif

static void pwms_init()
{
  MAP_SysCtlPeripheralEnable( SYSCTL_PERIPH_PWM );
  MAP_SysCtlPWMClockSet( SYSCTL_PWMDIV_1 );
}

// Return the PWM clock
u32 platform_pwm_get_clock( unsigned id )
{
  unsigned i;
  u32 clk;

  clk = MAP_SysCtlPWMClockGet();
  for( i = 0; i < sizeof( pwm_div_ctl ) / sizeof( u32 ); i ++ )
    if( clk == pwm_div_ctl[ i ] )
      break;
  return MAP_SysCtlClockGet() / pwm_div_data[ i ];
}

// Set the PWM clock
u32 platform_pwm_set_clock( unsigned id, u32 clock )
{
  unsigned i, min_i;
  u32 sysclk;

  sysclk = MAP_SysCtlClockGet();
  for( i = min_i = 0; i < sizeof( pwm_div_data ) / sizeof( u8 ); i ++ )
    if( ABSDIFF( clock, sysclk / pwm_div_data[ i ] ) < ABSDIFF( clock, sysclk / pwm_div_data[ min_i ] ) )
      min_i = i;
  MAP_SysCtlPWMClockSet( pwm_div_ctl[ min_i ] );
  return sysclk / pwm_div_data[ min_i ];
}

u32 platform_pwm_setup( unsigned id, u32 frequency, unsigned duty )
{
  u32 pwmclk = platform_pwm_get_clock( id );
  u32 period;

#if defined( FORLM3S9B92 ) || defined(FORLM3S9D92)
  GPIOPinConfigure( pwm_configs[ id ] );
#endif

  // Set pin as PWM
  MAP_GPIOPinTypePWM( pwm_ports[ id ], pwm_pins[ id ] );
  // Compute period
  period = pwmclk / frequency;
  // Set the period
  MAP_PWMGenConfigure( PWM_BASE, pwm_gens[ id >> 1 ], PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC );
  MAP_PWMGenPeriodSet( PWM_BASE, pwm_gens[ id >> 1 ], period );
  // Set duty cycle
  MAP_PWMPulseWidthSet( PWM_BASE, pwm_outs[ id ], ( period * duty ) / 100 );
  // Return actual frequency
  return pwmclk / period;
}

void platform_pwm_start( unsigned id )
{
  MAP_PWMOutputState( PWM_BASE, 1 << id, true );
  MAP_PWMGenEnable( PWM_BASE, pwm_gens[ id >> 1 ] );
}

void platform_pwm_stop( unsigned id )
{
  MAP_PWMOutputState( PWM_BASE, 1 << id, false );
  MAP_PWMGenDisable( PWM_BASE, pwm_gens[ id >> 1 ] );
}

// *****************************************************************************
// ADC specific functions and variables

#ifdef BUILD_ADC

// Pin configuration if necessary
#if defined( FORLM3S9B92 ) || defined(FORLM3S9D92)
  const static u32 adc_ports[] =  { GPIO_PORTE_BASE, GPIO_PORTE_BASE, GPIO_PORTE_BASE, GPIO_PORTE_BASE,
                                    GPIO_PORTD_BASE, GPIO_PORTD_BASE, GPIO_PORTD_BASE, GPIO_PORTD_BASE,
                                    GPIO_PORTE_BASE, GPIO_PORTE_BASE, GPIO_PORTB_BASE, GPIO_PORTB_BASE,
                                    GPIO_PORTD_BASE, GPIO_PORTD_BASE, GPIO_PORTD_BASE, GPIO_PORTD_BASE };
                                    
  const static u8 adc_pins[] =    { GPIO_PIN_7, GPIO_PIN_6, GPIO_PIN_5, GPIO_PIN_4,
                                    GPIO_PIN_7, GPIO_PIN_6, GPIO_PIN_5, GPIO_PIN_4,
                                    GPIO_PIN_3, GPIO_PIN_2, GPIO_PIN_4, GPIO_PIN_5,
                                    GPIO_PIN_3, GPIO_PIN_2, GPIO_PIN_1, GPIO_PIN_0 };

  const static u32 adc_ctls[] = { ADC_CTL_CH0, ADC_CTL_CH1, ADC_CTL_CH2, ADC_CTL_CH3,
                                  ADC_CTL_CH4, ADC_CTL_CH5, ADC_CTL_CH6, ADC_CTL_CH7,
                                  ADC_CTL_CH8, ADC_CTL_CH9, ADC_CTL_CH10, ADC_CTL_CH11,
                                  ADC_CTL_CH12, ADC_CTL_CH13, ADC_CTL_CH14, ADC_CTL_CH15 };

  #define ADC_PIN_CONFIG
#else
const static u32 adc_ctls[] = { ADC_CTL_CH0, ADC_CTL_CH1, ADC_CTL_CH2, ADC_CTL_CH3 };
#endif

const static u32 adc_ints[] = { INT_ADC0, INT_ADC1, INT_ADC2, INT_ADC3 };

int platform_adc_check_timer_id( unsigned id, unsigned timer_id )
{
  return ( ( timer_id >= ADC_TIMER_FIRST_ID ) && ( timer_id < ( ADC_TIMER_FIRST_ID + ADC_NUM_TIMERS ) ) );
}

void platform_adc_stop( unsigned id )
{
  elua_adc_ch_state *s = adc_get_ch_state( id );
  elua_adc_dev_state *d = adc_get_dev_state( 0 );
  
  s->op_pending = 0;
  INACTIVATE_CHANNEL(d, id);
  
  // If there are no more active channels, stop the sequencer
  if( d->ch_active == 0 )
  {
    MAP_ADCSequenceDisable( ADC_BASE, d->seq_id );
    d->running = 0;
  }
}

// Handle ADC interrupts
void ADCIntHandler( void )
{
  u32 tmpbuff[ NUM_ADC ];
  elua_adc_dev_state *d = adc_get_dev_state( 0 );
  elua_adc_ch_state *s;

  MAP_ADCIntClear( ADC_BASE, d->seq_id );
  MAP_ADCSequenceDataGet( ADC_BASE, d->seq_id, tmpbuff );
  
  d->seq_ctr = 0;
  
  // Update smoothing and/or write to buffer if needed
  while( d->seq_ctr < d->seq_len )
  {
    s = d->ch_state[ d->seq_ctr ];
    d->sample_buf[ d->seq_ctr ] = ( u16 )tmpbuff[ d->seq_ctr ];
    s->value_fresh = 1; // Mark sample as fresh
    
    // Fill in smoothing buffer until warmed up
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
    
    d->seq_ctr++;
  }
  d->seq_ctr = 0;
  
  // Only attempt to refresh sequence order if still running
  // This allows us to "cache" an old sequence if all channels
  // finish at the same time
  if ( d->running == 1 )
    adc_update_dev_sequence( 0 );
  
  if ( d->clocked == 0 && d->running == 1 )
  {
    // Need to manually fire off sample request in single sample mode
    MAP_ADCProcessorTrigger( ADC_BASE, d->seq_id );
  }
}

static void adcs_init()
{
  unsigned id;
  elua_adc_dev_state *d = adc_get_dev_state( 0 );
  
  MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC);

  // Try ramping up max sampling rate
  MAP_SysCtlADCSpeedSet(SYSCTL_ADCSPEED_500KSPS);
  MAP_SysCtlADCSpeedSet(SYSCTL_ADCSPEED_1MSPS);
  
  for( id = 0; id < NUM_ADC; id ++ )
    adc_init_ch_state( id );

  // Perform sequencer setup
  platform_adc_set_clock( 0, 0 );
  MAP_ADCIntEnable( ADC_BASE, d->seq_id );
  MAP_IntEnable( adc_ints[ 0 ] ); // Enable sequencer 0 int
}

u32 platform_adc_set_clock( unsigned id, u32 frequency )
{
  elua_adc_dev_state *d = adc_get_dev_state( 0 );
  
  // Make sure sequencer is disabled before making changes
  MAP_ADCSequenceDisable( ADC_BASE, d->seq_id );
  
  if ( frequency > 0 )
  {
    d->clocked = 1;
    // Set sequence id to be triggered repeatedly, with priority id
    MAP_ADCSequenceConfigure( ADC_BASE, d->seq_id, ADC_TRIGGER_TIMER, d->seq_id );

    // Set up timer trigger
    MAP_TimerLoadSet( timer_base[ d->timer_id ], TIMER_A, MAP_SysCtlClockGet() / frequency );
    frequency = MAP_SysCtlClockGet() / MAP_TimerLoadGet( timer_base[ d->timer_id ], TIMER_A );
  }
  else
  {
    d->clocked = 0;
    // Conversion will run back-to-back until required samples are acquired
    MAP_ADCSequenceConfigure( ADC_BASE, d->seq_id, ADC_TRIGGER_PROCESSOR, d->seq_id ) ;
  }
    
  return frequency;
}


int platform_adc_update_sequence( )
{  
  elua_adc_dev_state *d = adc_get_dev_state( 0 );
  
  MAP_ADCSequenceDisable( ADC_BASE, d->seq_id );
  
  // NOTE: seq ctr should have an incrementer that will wrap appropriately..
  d->seq_ctr = 0; 
  while( d->seq_ctr < d->seq_len-1 )
  {
    MAP_ADCSequenceStepConfigure( ADC_BASE, d->seq_id, d->seq_ctr, adc_ctls[ d->ch_state[ d->seq_ctr ]->id ] );
#ifdef ADC_PIN_CONFIG
    MAP_GPIOPinTypeADC( adc_ports[ d->ch_state[ d->seq_ctr ]->id ], adc_pins[ d->ch_state[ d->seq_ctr ]->id ] );
#endif
    d->seq_ctr++;
  }
  MAP_ADCSequenceStepConfigure( ADC_BASE, d->seq_id, d->seq_ctr, ADC_CTL_IE | ADC_CTL_END | adc_ctls[ d->ch_state[ d->seq_ctr ]->id ] );
#ifdef ADC_PIN_CONFIG
  MAP_GPIOPinTypeADC( adc_ports[ d->ch_state[ d->seq_ctr ]->id ], adc_pins[ d->ch_state[ d->seq_ctr ]->id ] );
#endif
  d->seq_ctr = 0;
  
  MAP_ADCSequenceEnable( ADC_BASE, d->seq_id );
      
  return PLATFORM_OK;
}


int platform_adc_start_sequence()
{ 
  elua_adc_dev_state *d = adc_get_dev_state( 0 );
  
  if( d->running != 1 )
  {
    adc_update_dev_sequence( 0 );

    MAP_ADCSequenceEnable( ADC_BASE, d->seq_id );
    d->running = 1;

    if( d->clocked == 1 )
    {
      MAP_TimerControlTrigger(timer_base[d->timer_id], TIMER_A, true);
      MAP_TimerEnable(timer_base[d->timer_id], TIMER_A);
    }
    else
    {
      MAP_ADCProcessorTrigger( ADC_BASE, d->seq_id );
    }
  }
  
  return PLATFORM_OK;
}

#endif // ifdef BUILD_ADC

// ****************************************************************************
// Support for specific onboard devices on 
// Texas Instruments / Luminary Micro kits.
//
// FIXME: This was previously tied to the "disp" module but should be renamed in the future
//        to include support for initialization of other onboard devices of the EK-LM3Sxxxx kits.
//        Note that not all kits have all devices available.

void lm3s_disp_init( unsigned long freq )
{
  RIT128x96x4Init( freq );
}

void lm3s_disp_clear()
{
  RIT128x96x4Clear();
}

void lm3s_disp_stringDraw( const char *str, unsigned long x, unsigned long y, unsigned char level )
{
  RIT128x96x4StringDraw( str, x, y, level );
}

void lm3s_disp_imageDraw( const unsigned char *img, unsigned long x, unsigned long y,
                              unsigned long width, unsigned long height )
{
  RIT128x96x4ImageDraw( img, x, y, width, height );
}


void lm3s_disp_enable( unsigned long freq )
{
  RIT128x96x4Enable( freq );
}

void lm3s_disp_disable()
{
  RIT128x96x4Disable();
}

void lm3s_disp_displayOn()
{
  RIT128x96x4DisplayOn();
}

void lm3s_disp_displayOff()
{
  RIT128x96x4DisplayOff();
}


// ****************************************************************************
// Ethernet functions

static void eth_init()
{
#ifdef BUILD_UIP
  u32 user0, user1, temp;
  static struct uip_eth_addr sTempAddr;

  // Enable and reset the controller
  MAP_SysCtlPeripheralEnable( SYSCTL_PERIPH_ETH );
  MAP_SysCtlPeripheralReset( SYSCTL_PERIPH_ETH );

#if defined( FORLM3S9B92 ) || defined(FORLM3S9D92)
  GPIOPinConfigure(GPIO_PF2_LED1);
  GPIOPinConfigure(GPIO_PF3_LED0);
#endif

  // Enable Ethernet LEDs
  MAP_GPIODirModeSet( GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3, GPIO_DIR_MODE_HW );
  MAP_GPIOPadConfigSet( GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD );

  // Configure SysTick for a periodic interrupt.
  MAP_SysTickPeriodSet( MAP_SysCtlClockGet() / SYSTICKHZ);
  MAP_SysTickEnable();
  MAP_SysTickIntEnable();

  // Intialize the Ethernet Controller and disable all Ethernet Controller interrupt sources.
  MAP_EthernetIntDisable(ETH_BASE, (ETH_INT_PHY | ETH_INT_MDIO | ETH_INT_RXER |
                     ETH_INT_RXOF | ETH_INT_TX | ETH_INT_TXER | ETH_INT_RX));
  temp = MAP_EthernetIntStatus(ETH_BASE, false);
  MAP_EthernetIntClear(ETH_BASE, temp);

  // Initialize the Ethernet Controller for operation.
  MAP_EthernetInitExpClk(ETH_BASE, MAP_SysCtlClockGet());

  // Configure the Ethernet Controller for normal operation.
  // - Full Duplex
  // - TX CRC Auto Generation
  // - TX Padding Enabled
  MAP_EthernetConfigSet(ETH_BASE, (ETH_CFG_TX_DPLXEN | ETH_CFG_TX_CRCEN | ETH_CFG_TX_PADEN));

  // Enable the Ethernet Controller.
  MAP_EthernetEnable(ETH_BASE);

  // Enable the Ethernet interrupt.
  MAP_IntEnable(INT_ETH);

  // Enable the Ethernet RX Packet interrupt source.
  MAP_EthernetIntEnable(ETH_BASE, ETH_INT_RX);

  // Enable all processor interrupts.
  MAP_IntMasterEnable();

  // Configure the hardware MAC address for Ethernet Controller filtering of
  // incoming packets.
  //
  // For the Ethernet Eval Kits, the MAC address will be stored in the
  // non-volatile USER0 and USER1 registers.  These registers can be read
  // using the FlashUserGet function, as illustrated below.


#if defined( ELUA_BOARD_SOLDERCORE )
  user0 = 0x00b61a00;
  user1 = 0x006d0a00;
#else
  MAP_FlashUserGet(&user0, &user1);
#endif
  

  // Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
  // address needed to program the hardware registers, then program the MAC
  // address into the Ethernet Controller registers.
  sTempAddr.addr[0] = ((user0 >>  0) & 0xff);
  sTempAddr.addr[1] = ((user0 >>  8) & 0xff);
  sTempAddr.addr[2] = ((user0 >> 16) & 0xff);
  sTempAddr.addr[3] = ((user1 >>  0) & 0xff);
  sTempAddr.addr[4] = ((user1 >>  8) & 0xff);
  sTempAddr.addr[5] = ((user1 >> 16) & 0xff);

  // Program the hardware with it's MAC address (for filtering).
  MAP_EthernetMACAddrSet(ETH_BASE, (unsigned char *)&sTempAddr);

  // Initialize the eLua uIP layer
  elua_uip_init( &sTempAddr );
#endif
}

#ifdef BUILD_UIP
static int eth_timer_fired;

void platform_eth_send_packet( const void* src, u32 size )
{
  MAP_EthernetPacketPut( ETH_BASE, uip_buf, uip_len );
}

u32 platform_eth_get_packet_nb( void* buf, u32 maxlen )
{
  return MAP_EthernetPacketGetNonBlocking( ETH_BASE, uip_buf, sizeof( uip_buf ) );
}

void platform_eth_force_interrupt()
{
  NVIC_SW_TRIG_R |= INT_ETH - 16;
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

void SysTickIntHandler()
{
  // Handle virtual timers
  cmn_virtual_timer_cb();

  // Indicate that a SysTick interrupt has occurred.
  eth_timer_fired = 1;

  // Generate a fake Ethernet interrupt.  This will perform the actual work
  // of incrementing the timers and taking the appropriate actions.
  platform_eth_force_interrupt();

  // System timer handling
  cmn_systimer_periodic();
}

void EthernetIntHandler()
{
  u32 temp;

  // Read and Clear the interrupt.
  temp = EthernetIntStatus( ETH_BASE, false );
  EthernetIntClear( ETH_BASE, temp );

  // Call the UIP main loop
  elua_uip_mainloop();
}

#else  // #ifdef ELUA_UIP

void SysTickIntHandler()
{
  cmn_virtual_timer_cb();

  // System timer handling
  cmn_systimer_periodic();
}

void EthernetIntHandler()
{
}
#endif // #ifdef ELUA_UIP

// ****************************************************************************
// USB functions

#if defined( BUILD_USB_CDC )

static void usb_init()
{
  USBBufferInit( &g_sTxBuffer );
  USBBufferInit( &g_sRxBuffer );

  // Pass the device information to the USB library and place the device
  // on the bus.
  USBDCDCInit( 0, &g_sCDCDevice );
}

void platform_usb_cdc_send( u8 data )
{
  USBBufferWrite( &g_sTxBuffer, &data, 1 );
}

int platform_usb_cdc_recv( s32 timeout )
{
  unsigned char data;
  unsigned long read;

  // Try to read one byte from buffer, if none available return -1 or
  // retry if timeout
  // FIXME: Respect requested timeout
  do {
    read = USBBufferRead(&g_sRxBuffer, &data, 1);
  } while( read == 0 && timeout != 0 );

  if( read == 0 )
    return -1;
  else
    return data;
}

unsigned long TxHandler(void *pvCBData, unsigned long ulEvent, unsigned long ulMsgValue, void *pvMsgData)
{
  // Which event was sent?
  switch(ulEvent)
  {
    case USB_EVENT_TX_COMPLETE:
    {
        // Nothing to do, already handled by USBBuffer
        break;
    }
      
    default:
        break;
  }
  
  return(0);
}

unsigned long RxHandler(void *pvCBData, unsigned long ulEvent, unsigned long ulMsgValue, void *pvMsgData)
{
  unsigned long ulCount;
  unsigned char ucChar;
  unsigned long ulRead;


  // Which event was sent?
  switch(ulEvent)
  {
    // A new packet has been received.
    case USB_EVENT_RX_AVAILABLE:
    {
      break;
    }

    //
    // This is a request for how much unprocessed data is still waiting to
    // be processed.  Return 0 if the UART is currently idle or 1 if it is
    // in the process of transmitting something.  The actual number of
    // bytes in the UART FIFO is not important here, merely whether or
    // not everything previously sent to us has been transmitted.
    //
    case USB_EVENT_DATA_REMAINING:
    {
      //
      // Get the number of bytes in the buffer and add 1 if some data
      // still has to clear the transmitter.
      //
      return(0);
    }

    //
    // This is a request for a buffer into which the next packet can be
    // read.  This mode of receiving data is not supported so let the
    // driver know by returning 0.  The CDC driver should not be sending
    // this message but this is included just for illustration and
    // completeness.
    //
    case USB_EVENT_REQUEST_BUFFER:
    {
      return(0);
    }

    // Other events can be safely ignored.
    default:
    {
      break;
    }
  }

  return(0);
}

unsigned long
ControlHandler(void *pvCBData, unsigned long ulEvent, unsigned long ulMsgValue,
               void *pvMsgData)
{
  switch(ulEvent) // Check event
  {
    // The host has connected.
    case USB_EVENT_CONNECTED:
    {
      USBBufferFlush(&g_sTxBuffer);
      USBBufferFlush(&g_sRxBuffer);
      break;
    }

    
    // The host has disconnected.
    
    case USB_EVENT_DISCONNECTED:
    {
      break;
    }
    
    // Return the current serial communication parameters.
    case USBD_CDC_EVENT_GET_LINE_CODING:
    {
      break;
    }

    // Set the current serial communication parameters.
    case USBD_CDC_EVENT_SET_LINE_CODING:
    {
      break;
    }

    
    // Set the current serial communication parameters.
    case USBD_CDC_EVENT_SET_CONTROL_LINE_STATE:
    {
      break;
    }

    //
    // Send a break condition on the serial line.
    //
    case USBD_CDC_EVENT_SEND_BREAK:
    {
      break;
    }

    //
    // Clear the break condition on the serial line.
    //
    case USBD_CDC_EVENT_CLEAR_BREAK:
    {
      break;
    }

    //
    // Ignore SUSPEND and RESUME for now.
    //
    case USB_EVENT_SUSPEND:
    case USB_EVENT_RESUME:
    {
      break;
    }

    //
    // Other events can be safely ignored.
    //
    default:
    {
      break;
    }
  }

  return(0);
}

#endif // BUILD_USB_CDC

// ****************************************************************************
// Flash access functions

#ifdef BUILD_WOFS
u32 platform_s_flash_write( const void *from, u32 toaddr, u32 size )
{
  return MAP_FlashProgram( ( unsigned long * )from, toaddr, size );
}

int platform_flash_erase_sector( u32 sector_id )
{
  return FlashErase( sector_id * INTERNAL_FLASH_SECTOR_SIZE ) == 0 ? PLATFORM_OK : PLATFORM_ERR;
}
#endif // #ifdef BUILD_WOFS

// ****************************************************************************
// Platform specific modules go here

#if defined( ENABLE_DISP ) || defined( ENABLE_LM3S_GPIO )

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"

extern const LUA_REG_TYPE disp_map[];
extern const LUA_REG_TYPE lm3s_pio_map[];

const LUA_REG_TYPE platform_map[] =
{
#if LUA_OPTIMIZE_MEMORY > 0
#ifdef ENABLE_DISP
  { LSTRKEY( "disp" ), LROVAL( disp_map ) },
#endif
#ifdef ENABLE_LM3S_GPIO
  { LSTRKEY( "pio" ), LROVAL( lm3s_pio_map ) },
#endif
#endif
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_platform( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, PS_LIB_TABLE_NAME, platform_map );

  // Setup the new tables inside platform table
  lua_newtable( L );
  luaL_register( L, NULL, disp_map );
  lua_setfield( L, -2, "disp" );
  lua_newtable( L );
  luaL_register( L, NULL, lm3s_pio_map );
  lua_setfield( L, -2, "pio" );

  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}

#else // #if defined( ENABLE_DISP ) || defined( ENABLE_LM3S_GPIO )

LUALIB_API int luaopen_platform( lua_State *L )
{
  return 0;
}

#endif // #if defined( ENABLE_DISP ) || defined( ENABLE_LM3S_GPIO )

